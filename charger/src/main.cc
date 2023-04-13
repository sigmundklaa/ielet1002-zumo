
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include <charge.cc>
#include <payment.cc>
#include <esp_now.cc>

void setupPins()
{
    pinMode(button_pin_1, INPUT);
}

// SETUP
void setup()
{
    Serial.begin(9600);
    setupEspNow();
    setupPins();
}

// LOOP
void loop()
{
    // Checks if there is a customer at the station
    if(c.customer_id != 0){
        customer = true;
    } else {
        customer = false;
    }

    switch(customer){
        case true:
            esp_now_register_recv_cb(process_customer);
            break;
        case false:
            break;           
    }
}