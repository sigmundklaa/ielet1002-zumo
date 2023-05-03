#include <Arduino.h>

#include <common.hh>
#include <connection.hh>
#include <charger.hh>
#include <breaker.hh>
#include <payment.hh>

void setupPins()
{
    pinMode(confirm_button_pin, INPUT);
    pinMode(select_button_pin, INPUT);

    pinMode(ired_pin, INPUT);
    pinMode(ired_diode, OUTPUT); // Only used if diode is connected to charge station
}

void setupReset(){ // Resets variabels and set startup values
    resetVariables();
}

// SETUP
void setup()
{
    Serial.begin(9600);

    setupReset();

    setupPins();
    setupOled();

    print_to_display("");
    
    setupConnection();
}

// LOOP
void loop()
{
    loopStationCode(); // charger.cc
    //loopBreakerCode(); // breaker.cc
    loopConnectionCode(); // connection.cc
    loopOledCode(); // charger.cc
}