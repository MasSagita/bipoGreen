/*

  Program BIPOGREEN Arduino Nano beta version
  
  Program untuk membaca data sensor bipo dan mengirim ke NodeMCU
  dan mengirim ke database menggunakan http.POST
  
  https://github.com/MasSagita
  
*/



#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <AverageValue.h>

// Declare the "link" serial port
SoftwareSerial linkSerial(6, 5); //RX,TX

#include <dht11.h>
#include <Wire.h>
#define DHT11_PIN     11
dht11 DHT;

#define ONE_WIRE_BUS  10  //Pin data DS18B20 1
OneWire ds(ONE_WIRE_BUS);

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcdI2C(0x3F, 16, 2);

//Rotary Encoder PIN
#define pinRotaryEncoderCLK     2
#define pinRotaryEncoderDT      3
#define pinRotaryEncoderSwitch  4

//Variables for Rotary Encoder
const int buttonPin = pinRotaryEncoderSwitch; // the number of the pushbutton pin
const int SHORT_PRESS_TIME = 800; // 1000 milliseconds
const int LONG_PRESS_TIME  = 800; // 1000 milliseconds

//Variables for switch Rotary Encoder
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;
bool longPressBtn = false;
bool shortPressBtn = false;
long pressDuration;

//Variables Rotary Switch will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//Variables for set interrupt encoder
uint8_t maskSensorA;
uint8_t maskSensorB;
uint8_t *pinSensorA;
uint8_t *pinSensorB;
volatile bool encoderAFlag = 0;
volatile bool encoderBFlag = 0;
int8_t nilaiEncoder = 0;
int nilaiSetting[4];
int countEcd;
String currentDir = "";

//Varibles for ph sensor
const int phPin = A1;
int samples = 10;
float adc_resolution = 1024.0;
float phCalib = -4.5;
float phVal;
float phVolt;
int phMeasurings;

//Variables for pressure sensor
const int pressPin = A0;
int pressVal;
float pressPascal;
float pressBar;
float pressPsi;
float pressVolt;
float pressCalib = 0.1;

float dsVal; //Store ds18b20 value
float dht11Val; //Store DHT11 value

//Variables for MQ sensor
int MQ135Pin = A2;
int MQRload = 9980;
float MQrO = 66000;
int MQadcRaw;
double MQrS;
float MQrSrO;
double MQppm = 1915;
float MQa = 2194.08;
float MQb = -2.498;
float MQminppm = 0;
float MQmaxppm = 0;

float MQmetanaVal; //Store metana value
float MQcarbonVal; //Store carbon value

const long MQ_MAX_VALUES_NUM = 10;
AverageValue<long> averageValue(MQ_MAX_VALUES_NUM); //Store Average Value

const int BUZZER_PIN = 12; //Buzzer pin
const int ledPin = 13; //Led pin

//Variables for menu
int countSend;
int manualScreen;
int countDisplay = 0;
int menu = 0;
int cursorSet = 0;
int cursorState;
//Variables for menu
static bool isMenu = false;
static bool isPhSetup = false;
static bool isScreenSetup = false;
static bool isPressSetup = false;
static bool isManualScreen = false;
static bool isJsonSend = false;
//Variables for menu
String screenSetup = "";
String jsonStatus = "";

int refresh = 0; //For refresh screen

float dallas(OneWire& ds, byte start = false) {
  int16_t temp;
  do {
    ds.reset();
    ds.write(0xCC);
    ds.write(0xBE);
    ds.read_bytes((uint8_t*) &temp, sizeof(temp));
    ds.reset();
    ds.write(0xCC);
    ds.write(0x44, 1);
    if (start) delay(1000);
  } while (start--);
  return (temp * 0.0625);
}

