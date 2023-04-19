
#include <charger.hh>
#include <payment.hh>

// PRE-CHARGE: Defines struct with data recieved for ESP-NOW
void process_customer(const uint8_t * mac, const uint8_t *data, int len){
    Customer* c = (Customer*) data;
    Serial.println("Customer data reviced");

    charged = false;
    ran_out = false;
    
    // #TODO Ask for desired charge.
    desired_charge = 100;
}

// CHARGING
void charge_battery()
{
    unsigned long charge_millis = 0;

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
                if(previousMillis - charge_millis == 1000){
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
                if(previousMillis - charge_millis == 1000){
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

// Fuction for sending Zumo away after charge, seperate for sendData bc it might need to do more than just send data
// #TODO Something more than sendData
void send_zumo(){
    Serial.println("Sending customer away.");
    sendData(c, routerDeviceInfo);
};

void sendData(Customer s_c, esp_now_peer_info_t &peerInfo)
{
    esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t *) &s_c,sizeof(s_c));
   
    if (result == ESP_OK) {
        Serial.println("Data successfully sent.");
        //Clear customer_id so station goes back to waiting. 
        c.customer_id = 0;
    } else {
        Serial.println("Error sending data.");
    }
} 