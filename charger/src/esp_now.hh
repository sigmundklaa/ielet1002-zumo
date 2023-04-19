
#ifndef ESP_NOW
#define ESP_NOW

#include <common.hh>

// Variables:


// Functions:
void setupEspNow();
void connectToEspNowPeer(esp_now_peer_info_t &peerInfo);
void onDataTransmitted(const uint8_t *receiverMacAddress, esp_now_send_status_t transmissionStatus);
void printEspErrorCode(String message, esp_err_t errorCode);

#endif