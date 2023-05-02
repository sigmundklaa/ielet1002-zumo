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

void setupOled()
{
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_);
    display.display();
    delay(2000);
}

void setupReset(){ // Resets variabels and set startup values
    resetVariables();
    battery_price = 300;
    desired_charge = 255;
    auto_mode = true;
}

// SETUP
void setup()
{
    Serial.begin(9600);

    setupReset();

    setupPins();
    setupOled();

    setupConnection();
}

// LOOP
void loop()
{
    loopStationCode(); // charger.cc
    //loopBreakerCode(); // breaker.cc
    loopConnectionCode(); // connection.cc
}

// OLED

void print_to_display(String str)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    display.println(str);
    display.display();
};