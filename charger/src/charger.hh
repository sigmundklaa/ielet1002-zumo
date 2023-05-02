
#ifndef CHARGER
#define CHARGER

/*
    charger.hh: Header file that declares variables and functions used mostly in charger.cc
*/

#include <common.hh>

// Variables:
    // BASICS
inline int desired_charge;
inline float order_cost;
inline int set_orders = 2;

    // FLAGS
inline bool auto_mode;
inline bool begin_maintenance;
inline bool allow_credit_message;

inline bool customer_waiting;
inline int customer_order;

inline int allow_credit = -1;
inline bool charged = true;
inline bool changed = true;
    

    // TIMERS
inline int debounce_delay = 50;
inline int change_delay = 10000;
inline unsigned long wait_millis = 0;

inline unsigned long select_button_timer = 0;
inline unsigned long confirm_button_timer = 0;
inline unsigned long battery_change_timer = 0;

    // BUTTON STATES
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

    // POST-SERVICE
void send_zumo(float order_cost, float credit, int order_type);
void resetVariables();

#endif // CHARGER