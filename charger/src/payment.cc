
#include <payment.hh>

/*
    payment.cc: File containing code relevant to handeling of payment and processing.
*/

void get_power_price(){ // Initiates a request call to Node-Red for updateing relevant values, (prices and desired charge)
    client.publish("/maintenance/price/in", "Get");
}

void send_order_details(float order_cost, float credit, int charge, int order_type){ // Converts variables to sendable variables and sends them to Node-Red
    // Function is the same for every segment, only variables changes
    char costString[8]; // Declares a variable of type char, as MQTT requires this
    dtostrf(order_cost, 1, 2, costString); // Converts variable and inserts it to the char variable
    Serial.print("Sending cost: "); Serial.println(costString); // Send feedback though Serial
    client.publish("/bank/payment/cost", costString); // Sends char variable over topic 

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

