
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

void updateButtonsStates(){
    updateSelectButtonState();
    updateConfirmButtonState();
}

void updateButtonsPressed(){
    updateSelectButtonPressed();
    updateConfirmButtonPressed();
}

// Checks if select button has been pressed.
void updateSelectButtonState(){
    // Makes sure the button does not update several times in one press.
    if((millis() - select_button_timer) < debounce_delay){
        return;
    }

    bool new_state = digitalRead(select_button_pin);

    // Resets delay timer if button is pressed.
    if (new_state != select_button_state){
        select_button_timer = millis();
    }   

    select_button_state = new_state;
}

// Updates select button value if select button is pressed.
void updateSelectButtonPressed(){
    // Value from 1 -> x orders possible   
    if((select_button_state == true) && (previous_select_button_state == false)){
        if(select_button_value == 3){
            select_button_value = 1;
        } else {
            select_button_value++;
        }
        Serial.print("Current selected order: "); Serial.println(select_button_value);
    }
    previous_select_button_state = select_button_state;
}

// Same as updateSelectButtonState, only for confirm button
void updateConfirmButtonState(){
    if((millis() - confirm_button_timer) < debounce_delay){
        return;
    }

    bool new_state = digitalRead(confirm_button_pin);

    if (new_state != confirm_button_state){
        confirm_button_timer = millis();
    }   

    confirm_button_state = new_state;
}

// Sets customer order of confirm button is pressed.
void updateConfirmButtonPressed(){
    if((confirm_button_state == true) && (previous_confirm_button_state == false)){
        if((select_button_value > 0) && (select_button_value < 4)){
            customer_order == select_button_value;
            confirm_order_button_pressed = true;
            Serial.println("Confirm button pressed.");
        }       
    }
    previous_confirm_button_state = confirm_button_state;
}

void begin_service(){
    unsigned long order_wait_millis = 0;

    switch(customer_order){
        case 0: // Wait for order
            switch(confirm_order_button_pressed){
                case false:
                    updateButtonsStates();
                    updateButtonsPressed();
                    //print_to_display(String(customer_order));
                    break;
                case true:
                    customer_order = select_button_value;
            }
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
    Serial.println(ran_out);
    
    switch(ran_out){
        case false: // If enough cash
            Serial.println(desired_charge);
            while(c.battery_level < desired_charge){
                if(c.account_balance == 0){
                    Serial.println("Customer ran out of money.");
                    ran_out = true;
                    break;
                }
                if((millis() - charge_millis) > 100){
                    charge_millis = millis();
                    c.battery_level++;
                    c.account_balance = c.account_balance - power_price;
                    
                    //print_to_display(String(c.battery_level));
                    Serial.print("Battery charge: ");
                    Serial.println(c.battery_level);
                }
            }
            break;
        case true: // If not enough cash #TODO: Ask for confirmation.
            Serial.println(ran_out);
            while(c.battery_level < desired_charge){
                if((millis() - charge_millis) > 100){
                    charge_millis = millis();
                    c.battery_level++;
                    c.credit = c.credit + power_price;

                    //print_to_display(String(c.battery_level));
                    Serial.print("Battery charge: ");
                    Serial.println(c.battery_level);
                }
            }
            break;
    }

    if((c.battery_level == desired_charge) || (c.battery_level >= 100)){
        charged = true;
        c.charging_cycles++;

        Serial.println("Battery charged.");
    }


    if(charged = true){
        // Checks for credit then asks for input YES or NO #TODO: Ask for confirmation
        if(check_credit() == true){
            Serial.println("Customer has credit.");
            pay_credit();
        }

        send_zumo();
    }
}; 

    // CHANGE BATTERY
void change_battery(){
    // TODO delay?
    Serial.println("Changing customer battery.");
    c.charging_cycles = 0;
    c.battery_health = 100;



    send_zumo();
}

// Print a string to OLED NB!! DOES NOT WORK
void print_to_display(String str){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    display.println(str);
    display.display();
};


// POST-SERVICE
void send_zumo(){
    Serial.println("Sending customer away.");
    sendData(c, routerDeviceInfo);
    resetVariables();
};

// Resets all temporary variables
void resetVariables(){
    Serial.println("Resetting variables");
    confirm_button_state = false;
    select_button_state = false;
    customer_order = 0;
    select_button_value = 0;
    ran_out = false;
}

// Sends customer data over ESP-NOW to zumo.
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