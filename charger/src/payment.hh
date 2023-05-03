
#ifndef PAYMENT
#define PAYMENT

/*
    payment.hh: Header file that declares variables and functions used mostly in payment.cc
*/

#include <common.hh>
#include <connection.hh>

// Variables:
    // FLAGS
inline bool ran_out;

    // PRICES
inline float power_price;
inline float battery_price;

    // DETAILS
inline float account_amount;
inline float credit;
inline char cost_string;

// Functions:
void get_power_price();

void get_account_details();
void send_order_details(float order_cost, float credit, int batt_status, int order_type);




#endif // PAYMENT