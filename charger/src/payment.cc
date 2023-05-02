
#include <payment.hh>

/*
    payment.cc: File containing code relevant to handeling of payment and processing.
*/

void get_power_price(){ // Get prices from node red on startup, as well as desired charge.
    client.publish("/maintenance/price/in", "Get");
}

void send_order_details(float order_cost, float credit, int order_type){ // Converts variables to sendable variables and sends them to server
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

void send_charge_update(float battery_level){ // Sends an update to server of battery status
    char levelString[8];
    dtostrf(battery_level, 1, 2, levelString);
    Serial.println("Sending charge update");
    client.publish("/maintenance/charge/update", levelString);
}