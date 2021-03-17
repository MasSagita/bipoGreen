/*

  Program BIPOGREEN NodeMCU beta version
  
  Program untuk menerima data sensor bipo dari arduino
  dan mengirim ke database menggunakan http.POST

  https://github.com/MasSagita
  
*/



#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcdI2C(0x27, 16, 2);

String apiKeyValue = "tPmAT5Ab3j7F9";

const char* serverName = "http://192.168.1.6/learnphp/bipo_test/post-data.php";

// Declare the "link" serial port
// Please see SoftwareSerial library for detail
SoftwareSerial linkSerial(12, 13); // pin D6, D7, RX, TX

const int led = 16;
int countDisplay = 0;
int refresh = 0;

int countPostInterval = 2;

//Variabel untuk json doc
float dsValFromArdu, phValFromArdu, pressBarFromArdu;
float pressPsiFromArdu, MQmetanaValFromArdu, MQkarbonValFromArdu;
float dhtTempFromArdu, dhtHumiFromArdu;

int countPost;
int httpResponseCode;

void setup() {
  //setup ssid and pass
  const char* ssid = "Odading Mang Oleh";
  const char* pass = "magelangan";

  // Initialize "debug" serial port
  // The data rate must be much higher than the "link" serial port
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  lcdI2C.init();

  for (int i = 0; i < 5; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }

  refresh = -1;

  lcdI2C.backlight();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    lcdI2C.setCursor(0, 0);
    lcdI2C.print("Connecting");
    Serial.println("Connecting..");
    if (++refresh >= 0) {
      lcdI2C.setCursor(refresh, 1);
      lcdI2C.print(".");
      delay(100);
      if (refresh == 16) {
        refresh = -1;
        lcdI2C.clear();
      }
    }
    digitalWrite(led, digitalRead(led) ^ 1);
  }

  Serial.print((String)"Connected to " + ssid + "\t");
  Serial.println(WiFi.localIP());
  lcdI2C.clear();
  lcdI2C.setCursor(0, 0);
  lcdI2C.print("Connected:");
  lcdI2C.setCursor(0, 1);
  lcdI2C.print(WiFi.localIP());

  delay(1500);

  lcdI2C.clear();
  lcdI2C.setCursor(0, 0);
  lcdI2C.print(F("BiPo_Green"));
  lcdI2C.setCursor(0, 1);
  lcdI2C.print(F("Trial Version"));

  delay(1000);

  refresh = 0;

  lcdI2C.clear();
  while (!Serial) continue;
  // Initialize the "link" serial port
  // Use the lowest possible data rate to reduce error ratio
  linkSerial.begin(4800);
}

