
#ifndef CHARGER
#define CHARGER

#include <common.hh>

// Variables:
inline bool charged = true;
inline int desired_charge = 100;

inline int select_button_value = 1;
inline int confirm_order_button_pressed = false;

inline bool confirm_button_state = false;
inline bool previous_confirm_button_state = false;

inline bool select_button_state = false;
inline bool previous_select_button_state = false;   


// Functions:
    // PRE-SERVICE:
void loopStationCode();
void begin_service(); 
    // BUTTONS:
void updateOrderButtonsState();
void updateOrderButtonsPressed();

void updateSelectButtonState();
void updateConfirmButtonState();

void updateSelectButtonPressed();
void updateConfirmButtonPressed();
    // SERVICE
void charge_battery();
void change_battery();
void print_to_display(String battery_level);
    // POST-SERVICE
void send_zumo();
void sendData(Customer s_c, esp_now_peer_info_t &peerInfo);
void resetVariables();

#endif