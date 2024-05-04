#include "IO.h"


class cSoilSensor
{
private:
    uint8_t Com[8] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09 }; //Command to read data


public:
    cSoilSensor() {
        // Initialization code here, if needed
    }

  void SetupSoilSensor()
  {
      mySerial.begin(baud);
    pinMode(RS485_TX_Enable,OUTPUT);
    digitalWrite(RS485_TX_Enable, LOW);  
  }
  void CheckSoilSensor()
  {
    static int processStep = 0;
      static unsigned long startTime = millis();  
    int temp;
    switch(processStep)
    {
      case 0:
      {
        digitalWrite(RS485_TX_Enable, HIGH);    //Enable high, RS485 shield waiting to transmit data
        for (int i = 0; i < 8; i++) 
        {
          mySerial.write(Com[i]);
        }
        digitalWrite(RS485_TX_Enable,LOW);    //Enable low, RS485 shield waiting to receive data
        startTime = millis();
        processStep++;
        break;
      }
      case 1:
      {
        if(millis() - startTime > 100) //10ms isn't enough time and we aren't in a hurry.  so 100ms. 
        {
          startTime = millis();
          processStep++;
        }
        break;
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
            // Serial.print(temp);
          }

          // Serial.println();
          // Serial.print("bytes read: ");
          // Serial.println(i);
          int offset = i - 10;
          float hum = ((Data[0+offset] << 8) + Data[1+offset]) / 10.00;
          float temC = ((Data[2+offset] << 8) + Data[3+offset]) / 10.00;
          float ec = ((Data[4+offset] << 8) + Data[5+offset]);
          float ph = ((Data[6+offset] << 8) + Data[7+offset]) /10.00;
          float temF = temC * 9.0/5.0 + 32.0;
          
          // Serial.println();
          // Serial.print("Hu: "); 
          // Serial.println(hum);
          // Serial.print("Tp: ");
          // Serial.println(temF); 
          // Serial.print("EC: ");
          // Serial.println(ec);
          // Serial.print("PH: ");
          // Serial.println(ph);
          

          Serial.print(hum);
          Serial.print(",");
          Serial.print(temF);
          Serial.print(",");
          Serial.println(ec);
          
          startTime = millis();
          processStep++;
        }
        if( millis() - startTime > 3000)
        {
          Serial.println("timeout");
          processStep = 0;
        }
        break;
      }
      case 3:
      {
        if( millis() - startTime > 1000)
        {
          processStep = 0;
        }
        break;
      }
      default:
      {
        processStep = 0;
        break;
      }
    }
  }
};



