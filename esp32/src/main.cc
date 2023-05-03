//Inkluderer nødvendige bibliotek
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <io/mqtt.hh>
#include <WiFi.h>


static io::mqtt_gateway mqtt(&io::mqtt_client, "/redmw/sensor/1", nullptr);

//Definerer en struct som skal sendes over NodeRED
struct __attribute__ ((packed)) SensorData {
  float temperature;
  float humidity;
  float pressure;
  int lightData;
};

SensorData sensorData;

//Definerer pin-navn
#define BME_SCK 25
#define BME_MISO 33
#define BME_MOSI 26
#define BME_CS 27
#define lightsensor 34

//Definerer variabler som sensorene skal lagre dataene til
float temperature;
float humidity;
float pressure;
float gasResistance;
int period = 10000;
unsigned long time_now = 0;

//Sier til bme688 hva pin-navnene den skal bruke heter
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

//Funksjon som henter sensordata
void getBME688Readings(){
  //Forteller BME688 at den skal begynne å lese av data
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
}

//Callback funksjon som sjekker om knappen på NodeRED dashbordet blir trykket, og hvis den blir det vil den sende sensordataen.
void callback(char* topic, uint8_t* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

   for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

   if(String(topic) == "/sensor/get"){
    if(messageTemp == "data"){
      Serial.print("Sending sensordata...");
      mqtt.write(&sensorData, sizeof(sensorData));
      Serial.println("SensorData sent");
    }
  }
  Serial.println();
}

//Reconnect funksjon for mqtt
static void reconnect(PubSubClient& client) {
    while (!client.connected()) {
        if (!client.connect("esp32")) {
            delay(500);
        }
    }

    client.subscribe("/sensor/get");
}

void setup() {
  //Serial og pindefinisjon
  Serial.begin(9600);
  pinMode(lightsensor, INPUT);
  pinMode(BME_MISO, INPUT);
  pinMode(BME_CS, INPUT);
  pinMode(BME_SCK, INPUT);
  pinMode(BME_MOSI, INPUT);

  //BME680 Oppsett:
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

//Kobler på samme WiFi som nodeRED er på
  WiFi.begin("WodanSurface", "53728431");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
//definerer hvilken mqtt server som dataen skal sendes til
  io::mqtt_client.setServer("192.168.137.226", 1883);
  io::mqtt_client.setCallback(callback);

}

void loop() {
  reconnect(io::mqtt_client);
  getBME688Readings();

//lagrer sensordataene til structen
  sensorData.lightData = analogRead(lightsensor);
  sensorData.temperature = temperature;
  sensorData.humidity = humidity;
  sensorData.pressure = pressure;

//Sender oppdaterte sensordataer hvert tiende sekund
 if(millis() >= time_now + period){
        time_now += period;
          //Printer dataene til Serial for enkel dataovervåking
          Serial.print("Temperature = ");
          Serial.print(sensorData.temperature);
          Serial.println(" *C");

          Serial.print("Pressure = ");
          Serial.print(sensorData.pressure);
          Serial.println(" hPa");

          Serial.print("Humidity = ");
          Serial.print(sensorData.humidity);
          Serial.println(" %");

          Serial.print("Lightdata = ");
          Serial.println(sensorData.lightData);

          Serial.println();
          //sender structen
          mqtt.write(&sensorData, sizeof(sensorData));
    }
  io::mqtt_client.loop();

}