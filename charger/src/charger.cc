
#include <charger.hh>
#include <payment.hh>

// PRE-SERVICE:
void loopStationCode(){
    switch(customer_waiting){
        case true:
            begin_service();
            break;
        case false:
            break;
    }
}

void begin_service(){
    unsigned long order_wait_millis = 0;

    //INPUT
    customer_order = 1; // TEMP UNTILL INPUT
    switch(customer_order){
        case 0: // Wait for order
            break;
        case 1: // Charge battery to desired level, default 100%
            charge_battery();
            break;
        case 2:
            change_battery(); // Change battery
            break;
        case 3:
            // ???
            break;
    }
}

// SERVICE
    // CHARGING
void charge_battery()
{
    unsigned long charge_millis = 0;

    //DESIRED CHARGE INPUT
    desired_charge = 100; // TEMP UNTILL DESIRED CHARGE INPUT

    // Checks for current power price
    check_price();
    
    switch(ran_out){
        case false: // If enough cash
            for(int i = c.battery_level; i < desired_charge; i++){
                if(c.account_balance == 0){
                    Serial.println("Customer ran out of money.");
                    ran_out = true;
                    break;
                }
                if(previousMillis - charge_millis > 1000){
                    charge_millis = millis();
                    i++;
                    c.account_balance = c.account_balance - power_price;
                    
                    Serial.print("Battery charge: ");
                    Serial.println(i);
                }
            }
            break;
        case true: // If not enough cash #TODO: Ask for confirmation.
            for(int i = c.battery_level; i < desired_charge; i++){
                if(previousMillis - charge_millis > 1000){
                    charge_millis = millis();
                    i++;
                    c.credit = c.credit + power_price;

                    Serial.print("Battery charge: ");
                    Serial.println(i);
                }
            }
            break;
    }

    // Safeguard for error in case charging went wrong
    if(c.battery_level == desired_charge){
        charged = true;
        c.charging_cycles++;

        Serial.println("Battery charged.");
    } else {
        Serial.println("Error: battery_level not equal to desired_charge after charge.");
    }

    // Checks for credit then asks for input YES or NO #TODO: Ask for confirmation
    if(check_credit() == true){
        Serial.println("Customer has credit.");
        pay_credit();
    }

    send_zumo();
}; 

    // CHANGE BATTERY
void change_battery(){
    // TODO delay?
    Serial.println("Changing customer battery.");
    c.charging_cycles = 0;
    c.battery_health = 100;

    

    send_zumo();
}


// POST-SERVICE
void send_zumo(){
    Serial.println("Sending customer away.");
    sendData(c, routerDeviceInfo);
};

void sendData(Customer s_c, esp_now_peer_info_t &peerInfo)
{
    esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t *) &s_c,sizeof(s_c));
   
    if (result == ESP_OK) {
        Serial.println("Data successfully sent.");
        customer_waiting = false; // Inform station that customer successfully left.
    } else {
        Serial.println("Error sending data.");
    }
} 