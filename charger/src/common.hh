
#ifndef COMMON
#define COMMON

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Variables:
    // BASICS
inline unsigned long previousMillis = 0;
inline int debounce_delay = 50;
inline unsigned long select_button_timer = 0;
inline unsigned long confirm_button_timer = 0;

    // OLED
#define SCREEN_WIDTH_ 128
#define SCREEN_HEIGHT_ 64
#define OLED_RESET_ -1
#define SCREEN_ADDRESS_ 0x3C
static Adafruit_SSD1306 display(SCREEN_WIDTH_, SCREEN_HEIGHT_, &Wire, OLED_RESET_);


    // PINS
inline const int confirm_button_pin = 25;
inline const int select_button_pin = 26;
//OLED PINS 22 -> SCK, 21 -> SDA;

    // Status
inline bool customer_waiting = false;
inline int customer_order = 0;

    // Structs
    // Struct for customer data
inline struct __attribute__((packed)) Customer {
    int customer_id;
    int battery_level;
    int battery_health;
    int charging_cycles;
    float account_balance;
    float credit;
} c;

    //WiFi
#define WIFI_SSID "WodanSurface"
#define WIFI_PASSWORD "Password"

    //MQTT
#define MQTT_HOST IPAdress(192,168,137,XXX)
#define MQTT_PORT 1883

#define MQTT_SUB_powerPrice "esp32/powerPrice/#"

    // ESP-NOW
      // Router:
inline esp_now_peer_info_t routerDeviceInfo = {
    // Change this address to the address of the router ESP32
    .peer_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .channel = 0,
    .encrypt = false,
};


// Functions:
void setupPins();
void setupOled();
void test_customer();

#endif