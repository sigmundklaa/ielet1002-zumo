
#ifndef CHARGER
#define CHARGER

#include <common.hh>

// Variables:
inline bool charged = true;
inline int desired_charge = 100;


// Functions:
void send_zumo();
void sendData(Customer s_c, esp_now_peer_info_t &peerInfo);
void process_customer(const uint8_t * mac, const uint8_t *data, int len);
void charge_battery();

#endif