
#include <common.hh>
#include <connection.hh>
#include <charger.hh>
#include <payment.hh>
#include <breaker.hh>

/*
    connection.cc: File containing code for connection between charging station and Node-red
*/  

void setupConnection(){ // Initiates WiFi and MQTT
    Serial.println("Connecting to WiFi.");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    client.setServer(MQTT_HOST, MQTT_PORT);
    client.setCallback(mqtt_callback);
}

void loopConnectionCode(){
    if (!client.connected()) {
    mqtt_reconnect();
    }
    client.loop();
}

// MQTT
void mqtt_callback(char* topic, u_int8_t* message, unsigned int length){
    Serial.print("Topic: "); Serial.println(topic); 
    String messageTemp;
    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }

    if(String(topic) ==  "/maintenance/price/out"){
        Serial.print("New Power Price: "); Serial.println(messageTemp);
        float message_float = messageTemp.toFloat();
        power_price = message_float;
    }

    if(String(topic) == "/maintenance/order/desired/out"){
        Serial.print("New desired charge: "); Serial.println(messageTemp);
        int message_int = messageTemp.toInt();
        desired_charge = message_int;
    }  

    if(String(topic) == "/breaker/out"){
        Serial.print("Breaker state: "); Serial.println(messageTemp);
        if(messageTemp == "True"){
            build_phase = false;
        }
    }

    if(String(topic) == "/bank/details/out"){
        Serial.print("Account amount: "); Serial.println(messageTemp);
        float message_float = messageTemp.toFloat();
        account_amount = message_float;
    }
    
    if(String(topic) == "/charger/order/mode"){
        Serial.print("Charger station mode: "); Serial.println(messageTemp);
        if(messageTemp == "True"){
            auto_mode = false;
        }
    }

    if(String(topic) == "/charger/order/account"){
        Serial.print("Bank Account: "); Serial.println(messageTemp);
        float message_float = messageTemp.toFloat();
        c.account_amount = message_float;
    }

    if(String(topic) == "/charger/order/b_level"){
        Serial.print("Battery Level: "); Serial.println(messageTemp);
        int message_int = messageTemp.toInt();
        c.battery_level = message_int;  
    }

    if(String(topic) == "/charger/order/health"){
        Serial.print("Battery Health: "); Serial.println(messageTemp);
        int message_int = messageTemp.toInt();
        c.account_amount = message_int;
    }

    if(String(topic) == "/charger/order/begin"){
        Serial.print("Order from node-red: "); Serial.println(messageTemp);
        int message_int = messageTemp.toInt();
        if(customer_waiting){
            Serial.println("Initiating maintinence on call from Node-red.");
            begin_maintenance = true;
        } else {
            Serial.println("Error ZUMO not send arrival flag.");
        }
    }

    if(String(topic).startsWith("/red/charge/")){
        Serial.println("Customer waiting");
        customer_waiting = true;
    }
}

void mqtt_reconnect() { // Initiates MQTT when connected and subscribes to relevant topics
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
        Serial.println("connected");

        client.subscribe("/maintenance/#");
        client.subscribe("/breaker/out");
        client.subscribe("/bank/charger/#");
        client.subscribe("/charger/order/#");
        client.subscribe("/red/charge/#");

        get_power_price(); // payment.cc
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
    }
  }
}