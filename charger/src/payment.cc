
#include <payment.hh>

/*
    payment.cc: File containing code relevant to handeling of payment and processing.
*/

void get_power_price(){ // Get prices from node red on startup, as well as desired charge.
    client.publish("/maintenance/price/in", "Get");
}

void send_order_details(float order_cost, float credit, int charge, int order_type){ // Converts variables to sendable variables and sends them to server
    char costString[8];
    dtostrf(order_cost, 1, 2, costString);
    Serial.print("Sending cost: "); Serial.println(costString);
    client.publish("/bank/payment/cost", costString);

    char creditString[8];
    dtostrf(credit, 1, 2, creditString);
    Serial.print("Sending credit: "); Serial.println(creditString);
    client.publish("/bank/payment/credit", creditString);

    char chargeString[8];
    dtostrf(charge, 1, 2, chargeString);
    Serial.print("Sending charge: "); Serial.println(chargeString);
    client.publish("/charger/charge/finish", chargeString);

    char orderString[8];
    dtostrf(order_type, 1, 2, orderString);
    Serial.print("Sending order type: "); Serial.println(orderString);
    client.publish("/bank/payment/order", orderString);
}

