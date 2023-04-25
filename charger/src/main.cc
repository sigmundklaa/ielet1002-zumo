#include <Arduino.h>

#include <common.hh>
#include <esp_now.hh>
#include <charger.hh>

void setupPins()
{
    pinMode(confirm_button_pin, INPUT);
    pinMode(select_button_pin, INPUT);
}

void setupOled()
{
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_);
    display.display();
    delay(2000);
}

void setupConnection(){
    Serial.println("Connecting to WiFi.");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

}

// Creates a test customer for testing
void test_customer(){
    c.customer_id = 1;
    c.battery_level = 10;
    c.battery_health = 1;
    c.charging_cycles = 0;
    c.account_balance = 200;
    c.credit = 0;

    customer_waiting = true;

    Serial.println("Test customer has arrived.");
}

// SETUP
void setup()
{
    Serial.begin(9600);
    
    setupEspNow();
    setupPins();
    setupOled();
    resetVariables();

    // test_customer();
}

// LOOP
void loop()
{
    loopStationCode();
}