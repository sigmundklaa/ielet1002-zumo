#include <Arduino.h>
#define BME_SCK 25
#define BME_MISO 33
#define BME_MOSI 26
#define BME_CS 27

Adafruit_BME680 bme;
Adafruit_BME680 bme(BME_CS);
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);


struct SensorData {
  
  int tempData;
  int humidData;
  int lightData;

};

SensorData sensorData;


void setup() {
  Serial.begin(115200);
  pinMode(32, INPUT);
  pinMode(BME_MISO, INPUT);
  pinMode(BME_CS, INPUT);
  pinMode(BME_SCK, INPUT);
  pinMode(BME_MOSI, INPUT);

}

void loop() {
  Serial.println(sensorData.tempData);
  Serial.println(sensorData.lightData);

  delay(1000);
  sensorData.tempData = (analogRead(32)-500)/10;
  sensorData.humidData = analogRead(33);
  sensorData.lightData = analogRead(34);


}