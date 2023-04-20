
#ifndef PAYMENT
#define PAYMENT

#include <common.hh>

// Variables:
inline bool ran_out = false;
inline float power_price = 0.00;
inline float battery_price = 000.00;

// Functions:
void check_price();
bool check_credit();
void pay_credit();


#endif