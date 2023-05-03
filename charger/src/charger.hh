
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
        // MODES
inline bool auto_mode;
inline bool begin_maintenance;
        // ORDER DETAILS
inline bool allow_credit_message;
inline bool cancel_flag;
inline bool order_completed;
        // PRE SERVICE
inline bool node_red_call;
inline bool customer_waiting;
inline int customer_order;
        // POST SERVICE
inline int allow_credit;
inline bool charged = true;
inline bool changed = true;
    

    // TIMERS
inline int debounce_delay = 50;
inline int change_delay = 10000;
inline unsigned long wait_millis = 0;
inline unsigned long update_millis = 0;
inline unsigned long oled_delay = 0;

inline unsigned long select_button_timer = 0;
inline unsigned long confirm_button_timer = 0;
inline unsigned long battery_change_timer = 0;

    // BUTTON STATES
inline int select_button_value;
inline int confirm_order_button_pressed = false;

inline bool confirm_button_state = false;
inline bool previous_confirm_button_state = false;

inline bool select_button_state = false;
inline bool previous_select_button_state = false;   

    // OLED
#define SCREEN_WIDTH_ 128
#define SCREEN_HEIGHT_ 64
#define OLED_RESET_ -1
#define SCREEN_ADDRESS_ 0x3C
static Adafruit_SSD1306 display(SCREEN_WIDTH_, SCREEN_HEIGHT_, &Wire, OLED_RESET_);

// Functions:
    // PRE-SERVICE:
void loopStationCode();
void begin_service(); 
    // BUTTONS:
void updateSelectButtonState();
void updateConfirmButtonState();

void updateSelectButtonPressed();
void updateConfirmButtonPressed();
    // SERVICE
void charge_battery();
void change_battery();

    // POST-SERVICE
void send_zumo(float order_cost, float credit, int batt_status, int order_type);
void resetVariables();

    // Oled
void loopOledCode();
void print_to_display(String text);

#endif // CHARGER