void setup() {
  // Initialize "debug" serial port
  // The data rate must be much higher than the "link" serial port
  Serial.begin(115200);
  lcdI2C.init();

  while (!Serial) continue;
  // Initialize the "link" serial port
  // Use the lowest possible data rate to reduce error ratio
  linkSerial.begin(4800);

  Serial.println("-------BGV1-------");
  dallas(ds, true);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ledPin, OUTPUT);

  pinMode(MQ135Pin, INPUT);
  pinMode(pinRotaryEncoderCLK, INPUT_PULLUP);
  pinMode(pinRotaryEncoderDT, INPUT_PULLUP);
  pinMode(pinRotaryEncoderSwitch, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinRotaryEncoderCLK), encoderARising, RISING);
  attachInterrupt(digitalPinToInterrupt(pinRotaryEncoderDT), encoderBRising, RISING);

  maskSensorA  = digitalPinToBitMask(pinRotaryEncoderCLK);
  pinSensorA = portInputRegister(digitalPinToPort(pinRotaryEncoderCLK));
  maskSensorB  = digitalPinToBitMask(pinRotaryEncoderDT);
  pinSensorB = portInputRegister(digitalPinToPort(pinRotaryEncoderDT));

  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, 2394, 125); //2394
    digitalWrite(ledPin, 1);
    delay(130);
    tone(BUZZER_PIN, 3136, 125); //3136
    digitalWrite(ledPin , 0);
    delay(125);
  }

  //set limit for MQ sensor
  //min[Rs/Ro]=(max[ppm]/a)^(1/b)
  MQminppm = pow((10000 / MQa), 1 / MQb);
  //max[Rs/Ro]=(min[ppm]/a)^(1/b)
  MQmaxppm = pow((100 / MQa), 1 / MQb);

  manualScreen = 0;
  screenSetup = "Auto";
  jsonStatus = "ON";

  longPressBtn = true;
  lcdI2C.backlight();
  lcdI2C.clear();
  lcdI2C.setCursor(0, 0);
  lcdI2C.print(F("BiPo_Green"));
  lcdI2C.setCursor(0, 1);
  lcdI2C.print(F("Trial Version"));
  delay(1000);

}

float ph (float voltage) {
  return 7 + ((2.5 - voltage) / 0.18);
}

void loop() {

  isMenu = false;
  isPhSetup = false;
  isScreenSetup = false;
  isPressSetup = false;
  isJsonSend = false;

  getRotaryBtn();

  hitungSuhuDS();
  hitungTekanan();
  hitungPH();
  hitungDHT11();
  hitungMQ();

  if (screenSetup == "Auto") {
    isManualScreen = false;
    if (isManualScreen == false) {
      countDisplay++;
      manualScreen = 10;
      if (++refresh > 1) {
        lcdI2C.clear();
        refresh = 0;
      }
      lcdI2C.setCursor(15, 0);
      lcdI2C.print(F("A"));
      displayScreen();
    }
  }

  if (screenSetup == "Manual") {
    isManualScreen = true;
    if (++refresh > 1) {
      lcdI2C.clear();
      refresh = 0;
    }
    lcdI2C.setCursor(15, 0);
    lcdI2C.print(F("M"));
    countDisplay = -1;
    if (manualScreen > 4) manualScreen = 0;
    if (manualScreen < 0) manualScreen = 4;
    displayScreen();
  }

  if (shortPressBtn) {
    displayMenu();
    countDisplay = 0;
  }

  StaticJsonDocument<500> doc;
  doc["dsVal"] = dsVal;
  doc["phVal"] = phVal;
  doc["pressBar"] = pressBar;
  doc["pressPsi"] = pressPsi;
  doc["dhtTemp"] = DHT.temperature;
  doc["dhtHumi"] = DHT.humidity;
  doc["mqMetana"] = MQmetanaVal;
  doc["mqCarbon"] = MQcarbonVal;

  // Send the JSON document over the "link" serial port
  serializeJson(doc, linkSerial);

  Serial.println((String) countSend + "\t" + screenSetup + "\t"
                 + countDisplay + "\t" + manualScreen);
}

void displayScreen() {

  if (countDisplay >= 0 && countDisplay <= 9 || manualScreen == 0) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_DS18B20:"));
    lcdI2C.setCursor(0, 1); lcdI2C.print(F("Temp:")); lcdI2C.print(dsVal);
  }

  if (countDisplay >= 10 && countDisplay <= 19 || manualScreen == 1) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_pH:"));
    lcdI2C.setCursor(0, 1); lcdI2C.print(F("pHVal:"));  lcdI2C.print(phVal);
  }

  if (countDisplay >= 20 && countDisplay <= 29 || manualScreen == 2) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_Pressure:"));
    lcdI2C.setCursor(0, 1); lcdI2C.print(F("Bar:"));  lcdI2C.print(pressBar);
  }

  if (countDisplay >= 30 && countDisplay <= 39 || manualScreen == 3) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_DHT11:"));
    lcdI2C.setCursor(0, 1);
    lcdI2C.print(F("T:")); lcdI2C.print(DHT.temperature, 1);
    lcdI2C.print(F(" H:"));  lcdI2C.print(DHT.humidity, 1);  lcdI2C.print(F("%"));
  }

  if (countDisplay >= 40 && countDisplay <= 49 || manualScreen == 4) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_MQ-6:"));
    lcdI2C.setCursor(0, 1);
    lcdI2C.print(F("CH4:"));  lcdI2C.print(MQmetanaVal, 1);  lcdI2C.print(F("ppm"));
  }

  if (countDisplay > 49) countDisplay = 0;
}

