#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"


#define BME_SCK 25
#define BME_MISO 33
#define BME_MOSI 26
#define BME_CS 27
#define lightsensor 34

#define SEALEVELPRESSURE_HPA (1013.25)

float temperature;
float humidity;
float pressure;
float gasResistance;

Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);


struct SensorData {
  
  float temperature;
  float humidity;
  float pressure;
  int lightData;

};

SensorData sensorData;

void getBME680Readings(){
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  temperature = bme.temperature;
  pressure = bme.pressure / 100.0;
  humidity = bme.humidity;
  gasResistance = bme.gas_resistance / 1000.0;
}


void setup() {
  Serial.begin(9600);
  pinMode(lightsensor, INPUT);
  pinMode(BME_MISO, INPUT);
  pinMode(BME_CS, INPUT);
  pinMode(BME_SCK, INPUT);
  pinMode(BME_MOSI, INPUT);

  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

}

void loop() {

  getBME680Readings();

  sensorData.lightData = analogRead(lightsensor);
  sensorData.temperature = temperature;
  sensorData.humidity = humidity;
  sensorData.pressure = pressure;

  delay(2000);

  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Lightdata = ");
  Serial.println(sensorData.lightData);

  Serial.println();


}