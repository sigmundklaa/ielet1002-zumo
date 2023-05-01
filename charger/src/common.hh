
#ifndef COMMON
#define COMMON

/*
    common.hh: Header file that declares variables and functions used by main.cc and most other files. 
*/

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

// Variables:
    // BASICS
inline unsigned long previousMillis = 0;

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

    // Structs
    // Struct for customer data
inline struct __attribute__((packed)) Customer {
    int customer_id;
    int battery_level;
    int battery_health;
    int charging_cycles;
    float account_amount;
} c;

// Functions:
void setupPins();
void setupOled();
void setupReset();

void test_customer();
    // OLed
void print_to_display(String battery_level);

#endif // COMMON