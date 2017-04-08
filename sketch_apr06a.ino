#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes

//
// JSON BUILDER
//
StaticJsonBuffer<200> jsonBuffer;
JsonObject& jsonObject = jsonBuffer.createObject();

//
// LED Section 
//
#define LED_PIN  7
#define NUMPIXELS 7
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

//
// DUST SECTION
//
unsigned char buf[LENG];
int PM01Value = 0;
int PM2_5Value = 0;
int PM10Value = 0;

//
// TEMPERATURE
//
#define DHT_PIN 2
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

//
// BUTTON
//
#define BUTTON_PIN 4
int buttonState = 0;

//
// LIGHT
//
#define LIGHT_PIN 5
int lightState = 0;
//
// COMMANDS SECTION
//
int MAX_CMD_LENGTH = 32;
char cmd[32];
int cmdIndex;
char incomingByte;

//
// Conectivity - to node, socket, http 
//
int CONNECTING_STATUS = 0;


///-----------------------------


/**
 * DUST SECTION START 
 */

char checkValue(unsigned char *thebuf, char leng) {
  char receiveflag = 0;
  int receiveSum = 0;

  for (int i = 0; i < (leng - 2); i++) {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf) {
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

int transmitPM2_5(unsigned char *thebuf) {
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); //count PM2.5 value of the air detector module
  return PM2_5Val;
}

int transmitPM10(unsigned char *thebuf) {
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}

void readDust(JsonObject& json) {
  /**
   * Dust sensor read
   */
  if (Serial1.find(0x42)) {  //start to read when detect 0x42
    Serial1.readBytes(buf, LENG);

    if (buf[0] == 0x4d) {
      if (checkValue(buf, LENG)) {
        PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value = transmitPM2_5(buf); //count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }

  static unsigned long OledTimer = millis();
  if (millis() - OledTimer >= 1000)
  {
    OledTimer = millis();

    json["PM01Value"] = PM01Value;
    json["PM2_5Value"] = PM2_5Value;
    json["PM10Value"] = PM10Value;
  }
}

/**
 * DUST END
 */

///----------------------------- 

/**
 * TEMPERATURE START
 */

void readTemperature(JsonObject& json) {
  delay(500);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) || !isnan(humidity)) {
    json["temperature"] = temperature;
    json["humidity"] = humidity;
  }
}

/**
 * TEMPERATURE END
 */

///----------------------------- 

/**
 * BUTTON START
 */

void readButton(JsonObject& json) {
  buttonState = !digitalRead(BUTTON_PIN);
  json["rocketChair"] = buttonState;
}

/**
 * BUTTON END
 */

///----------------------------- 

/**
 * LIGHT START
 */

void readLight(JsonObject& json) {
  lightState = analogRead(LIGHT_PIN);
  json["light"] = lightState;
}

/**
 * LIGHT END
 */

///----------------------------- 

/**
 * LED SECTION START
 */
void clearAllLEDS() {
  for(int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();
  }
}

void cmd_connecting() {
  for(int j = 0; j < 4; j++) {
    for(int i = 1; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0,150,0));
      pixels.show();
      delay(200);
      pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
  } 
}

void cmd_connected() {
  pixels.setBrightness(10);
  for(int i = 1; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(215,45,109));
    pixels.show();
  }
  delay(2000);
}

void cmd_alert() {
  pixels.setBrightness(75);
  clearAllLEDS();

  pixels.setPixelColor(1, pixels.Color(255,35,1));
  pixels.setPixelColor(3, pixels.Color(255,35,1));
  pixels.setPixelColor(5, pixels.Color(255,35,1));
  pixels.show();
  delay(250);

  pixels.setPixelColor(1, pixels.Color(0,0,0));
  pixels.setPixelColor(3, pixels.Color(0,0,0));
  pixels.setPixelColor(5, pixels.Color(0,0,0)); 
  pixels.show();
  delay(250);

  pixels.setPixelColor(2, pixels.Color(255,35,1));
  pixels.setPixelColor(4, pixels.Color(255,35,1));
  pixels.setPixelColor(6, pixels.Color(255,35,1));
  pixels.show();
  delay(250);

  pixels.setPixelColor(2, pixels.Color(0,0,0));
  pixels.setPixelColor(4, pixels.Color(0,0,0));
  pixels.setPixelColor(6, pixels.Color(0,0,0)); 
  pixels.show();
}

void cmd_ok() {
  for(int i = 1; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0,150,0));
  }
  pixels.show();
}

void cmd_dataReading() {
  pixels.setBrightness(10);
  pixels.setPixelColor(0, pixels.Color(34,113,179));
  pixels.show();
  delay(200);
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.show();
  delay(200);
}

void cmd_standBy() {
  cmd_ok();

  for (int i = 1; i < 50; i++) {
    pixels.setBrightness(i);
    pixels.show();
    delay(50);
  }

  for (int i = 50; i > 0; i--) {
    pixels.setBrightness(i);
    pixels.show();
    delay(50);
  }
}

/**
 * LED SECTION END
 */


///-----------------------------

void responseGenerator() {
  readTemperature(jsonObject);
  readDust(jsonObject);
  readButton(jsonObject);
  readLight(jsonObject);

  delay(50);
  jsonObject.printTo(Serial);
  Serial.println();
}


void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial1.setTimeout(1500);
  Serial.setTimeout(1500);
  
  //INIT LEDS
  pixels.begin();

  //INIT TEMPERATURE
  dht.begin();

  //INIT BUTTON
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //SET VATIABLES
  cmdIndex = 0;
}

void readCommands() {
  if (incomingByte = Serial.available() > 0) {
    char byteIn = Serial.read();
    cmd[cmdIndex] = byteIn;

    if(byteIn=='\n'){
      cmd[cmdIndex] = '\0';
      Serial.println(cmd);
      cmdIndex = 0;

      if(strcmp(cmd, "CMD:CONNECTING")  == 0){
        cmd_connecting();
      } else if (strcmp(cmd, "CMD:CONNECTED")  == 0) {
        cmd_connected();
      } else if (strcmp(cmd, "CMD:STANDBY")  == 0) {
        cmd_standBy();
      } else if (strcmp(cmd, "CMD:ALERT")  == 0) {
        cmd_alert();
      } else if (strcmp(cmd, "CMD:OK")  == 0) {
        cmd_ok();
      } else if (strcmp(cmd, "CMD:READ")  == 0) {
        cmd_dataReading();
      } else if (strcmp(cmd, "CMD:CLEAR")  == 0) {
        clearAllLEDS();
      }
    } else {
      if(cmdIndex++ >= MAX_CMD_LENGTH){
        cmdIndex = 0;
      }
    }
  }
}


void loop()
{
  readCommands();
  responseGenerator();

  
  //delay(1000);

}

