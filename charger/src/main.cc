#include <Arduino.h>

// Includes all header files, which gives access to the rest of the written code. 
#include <common.hh>
#include <connection.hh>
#include <charger.hh>
#include <payment.hh>

void setupPins()
{
    pinMode(confirm_button_pin, INPUT);
    pinMode(select_button_pin, INPUT);
}

void setupReset(){ // Resets variabels, only called on startup
    resetVariables();
}

// SETUP
void setup()
{
    Serial.begin(9600);

    // Setup functions
    setupReset();
    setupPins();
    setupOled();

    // Clear potential Oled text
    print_to_display("");
    
    // Setup wifi, done last to overwrite setup values if needed
    setupConnection();
}

// LOOP
void loop() // Core code that loops segment code, found in related .cc files. 
{
    loopStationCode(); // charger.cc
    loopConnectionCode(); // connection.cc
    loopOledCode(); // charger.cc
}