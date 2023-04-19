
#ifndef COMMON
#define COMMON

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Variables:
    // BASICS
inline unsigned long previousMillis = 0;

    // PINS
inline const int button_pin_1 = -1;
inline const int LED_pin_running = -1;
inline const int LED_pin_input = -1;
inline const int LED_pin_error = -1;

    // Status
inline bool customer = false;
inline bool ran_out = false;
inline float power_price = 0.00;

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

    // ESP-NOW
      // Router:
inline esp_now_peer_info_t routerDeviceInfo = {
    // Change this address to the address of the router ESP32
    .peer_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .channel = 0,
    .encrypt = false,
};


// Functions:


#endif