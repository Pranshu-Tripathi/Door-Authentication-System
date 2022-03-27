
#include <Adafruit_Fingerprint.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int send_signal = 13;
int open_lock = 12;
int row[]={4,5,6,7};
int col[]={8,9,10,11};
int i,j;
int col_scan;
int passcode = 1234;
int inputcode = 0;
void setup()
{
  pinMode(send_signal,OUTPUT);
  pinMode(open_lock,OUTPUT);
  Serial.begin(9600);
  for(i=0;i<=3;i++)
  {
    pinMode(row[i],OUTPUT);
    pinMode(col[i],INPUT);
    digitalWrite(col[i],HIGH);
  } 

  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();
  digitalWrite(send_signal,LOW);
  digitalWrite(open_lock,LOW);
}

void loop()
{ 
  digitalWrite(send_signal,LOW);
  digitalWrite(open_lock,LOW);
  while(inputcode < 1000)
  {
    digitalWrite(send_signal,LOW);
    digitalWrite(open_lock,LOW);
    for(i=0; i<=3; i++)
    {
      digitalWrite(row[0],HIGH);
      digitalWrite(row[1],HIGH);
      digitalWrite(row[2],HIGH);
      digitalWrite(row[3],HIGH);
      digitalWrite(row[i],LOW);
      for(j=0; j<=3; j++)
      {
        col_scan=digitalRead(col[j]);
        if(col_scan==LOW)
        {
          getPressedKey(i,j);
          delay(300);
        }
      }
    }
  }

  if(inputcode == passcode)
  {
    Serial.println("\nAccepted By Hex KeyPad");
    Serial.println("Enter Fingerprint >>>>>>>>>>>>>>>>");
    int p = 0;
    int b = -1;
    while(p < 100)
    {
      b = getFingerprintID();
      Serial.println(p);
      if(b > 0)
        break;
      p++;
      //Serial.println(p);
      delay(50);
    }
    if(b > 0) {
      Serial.println("\n Accepted");
      digitalWrite(open_lock,HIGH);
      delay(5000);
    }
    else{
      Serial.println("\n Declined");
      digitalWrite(send_signal,HIGH);
      delay(5000);
    }

    inputcode = 0;
    delay(1000);
  }
  else
  {
    Serial.println("\nDeclined\n");
    inputcode = 0;
    digitalWrite(send_signal,HIGH);
    delay(5000);
    delay(1000);
  }
}


void getPressedKey(int i, int j)
{
  if(j == 3){
    if(i == 0){
      Serial.print("A"); 
      inputcode = 0; 
    }
    else if(i == 1){
      Serial.print("B");
      inputcode = 0;
    }
    else if(i == 2){
      Serial.print("C");
      inputcode = 0;
    }
    else{
      Serial.print("D");
      inputcode = 0;  
    }
  }
  else if(i == 3){
    if(j == 0){
      Serial.print("*"); 
      inputcode = 0; 
    }
    else if(j == 1){
      Serial.print(0);
      inputcode = inputcode * 10;
    }
    else if(j == 2){
      Serial.print("#");
      inputcode = 0;
    }
    else{
      Serial.print("D");
      inputcode = 0;  
    }
  }
  else{
    inputcode = inputcode * 10 + i*3 + j + 1;
    Serial.print(i*3 + j + 1);
  }
}



int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -1;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return -1;
    default:
      Serial.println("Unknown error");
      return -1;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -1;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return -1;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return -1;
    default:
      Serial.println("Unknown error");
      return -1;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return -1;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return 0;
  } else {
    Serial.println("Unknown error");
    return -1;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}
