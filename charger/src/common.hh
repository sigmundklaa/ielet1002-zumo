
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
inline unsigned long oled_millis = 0;

    // PINS
inline const int confirm_button_pin = 25;
inline const int select_button_pin = 26;
//OLED PINS 22 -> SCK, 21 -> SDA; // Set by library

    // Structs
    // Struct for customer data
inline struct __attribute__((packed)) Customer {
    int id;
    int batt_status;
    int batt_health;
    float account_amount;
} c;

// Functions:
void setupPins();
void setupOled();
void setupReset();

#endif // COMMON