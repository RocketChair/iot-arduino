#line 1 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
#line 1 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
#include <Arduino.h>
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module


#line 10 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
void setup();
#line 19 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
void loop();
#line 53 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
char checkValue(unsigned char *thebuf, char leng);
#line 71 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
int transmitPM01(unsigned char *thebuf);
#line 79 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
int transmitPM2_5(unsigned char *thebuf);
#line 87 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
int transmitPM10(unsigned char *thebuf);
#line 10 "/Users/sptq/Documents/Arduino/sketch_apr06a/sketch_apr06a.ino"
void setup()
{
  Serial.begin(9600);   //use serial0
  Serial1.begin(9600);   //use serial1
  Serial1.setTimeout(1500);
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
  //test
}

void loop()
{
  if(Serial1.find(0x42)){    //start to read when detect 0x42
    Serial1.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
    } 
  }

  static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0: ");  
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0: ");  
      Serial.print(PM10Value);
      Serial.println("  ug/m3");   
      Serial.println();
    }
  
}
char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}

