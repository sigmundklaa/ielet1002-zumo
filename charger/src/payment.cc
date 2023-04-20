
#include <payment.hh>

/*
PAYMENT: All things related to payment
*/

void check_price()
{
    power_price = 0.80; //TODO make it get price from server. 
    battery_price = 120.60; //TODO ^^
    
    Serial.print("Power price: "); Serial.print(power_price);
    Serial.print(". Battery price: "); Serial.println(battery_price);
};

bool check_credit()
{
    Serial.println("Checking for customer credit");

    bool has_credit = false;
    if(c.credit > 0){
        has_credit = true;
    } else{
        has_credit = false;
    }
    return has_credit;
};

void pay_credit()
{
    for(int i = c.credit; i > 0; i--){
        if(c.account_balance == 0){
            Serial.println("Customer ran out of money while paying off credit.");
            ran_out = true;
            break;
        }
        c.credit--;
        c.account_balance--;

        Serial.print("Customer credit: ");
        Serial.println(c.credit);
    }
};