void displayMenu() {
  isMenu = true;
  isPhSetup = false;
  isScreenSetup = false;
  isPressSetup = false;
  isJsonSend = false;
  isManualScreen = false;
  menu = 0;
  lcdI2C.clear();
  lcdI2C.setCursor(0, 0);
  lcdI2C.print(F("Setup Screen"));
  
  tone(BUZZER_PIN, 3136, 125); //3136
  digitalWrite(ledPin, 1);
  delay(200);
  tone(BUZZER_PIN, 2394, 125); //2394
  digitalWrite(ledPin , 0);
  delay(300);
  
  delay(500);

  while (1) {
    getRotaryBtn();
    if (ledState == 0) {
      isMenu = false;
      cursorSet = 9;
    }
    if (ledState == 1) {
      isMenu = true;
      isPhSetup = false;
      isScreenSetup = false;
      isPressSetup = false;
      isJsonSend = false;
      cursorSet = 0;
    }

    if (++refresh > 5) {
      lcdI2C.clear();
      refresh = 0;
    }
    //Serial.println(menu);

    if (menu > 4) menu = 0;
    if (menu < 0) menu = 4;

    if (menu == 0) {
      isScreenSetup = true;
      isPhSetup = false;
      isPressSetup = false;
      isJsonSend = false;

      lcdI2C.setCursor(0, 0);
      lcdI2C.print(menu); lcdI2C.print(F(".Screen Setup"));

      lcdI2C.setCursor(cursorSet, 1);
      lcdI2C.print(F(">"));
      lcdI2C.setCursor(1, 1);
      lcdI2C.print("Scroll");
      lcdI2C.setCursor(10, 1);
      lcdI2C.print(screenSetup);
    }

    if (menu == 1) {
      isScreenSetup = false;
      isPhSetup = true;
      isPressSetup = false;
      isJsonSend = false;

      lcdI2C.setCursor(0, 0);
      lcdI2C.print(menu); lcdI2C.print(F(".pH Setup"));

      lcdI2C.setCursor(cursorSet, 1);
      lcdI2C.print(F(">"));
      lcdI2C.setCursor(1, 1);
      lcdI2C.print("calVal:");
      lcdI2C.setCursor(10, 1);
      lcdI2C.print(phCalib);
    }

    if (menu == 2) {
      isScreenSetup = false;
      isPhSetup = false;
      isPressSetup = true;
      isJsonSend = false;

      lcdI2C.setCursor(0, 0);
      lcdI2C.print(menu); lcdI2C.print(F(".Press Setup"));

      lcdI2C.setCursor(cursorSet, 1);
      lcdI2C.print(F(">"));
      lcdI2C.setCursor(1, 1);
      lcdI2C.print("calVal:");
      lcdI2C.setCursor(10, 1);
      lcdI2C.print(pressCalib);
    }

    if (menu == 3) {
      isPhSetup = false;
      isScreenSetup = false;
      isPressSetup = false;
      isJsonSend = true;

      lcdI2C.setCursor(0, 0);
      lcdI2C.print(menu); lcdI2C.print(F(".JSON Send"));

      lcdI2C.setCursor(cursorSet, 1);
      lcdI2C.print(F(">"));
      lcdI2C.setCursor(1, 1);
      lcdI2C.print("Status:");
      lcdI2C.setCursor(10, 1);
      lcdI2C.print(jsonStatus);

    }

    if (menu == 4) {
      lcdI2C.setCursor(0, 0);
      lcdI2C.print(menu); lcdI2C.print(F(".versions"));
      lcdI2C.setCursor(0, 1);
      lcdI2C.print("BiPo_Green V0.1");
    }

    if (longPressBtn) {
      lcdI2C.clear();
      lcdI2C.setCursor(0, 0);
      lcdI2C.print("Saving..");
      tone(BUZZER_PIN, 2394, 125); //2394
      digitalWrite(ledPin, 1);
      delay(200);
      tone(BUZZER_PIN, 3136, 125); //3136
      digitalWrite(ledPin , 0);
      delay(300);
      delay(500);
      break;
    }
  }
}

