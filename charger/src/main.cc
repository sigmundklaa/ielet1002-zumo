#include <Arduino.h>

#include <common.hh>
#include <esp_now.hh>
#include <charger.hh>

void setupPins()
{
    //pinMode(button_pin_1, INPUT);
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
    loopStationCode();
}