
/*
    charger.cc: File containing code related to the charging stations functions. 
*/

#include <charger.hh>
#include <payment.hh>

// PRE-SERVICE:
void loopStationCode()
{
    switch(customer_waiting){
        case true:
            begin_service();
            break;
        case false:
            break;
    }
}

void updateButtonsStates()
{
    updateSelectButtonState();
    updateConfirmButtonState();
}

void updateButtonsPressed()
{
    updateSelectButtonPressed();
    updateConfirmButtonPressed();
}

void updateSelectButtonState() // Used to check if the button change state (meaning it was pressed)
{
    if((millis() - select_button_timer) < debounce_delay){ // Prevents double presses
        return;
    }

    bool new_state = !digitalRead(select_button_pin);

    if (new_state != select_button_state){
        select_button_timer = millis();
    }   

    select_button_state = new_state;
}

void updateSelectButtonPressed() // Checks if button has been pressed and changes relevant value
{
    if((select_button_state == true) && (previous_select_button_state == false)){ 
        if(select_button_value == 3){ // Edit "3" if more than 3 order types (currently 2);
            select_button_value = 1;
        } else {
            select_button_value++;
        }
        Serial.print("Current selected order: "); Serial.println(select_button_value);
    }
    previous_select_button_state = select_button_state; 
}

void updateConfirmButtonState(){ // Same as select
    if((millis() - confirm_button_timer) < debounce_delay){
        return;
    }

    bool new_state = !digitalRead(confirm_button_pin);

    if (new_state != confirm_button_state){
        confirm_button_timer = millis();
    }   

    confirm_button_state = new_state;
}

void updateConfirmButtonPressed() // Checks if button has been pressed and proceedes.
{
    if((confirm_button_state == true) && (previous_confirm_button_state == false)){
        if((select_button_value > 0) && (select_button_value < 4)){ // Makes sure order is withing order types. 
            customer_order == select_button_value;
            confirm_order_button_pressed = true;
            Serial.println("Confirm button pressed.");

            //get_account_details(); // payment.cc
        }       
    }
    previous_confirm_button_state = confirm_button_state;
}

void begin_service()
{
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
        case 1: // Charge battery to desired level, default 100 (percent)
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
    Serial.print("Desired Charge: "); Serial.println(desired_charge);
    while(c.battery_level < desired_charge){ // Charges battery to level from node-red (default: 100);
        switch(ran_out){ // ran_out == ran out of money in the bank account
            case false:
                if((c.account_amount - order_cost) < 0){ // Checks if customer can afford it.
                    Serial.println("Customer ran out of money.");
                    ran_out = true;
                    break;
                }
                if((millis() - wait_millis) > 100){ // Delay to simulate charging time
                    charged = false;
                    wait_millis = millis();
                    c.battery_level++;
                    order_cost += power_price;
                            
                    //print_to_display(String(c.battery_level));
                    Serial.print("Battery charge: ");
                    Serial.println(c.battery_level);
                }
                break;
            case true:
                if((millis() - wait_millis) > 100){    
                    charged = false;
                    wait_millis = millis();
                    c.battery_level++;
                    credit += power_price;

                    //print_to_display(String(c.battery_level));
                    Serial.print("Battery charge: ");
                    Serial.println(c.battery_level);
                }
                break;
            }
    }

    if((c.battery_level == desired_charge) || (c.battery_level >= 100)){ // Confirms order is completed.
        charged = true;
        c.charging_cycles++;

        Serial.println("Battery charged.");
    }


    if(charged = true){
        send_zumo(order_cost, credit);
    } else {
        Serial.print("Error, battery was not charged.");
        send_zumo(order_cost, credit);
    }
}; 

    // CHANGE BATTERY
void change_battery()
{
    if(changed && ((millis()-battery_change_timer) > change_delay)){ // Initiates battery change.
        changed = false;
        Serial.println("Changing customer battery.");
        battery_change_timer = millis();

    } else if (!changed && ((millis()-battery_change_timer) > change_delay)){ // Completes change order. 
        Serial.println(" Completed!");
        c.charging_cycles = 0;
        c.battery_health = 100;

        order_cost = battery_price;

        if((c.account_amount - order_cost) < 0){ // Checks if customer can afford, adds unpaid amount to account credit. 
            Serial.println("Not enough money, adding rest to credit.");
            float leftover = order_cost - c.account_amount;
            order_cost = order_cost - leftover;
            credit = leftover;
        }

        send_zumo(order_cost, credit);
    } else if (!changed && ((millis()-battery_change_timer) < change_delay)){ // Delay to simulate time to change
        if((millis()-wait_millis)>500){
            wait_millis = millis();
            Serial.print(".");
        }       
    }
}

// POST-SERVICE
void send_zumo(float order_cost, float credit)
{
    Serial.println("Sending customer away.");
    send_order_details(order_cost, credit); //payment.cc
    resetVariables();
};

void resetVariables() // Resets all temporary variables to prepare for next customer
{
    Serial.println("Resetting variables.");

    confirm_button_state = false;
    select_button_state = false;

    customer_order = 0;
    select_button_value = 0;

    order_cost = 0;
    credit = 0;
    ran_out = false;

    customer_waiting = false;
    begin_maintenance = false;
}

