int row[]={4,5,6,7};
int col[]={8,9,10,11};
int i,j;
int col_scan;
int passcode = 1234;
int inputcode = 0;
void setup()
{
  Serial.begin(9600);
  for(i=0;i<=3;i++)
  {
    pinMode(row[i],OUTPUT);
    pinMode(col[i],INPUT);
    digitalWrite(col[i],HIGH);
  } 
}

void loop()
{ 

  while(inputcode < 1000)
  {
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
          keypress(i,j);
          delay(300);
        }
      }
    }
  }

  if(inputcode == passcode)
  {
    Serial.println("\nAccepted");
    inputcode = 0;
    delay(1000);
  }
  else
  {
    Serial.println("\nDeclined\n");
    inputcode = 0;
    delay(1000);
  }
}


void keypress(int i, int j)
{
  if(i==0&&j==0)
  {
    Serial.print("1");
    inputcode = inputcode * 10 + 1;
  }
  if(i==0&&j==1)
  {
    Serial.print("2");
    inputcode = inputcode * 10 + 2;
  }
  if(i==0&&j==2)
  {
    Serial.print("3");
    inputcode = inputcode * 10 + 3;
  }
  if(i==0&&j==3)
  {
    Serial.println("A");
  }
  if(i==1&&j==0)
  {
    Serial.print("4");
    inputcode = inputcode * 10 + 4;
  }
  if(i==1&&j==1)
  {
    Serial.print("5");
    inputcode = inputcode * 10 + 5;
  }
  if(i==1&&j==2)
  {
    Serial.print("6");
    inputcode = inputcode * 10 + 6;
  }
  if(i==1&&j==3)
  {
    Serial.println("B");
  }
  if(i==2&&j==0)
  {
    Serial.print("7");
    inputcode = inputcode * 10 + 7;
  }
  if(i==2&&j==1)
  {
    Serial.print("8");
    inputcode = inputcode * 10 + 8;
  }
  if(i==2&&j==2)
  {
    Serial.print("9");
    inputcode = inputcode * 10 + 9;
  }
  if(i==2&&j==3)
  {
    Serial.println("C");
  }
  if(i==3&&j==0)
  {
    Serial.println("*");
  }
  if(i==3&&j==1)
  {
    Serial.print("0");
    inputcode = inputcode * 10;
  }
  if(i==3&&j==2)
    Serial.println("#");
  if(i==3&&j==3)
    Serial.println("D");
}
