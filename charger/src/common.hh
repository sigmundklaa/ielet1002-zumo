
/*
GLOBAL: Variables used by several files
*/

unsigned long previousMillis = 0;

struct __attribute__((packed)) Customer {
    int customer_id;
    int battery_level;
    int battery_health;
    int charging_cycles;
    float account_balance;
    float credit;
} c;

// PINS
const int button_pin_1;