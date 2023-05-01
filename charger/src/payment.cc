
#include <payment.hh>

/*
    payment.cc: File containing code relevant to handeling of payment and processing.
*/

void get_power_price(){ // Get power price from node red on startup
    client.publish("/maintenance/price/in", "Get");
}

void send_order_details(float order_cost, float credit, int order_type){
    char costString[8];
    dtostrf(order_cost, 1, 2, costString);
    Serial.println("Sending cost.");
    client.publish("/bank/payment/cost", costString);

    char creditString[8];
    dtostrf(credit, 1, 2, creditString);
    Serial.println("Sending credit.");
    client.publish("/bank/payment/credit", creditString);

    char orderString[8];
    dtostrf(order_type, 1, 2, orderString);
    Serial.println("Sending order type.");
    client.publish("/bank/payment/order", orderString);


}