
#include <WiFi.h>
#include <esp_now.h>

#include <common.hh>

/*
ESP-NOW: Everything related to the ESP-NOW connection management
*/

esp_now_peer_info_t routerDeviceInfo = {
    // Change this address to the address of the router ESP32
    .peer_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .channel = 0,
    .encrypt = false,
};

void setupEspNow()
{
    WiFi.mode(WIFI_STA);

    esp_err_t espNowInitStatus = esp_now_init();
    if(espNowInitStatus != ESP_OK){
        printEspErrorCode("Error initializing ESP-NOW", espNowInitStatus);
        ESP.restart();
    }

    connectToEspNowPeer(routerDeviceInfo);

    // Give feedback if data was successfully transmitted or not.
    esp_now_register_send_cb(onDataTransmitted);
}

void connectToEspNowPeer(esp_now_peer_info_t &peerInfo)
{
    // Safeguard for setting correct MAC Address
    if (peerInfo.peer_addr[0] == 0xFF)
    {
        Serial.println("Please change the peer mac address to the address of the other ESP32 at the top of the code");
        Serial.println("Rebooting");
        delay(5000);
        ESP.restart();
    }

    // Add peer (other ESP32)
    esp_err_t peerConnectionStatus = esp_now_add_peer(&peerInfo);
    if (peerConnectionStatus != ESP_OK)
    {
        printEspErrorCode("Failed to add peer", peerConnectionStatus);
        ESP.restart();
    }
}

void onDataTransmitted(const uint8_t *receiverMacAddress, esp_now_send_status_t transmissionStatus)
{
    if (transmissionStatus != ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("Data transmission failed!");
        return;
    }

    Serial.println("Data transmission was successful!");
} 

void printEspErrorCode(String message, esp_err_t errorCode)
{
    Serial.println(message);
    Serial.print("Error Code:");
    Serial.println(errorCode, HEX);
    Serial.println("The meaning of this error code can be found at:");
    Serial.println("https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-codes.html");
}