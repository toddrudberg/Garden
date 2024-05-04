#include <SoftwareSerial.h>

int led = 13;
int EN = 4;  //Definition RS485 shield enable terminal 
SoftwareSerial mySerial(2, 3); // RX, TX

uint8_t Com[8] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09 };
unsigned long baud = 9600;
void setup()
{
  Serial.begin(baud);
  mySerial.begin(baud);
  pinMode(led,OUTPUT);
  pinMode(EN,OUTPUT);
  digitalWrite(EN, LOW);  
}

int processStep = 0;
void loop()
{
  static unsigned long startTime = millis();  
  int temp;
  switch(processStep)
  {
    case 0:
      digitalWrite(led, HIGH);
      digitalWrite(EN, HIGH);    //Enable high, RS485 shield waiting to transmit data
      //delay(10);
      for (int i = 0; i < 8; i++) 
      {
        mySerial.write(Com[i]);
      }

      digitalWrite(EN,LOW);    //Enable low, RS485 shield waiting to receive data
      digitalWrite(led, LOW);
      delay(100);    //Delay for some time, waiting for data transmitted

      startTime = millis();
      processStep++;
      break;

    case 1:
      if(millis() - startTime > 250)
      {
        startTime = millis();
        processStep++;
      }
    case 2:
    {
      int i = 0;
      uint8_t Data[100] = { 0 };
      if(mySerial.available())
      {

        while (mySerial.available() > 0) 
        {
            int temp = mySerial.read();
            Data[i++] = temp;
        }
        // Serial.print("bytes read: ");
        // Serial.println(i);
        int offset = i-10;
        float hum = ((Data[0+offset] << 8) + Data[1+offset]) / 10.00;
        float temC = ((Data[2+offset] << 8) + Data[3+offset]) / 10.00;
        float ec = ((Data[4+offset] << 8) + Data[5+offset]);
        float ph = ((Data[6+offset] << 8) + Data[7+offset]) /10.00;
        float temF = temC * 9.0/5.0 + 32.0;
        Serial.println();
        Serial.print("Hu: "); 
        Serial.println(hum);
        Serial.print("Tp: ");
        Serial.println(temF); 
        Serial.print("EC: ");
        Serial.println(ec);
        Serial.print("PH: ");
        Serial.println(ph);

        Serial.print(hum);
        Serial.print(",");
        Serial.print(temF);
        Serial.print(",");
        Serial.println(ec);

        processStep++;
      }
      if( millis() - startTime > 3000)
      {
        Serial.println("timeout");
        processStep = 0;
      }
      break;
    }
    default:
      delay(1000);
      processStep = 0;
      break;
  }
}