void loop() {
  if (linkSerial.available()) {
    countDisplay++;
    if (WiFi.status() == WL_CONNECTED) {

      HTTPClient http;
      http.begin(serverName);
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");


      // Allocate the JSON document
      // This one must be bigger than for the sender because it must store the strings
      StaticJsonDocument<500> doc;
      //deserializeJson(doc, json);

      // Read the JSON document from the "link" serial port
      DeserializationError err = deserializeJson(doc, linkSerial);

      if (err == DeserializationError::Ok)
      {

        if (++refresh < 2) {
          lcdI2C.clear();
          refresh = 0;
        }

        dsValFromArdu = doc["dsVal"].as<float>();
        phValFromArdu = doc["phVal"].as<float>();
        pressBarFromArdu = doc["pressBar"].as<float>();
        pressPsiFromArdu = doc["pressPsi"].as<float>();
        dhtTempFromArdu = doc["dhtTemp"].as<float>();
        dhtHumiFromArdu = doc["dhtHumi"].as<float>();
        MQmetanaValFromArdu = doc["mqMetana"].as<float>();

        /*
          String httpRequestData = "api_key=" + apiKeyValue + "&value1=" + String(dsValFromArdu)
                                 + "&value2=" + String(phValFromArdu) + "&value3=" + String(pressBarFromArdu)
                                 + "&value4=" + String(pressPsiFromArdu) + "";
        */

        String httpRequestData = "api_key=" + apiKeyValue
                                 + "&value1=" + String(phValFromArdu)         //ph
                                 + "&value2=" + String(dsValFromArdu)         //suhu ds
                                 + "&value3=" + String(dhtTempFromArdu)       //suhu dht
                                 + "&value4=" + String(pressBarFromArdu)
                                 + "&value5=" + String(pressPsiFromArdu)
                                 + "&value6=" + String(dhtHumiFromArdu)
                                 + "&value7=" + String(MQmetanaValFromArdu)
                                 + "&value8=" + String(MQkarbonValFromArdu)
                                 + "";

        if (++countPost > countPostInterval) {
          lcdI2C.setCursor(15, 0);
          lcdI2C.print("S");
          httpResponseCode = http.POST(httpRequestData);
          countPost = 0;
          if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
          }
          else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
            lcdI2C.setCursor(0, 1);
            lcdI2C.print("Send Failed");
          }
          http.end();
        }


        // Print the values
        // (we must use as<T>() to resolve the ambiguity)
        Serial.print("TX ->: ");
        Serial.print(countPost);
        Serial.print("\t DS: ");
        Serial.print(dsValFromArdu);
        Serial.print("\t pH: ");
        Serial.print(phValFromArdu);
        Serial.print("\t Bar: ");
        Serial.print(pressBarFromArdu);
        Serial.print("\t Psi: ");
        Serial.print(pressPsiFromArdu);
        Serial.print("\t dhtT: ");
        Serial.print(dhtTempFromArdu);
        Serial.print("\t dhtH: ");
        Serial.println(dhtHumiFromArdu);
        displayScreen();
      }
      else
      {
        // Print error to the "debug" serial port
        Serial.print("deserializeJson() returned ");
        Serial.println(err.c_str());
        lcdI2C.clear();
        lcdI2C.setCursor(0, 0);
        lcdI2C.print("Json returned");
        lcdI2C.setCursor(0, 1);
        lcdI2C.print("Invalid Input");
        // Flush all bytes in the "link" serial port buffer
        while (linkSerial.available() > 0)
          linkSerial.read();
      }
    }
    else {
      Serial.println("Error in WiFi connection");
    }
  }
  digitalWrite(led, digitalRead(led) ^ 1);
}

void displayScreen() {
  if (countDisplay >= 0 && countDisplay <= 9) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_DS18B20:"));
    lcdI2C.setCursor(0, 1); lcdI2C.print(F("Temp:")); lcdI2C.print(dsValFromArdu);
  }

  if (countDisplay >= 10 && countDisplay <= 19) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_pH:"));
    lcdI2C.setCursor(0, 1); lcdI2C.print(F("pHVal:"));  lcdI2C.print(phValFromArdu);
  }

  if (countDisplay >= 20 && countDisplay <= 29) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_Pressure:"));
    lcdI2C.setCursor(0, 1); lcdI2C.print(F("Bar:"));  lcdI2C.print(pressBarFromArdu);
  }

  if (countDisplay >= 30 && countDisplay <= 39) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_DHT11:"));
    lcdI2C.setCursor(0, 1);
    lcdI2C.print(F("T:")); lcdI2C.print(dhtTempFromArdu);
    lcdI2C.print(F(" H:"));  lcdI2C.print(dhtHumiFromArdu);  lcdI2C.print(F("%"));
  }

  if (countDisplay >= 40 && countDisplay <= 49) {
    lcdI2C.setCursor(0, 0); lcdI2C.print(F("BiPo_MQ-6:"));
    lcdI2C.setCursor(0, 1);
    lcdI2C.print(F("CH4:"));  lcdI2C.print(MQmetanaValFromArdu, 1);  lcdI2C.print(F("ppm"));
  }

  if (countDisplay > 49) countDisplay = 0;
}