void getRotary() {

}

void getRotaryBtn() {

  currentState = digitalRead(buttonPin);

  // If the switch changed, due to noise or pressing:
  if (currentState != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (currentState != buttonState) {
      buttonState = currentState;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  lastButtonState = currentState;

  if (lastState == HIGH && currentState == LOW) {       // button is pressed
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  } else if (lastState == LOW && currentState == HIGH) { // button is released
    isPressing = false;
    releasedTime = millis();

    pressDuration = releasedTime - pressedTime;

    if ( pressDuration < SHORT_PRESS_TIME ) {
      longPressBtn = false;
      shortPressBtn = true;
      Serial.print((String)"A short press is detected \t");
      Serial.println((String) longPressBtn + shortPressBtn);
    }
  }

  if (isPressing == true && isLongDetected == false) {
    pressDuration = millis() - pressedTime;

    if ( pressDuration > LONG_PRESS_TIME ) {
      longPressBtn = true;
      shortPressBtn = false;
      Serial.print((String)"A long press is detected \t");
      Serial.println((String) longPressBtn + shortPressBtn);
      isLongDetected = true;
    }
  }
  digitalWrite(ledPin, isLongDetected);
  // save the the last state
  lastState = currentState;
}

void hitungSuhuDS() {
  dsVal = dallas(ds);
}

void hitungTekanan() {
  pressVal = analogRead(pressPin);
  pressVolt = (pressVal * 5.0) / adc_resolution;
  pressPascal = (3.0 * (pressVolt - 0.53)) * 1000000.0;
  pressBar = pressPascal / 10e5;
  pressPsi = pressBar * 14.5038;
}

void hitungPH() {
  phMeasurings = 0;
  for (int i = 0; i < samples; i++) {
    phMeasurings += analogRead(phPin);
    delay(10);
  }
  phVolt = 5 / adc_resolution * phMeasurings / samples;
  phVal = ph(phVolt) + phCalib;
}

void hitungDHT11() {
  dht11Val = DHT.read(DHT11_PIN);
}

void hitungMQ() {
  MQadcRaw = analogRead(MQ135Pin);
  MQrS = ((1024.0 * MQRload) / MQadcRaw) - MQRload;
  MQrSrO = MQrS / MQrO;

  float MQppm = MQa * pow((float)MQrS / (float)MQrO, MQb);
  averageValue.push(MQppm);
  MQmetanaVal = averageValue.average();
}

void serialDisplay() {
  Serial.print("Data to NMCU -> sensorDS: ");
  Serial.print(dsVal); // index 0 adalah ic pertama
  Serial.print("\t phVal: ");
  Serial.print(phVal);
  Serial.print("\t Bar: ");
  Serial.print(pressBar);
  Serial.print("\t Psi: ");
  Serial.println(pressPsi);
  Serial.println("-----------------------------------------");
}

void jsonSend() {
  // Create the JSON document

}

void encoderARising() {
  if ((*pinSensorA & maskSensorA) &&  (*pinSensorB & maskSensorB) && encoderAFlag)
  {
    nilaiEncoder = -1;
    countEcd --;
    currentDir = "CCW";
    if (isMenu)menu --;
    else if (isPhSetup)phCalib -= 0.50;
    else if (isPressSetup)pressCalib -= 0.50;
    else if (isManualScreen)manualScreen--;
    else if (isScreenSetup)screenSetup = "Manual";
    else if (isJsonSend)jsonStatus = "OFF";
    encoderAFlag = false;
    encoderBFlag = false;
  }
  else if (*pinSensorA & maskSensorA)
  {
    encoderBFlag = true;
  }
  EIFR = 0xFF;
}

void encoderBRising() {
  if ((*pinSensorA & maskSensorA) &&  (*pinSensorB & maskSensorB) && encoderBFlag)
  {
    nilaiEncoder = 1;
    countEcd ++;
    currentDir = "CW";
    if (isMenu)menu ++;
    else if (isPhSetup)phCalib += 0.50;
    else if (isPressSetup)pressCalib += 0.50;
    else if (isManualScreen)manualScreen++;
    else if (isScreenSetup)screenSetup = "Auto";
    else if (isJsonSend)jsonStatus = "ON";
    encoderAFlag = false;
    encoderBFlag = false;
  }
  else if (*pinSensorB & maskSensorB)
  {
    encoderAFlag = true;
  }
  EIFR = 0xFF;
}

