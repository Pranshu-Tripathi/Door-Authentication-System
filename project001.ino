
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
int row[] = {4, 5, 6, 7};
int col[] = {8, 9, 10, 11};
int i, j;
int col_scan;
int passcode = 1234;
int inputcode = 0;
int operation = -1;
int id = 1;
void setup()
{
    pinMode(send_signal, OUTPUT);
    pinMode(open_lock, OUTPUT);
    Serial.begin(9600);
    for (i = 0; i <= 3; i++)
    {
        pinMode(row[i], OUTPUT);
        pinMode(col[i], INPUT);
        digitalWrite(col[i], HIGH);
    }

    finger.begin(57600);
    delay(5);
    if (finger.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        }
    }

    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x"));
    Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));
    Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: "));
    Serial.println(finger.capacity);
    Serial.print(F("Security level: "));
    Serial.println(finger.security_level);
    Serial.print(F("Device address: "));
    Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));
    Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: "));
    Serial.println(finger.baud_rate);

    finger.getTemplateCount();
    id = finger.templateCount+1;
    digitalWrite(send_signal, LOW);
    digitalWrite(open_lock, LOW);
}

void loop()
{
    operation = -1;
    inputcode = 0;

    digitalWrite(send_signal, LOW);
    digitalWrite(open_lock, LOW);

    Serial.println("A -> Unlock Door");
    Serial.println("B -> Enroll New User");
    Serial.println("C -> Password Reset");
    Serial.println("D -> Delete User");

    operation = enterOperation();

    if (operation == 200)
    {
        auth();
    }
    else if (operation == 201)
    {
        if(auth()){
            while(!getFingerprintEnroll()){}
            id++;
        }

    }
    else if (operation == 202)
    {

        if (auth())
        {
            Serial.print("Input New Passowrd :-> \n");
            int temp = enterPassword();
            if(temp != -1)
            {
                passcode = temp;
                Serial.print("\n Password Updated to -> ");
                Serial.println(passcode);
            }
            else
            {
                Serial.print("\n Error !\n");
            }
        }
    }
    else if (operation == 203)
    {
    }
    else if (operation == 204)
    {
    }
}

int enterOperation()
{
    while (true)
    {
        int key = -1;
        for (i = 0; i <= 3; i++)
        {
            digitalWrite(row[0], HIGH);
            digitalWrite(row[1], HIGH);
            digitalWrite(row[2], HIGH);
            digitalWrite(row[3], HIGH);
            digitalWrite(row[i], LOW);
            for (j = 0; j <= 3; j++)
            {
                col_scan = digitalRead(col[j]);
                if (col_scan == LOW)
                {
                    key = getPressedKey(i, j);
                    delay(300);
                    break;
                }
            }
        }
        if (key >= 200)
        {
            return key;
        }
    }
}

int enterPassword()
{
    Serial.print("Input Password : ");
    int tempPassword = 0;
    while (tempPassword < 1000)
    {
        int key = -1;
        for (i = 0; i <= 3; i++)
        {
            digitalWrite(row[0], HIGH);
            digitalWrite(row[1], HIGH);
            digitalWrite(row[2], HIGH);
            digitalWrite(row[3], HIGH);
            digitalWrite(row[i], LOW);
            for (j = 0; j <= 3; j++)
            {
                col_scan = digitalRead(col[j]);
                if (col_scan == LOW)
                {
                    key = getPressedKey(i, j);
                    delay(300);
                    break;
                }
            }
        }
        if (key >= 200)
        {
            return -1;
        }
        if (key != -1)
            tempPassword = tempPassword * 10 + key;
    }
    return tempPassword;
}

bool auth()
{
    // unlock check
    inputcode = enterPassword();
    if (passwordMatch())
    {
        int b = fingerprintMatch();
        if (b > 0)
        {
            Serial.println("\nAccepted");
            digitalWrite(open_lock, HIGH);
            delay(5000);
            return true;
        }
    }
    Serial.println("\nDeclined");
    digitalWrite(send_signal, HIGH);
    delay(5000);
    return false;
}

int getPressedKey(int i, int j)
{
    if (j == 3)
    {
        if (i == 0)
        {
            Serial.println("A");
            return 200;
        }
        else if (i == 1)
        {
            Serial.println("B");
            return 201;
        }
        else if (i == 2)
        {
            Serial.println("C");

            return 202;
        }
        else
        {
            Serial.println("D");
            return 203;
        }
    }
    else if (i == 3)
    {
        if (j == 0)
        {
            Serial.println("*");
            return 205;
        }
        else if (j == 1)
        {
            Serial.print(0);
            return 0;
        }
        else
        {
            Serial.println("#");
            return 204;
        }
    }
    else
    {
        Serial.print(i * 3 + j + 1);
        //   Serial.print("Hello");
        return i * 3 + j + 1;
    }
}

bool passwordMatch()
{
    return inputcode == passcode;
}

int fingerprintMatch()
{
    Serial.println("Enter Fingerprint >>>>>>>>>>>>>>>>");
    int p = 0;
    int b = -1;
    while (p < 100)
    {
        b = getFingerprintID();
        Serial.println(p);
        if (b > 0)
            break;
        p++;
        delay(100);
    }
    return b;
}

int getFingerprintID()
{
    uint8_t p = finger.getImage();
    switch (p)
    {
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
    switch (p)
    {
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
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return -1;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Did not find a match");
        return 0;
    }
    else
    {
        Serial.println("Unknown error");
        return -1;
    }

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);

    return finger.fingerID;
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
