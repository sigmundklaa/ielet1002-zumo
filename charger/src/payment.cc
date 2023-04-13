
#include <payment.hh>

/*
PAYMENT: All things related to payment
*/

void check_price()
{
    // #TODO: Get price from cloud
};

bool check_credit()
{
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
            ran_out = true;
            break;
        }
        c.credit--;
        c.account_balance--;
    }
};