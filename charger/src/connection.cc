
#include <common.hh>
#include <connection.hh>
#include <charger.hh>
#include <payment.hh>

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

void loopConnectionCode(){ // Core connection code, makes sure charge station is connected to MQTT
    if (!client.connected()) {
    mqtt_reconnect();
    }
    client.loop();
}

// MQTT
void mqtt_callback(char* topic, u_int8_t* message, unsigned int length){ // Handles incomming messages from MQTT
    Serial.print("Topic: "); Serial.println(topic); 
    String messageTemp; // Saves message to string
    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }

    // Function is the same for every if-segment, only values changes
    if(String(topic) ==  "/maintenance/price/power"){ // Choses what the message is based on topic
        Serial.print("New Power Price: "); Serial.println(messageTemp); // Prints to serial for feedback
        float message_float = messageTemp.toFloat(); // Converts string to actual value (int, float or bool)
        power_price = message_float; // Changes variable
    }
    if(String(topic) == "/maintenance/price/batt"){
        Serial.print("New Battery Price: "); Serial.println(messageTemp);
        float message_float = messageTemp.toFloat();
        battery_price = message_float;
    }

    if(String(topic) == "/maintenance/order/desired/out"){
        Serial.print("New desired charge: "); Serial.println(messageTemp);
        int message_int = messageTemp.toInt();
        desired_charge = message_int;
    }  

    if(String(topic) == "/bank/details/out"){
        Serial.print("Account amount: "); Serial.println(messageTemp);
        float message_float = messageTemp.toFloat();
        account_amount = message_float;
    }
    
    if(String(topic) == "/charger/order/mode"){
        Serial.print("Charger station mode: "); Serial.println(messageTemp);
        if(messageTemp == "true"){
            auto_mode = true;
            Serial.println(auto_mode);
        } else {
            auto_mode = false;
            Serial.println(auto_mode);
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
        c.batt_status = message_int;  
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
            node_red_call = true;
        } else { // Is here to makes sure there is actually a customer, was more relevant when dashboard had button, but still useful.
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

        // Subscribes to relevant topics "/#" means it subscribes to any call starts the identically to what is before the "/#"
        client.subscribe("/maintenance/#");
        client.subscribe("/breaker/out");
        client.subscribe("/bank/charger/#");
        client.subscribe("/charger/order/#");
        client.subscribe("/red/charge/#");

        get_power_price(); // payment.cc
    } else { // If client does not connect, retries
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
    }
  }
}