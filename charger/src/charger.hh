
#ifndef CHARGER
#define CHARGER

#include <common.hh>

// Variables:
inline bool charged = true;
inline int desired_charge = 100;


// Functions:
    // PRE-SERVICE:
void loopStationCode();
void begin_service(); 
    // SERVICE
void charge_battery();
void change_battery();
    // POST-SERVICE
void send_zumo();
void sendData(Customer s_c, esp_now_peer_info_t &peerInfo);
#endif