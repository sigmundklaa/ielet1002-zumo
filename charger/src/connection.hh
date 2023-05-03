
#ifndef CONNECTION
#define CONNECTION

/*
    connection.hh: Header file that declares variables and functions used mostly in connection.cc
*/

#include <common.hh>

// Variables:
    //WiFi
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

    //MQTT
#define MQTT_HOST IPAddress(192,168,137,XXX) // Usually only XXX needs to change
#define MQTT_PORT 1883

inline WiFiClient charging_station;
inline PubSubClient client(charging_station);


// Functions:
    // Setup
void setupConnection();
void loopConnectionCode();

    // MQTT
void mqtt_callback(char* topic, uint8_t* message, unsigned int length);
void mqtt_reconnect();

#endif // CONNECTION