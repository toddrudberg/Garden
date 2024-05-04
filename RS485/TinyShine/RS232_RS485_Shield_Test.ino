
/************************Copyright (c)*******************************************************************
*********************************************************************************************************
**                 合肥途思电子科技有限公司            
**                        研发中心                             
**                         硬件部                                
**                                                               
**                http://www.tinysine.com                                 
**
**-------------文件信息---------------------------------------------------------------------------------
**文   件   名：WiFi_Shield_Test
**创   建   人：黄超
**最后修改日期：2013年11月20日
**版        本：V1r0
**描        述：
**备        注: 
**              
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

/********************************************************************************************************
 * 										INCLUDE FILES
********************************************************************************************************/
#include <SoftwareSerial.h>

#define RS485_DIR_PIN     4

/********************************************************************************************************
 * 										VARIABLES DEFINITION
********************************************************************************************************/
unsigned char Cmd;
unsigned char i;
              
/********************************************************************************************************
 * 										FUNCTION PROTOTYPES
********************************************************************************************************/ 


SoftwareSerial mySerial(2, 3); // RX, TX

void setup()  
{
    pinMode(RS485_DIR_PIN, OUTPUT);             //Set RS485 direction pin as output
    digitalWrite(RS485_DIR_PIN, LOW);           //Set RS485 chip as receiver
    
    Serial.begin(9600);
    mySerial.begin(9600);

    while (!Serial) 
    {
        ; // wait for serial port to connect. Needed for Leonardo only
    }
    
    digitalWrite(RS485_DIR_PIN, HIGH);         //Set RS485 as transmitter
    //delay(10);
    //MCU send some message to wifi bee throutht software serial
    Serial.println("Hello,this is RS232&RS485 Shield testing program.");
    Serial.println("Provided by Tinysine");
    Serial.println("Now,we use the hardware serial port!");
    Serial.println("if OK,Please send '1' to begin test");
    
    mySerial.println("Hello,this is RS232&RS485 Shield testing program.");
    mySerial.println("Provided by Tinysine");
    mySerial.println("Now,we use the software serial port!");
    mySerial.println("if OK,Please send '1' to begin test");
}

void loop() // run over and over
{
    digitalWrite(RS485_DIR_PIN, LOW);         //Set RS485 as receiver 
    
    if (Serial.available()) 
    {
        Cmd = Serial.read();
        
        digitalWrite(RS485_DIR_PIN, HIGH);
        //delay(10);
        
        Serial.write(Cmd);
        
        //MCU get some message from wifi bee throutht hardware serial
        if('1'==Cmd) 
        {
            Serial.println(" ");
            Serial.println(" ");
            Serial.println("1**************************************************");
            Serial.println("0123456789");
            Serial.println("abcdefg hijklm nopqrst uvwxyz");
            Serial.println("The hardware serial port testing                OK!:");
            Serial.println("1**************************************************");
            //delay(1000);
            //Serial.end(); 
        }
    }  
    
    if (mySerial.available()) 
    {
        Cmd = mySerial.read();
        
        digitalWrite(RS485_DIR_PIN, HIGH);
        //delay(10);
        
        mySerial.write(Cmd);
        
        //MCU get some message from wifi bee throutht software serial
        if('1'==Cmd)   
        {
            mySerial.println(" ");
            mySerial.println(" ");
            mySerial.println("1**************************************************");
            mySerial.println("0123456789");
            mySerial.println("abcdefg hijklm nopqrst uvwxyz");
            mySerial.println("The software serial port testing                OK!:");
            mySerial.println("1**************************************************");
            //delay(1000);
            //mySerial.end(); 

        }
    } 
}
