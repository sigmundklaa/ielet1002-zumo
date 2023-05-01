
#include <payment.hh>

/*
    payment.cc: File containing code relevant to handeling of payment and processing.
*/

void get_power_price(){ // Get power price from node red on startup
    client.publish("/maintenance/price/in", "Get");
}

void send_order_details(float order_cost, float credit){
    char costString[8];
    dtostrf(order_cost, 1, 2, costString);

    client.publish("/bank/cost/in", costString);

    char creditString[8];
    dtostrf(credit, 1, 2, creditString);

    client.publish("/bank/credit/in", creditString);
}