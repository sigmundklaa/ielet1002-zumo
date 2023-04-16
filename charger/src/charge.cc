
#include <common.hh>
#include <charge.hh>

/*
CHARGE: All things related to charging-ish
*/

// PRE-CHARGE: Defines struct with data recieved for ESP-NOW
void process_customer(const uint8_t * mac, const uint8_t *data, int len){
    Customer* c = (Customer*) data;
    charged = false;
    ran_out = false;
    // #TODO Ask for desired charge.
    desired_charge = 100;
}

// CHARGING
void charge_battery()
{
    unsigned long charge_millis = 0;
    
    switch(ran_out){
        case false: // If enough cash
            for(int i = c.battery_level; i < desired_charge; i++){
                if(c.account_balance == 0){
                    ran_out = true;
                    break;
                }
                if(previousMillis - charge_millis == 1000){
                    charge_millis = millis();
                    i++;
                    c.account_balance--;
                }
            }
            break;
        case true: // If not enough cash #TODO: Ask for confirmation.
            for(int i = c.battery_level; i < desired_charge; i++){
                if(previousMillis - charge_millis == 1000){
                    charge_millis = millis();
                    i++;
                    c.credit++;
                }
            }
            break;
    }

    // Safeguard for error in case charging went wrong
    if(c.battery_level == desired_charge){
        charged = true;
    } else {
        Serial.println("Error: battery_level not equal to desired_charge after charge");
    }

    // Checks for credit then asks for input YES or NO #TODO: Ask for confirmation
    if(check_credit() == true){
        pay_credit();
    }
    send_zumo();
}; 

// Fuction for sending Zumo away after charge, seperate for sendData bc it might need to do more than just send data
// #TODO Something more than sendData
void send_zumo(){
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