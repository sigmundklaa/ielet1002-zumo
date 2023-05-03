
/*
    charger.cc: File containing code related to the charging stations functions. 
*/

#include <charger.hh>
#include <payment.hh>

// PRE-SERVICE:
void loopStationCode() // Core switch that chooses if service runs or not
{
    switch(begin_maintenance){
        case true:
            begin_service();
            break;
        case false:
            if(node_red_call && customer_waiting){ // Initiates service if Node-Red and Customer calls for it. 
                begin_maintenance = true;
            }
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

void updateSelectButtonPressed() // If button state changes, relevant value changes, as described:
{
    if((select_button_state == true) && (previous_select_button_state == false)){ 
        if(!confirm_order_button_pressed){ // 1) Runs before order is set, used to set order. 
            if(select_button_value == set_orders){ // Edit "set_orders" if more order choices (currently 2);
                select_button_value = 1;
            } else {
                select_button_value++;
            }
            Serial.print("Current selected order: "); Serial.println(select_button_value);

        } else if(ran_out){ // 2) Runs after order is set and customer runs out of money, cancels service. 
            Serial.println("Ending charge order");
            cancel_flag = true;
            allow_credit = false;
        }
    }
    previous_select_button_state = select_button_state; 
}

void updateConfirmButtonState(){ // Same as updateSelectButtonState, instead for select button
    if((millis() - confirm_button_timer) < debounce_delay){
        return;
    }

    bool new_state = !digitalRead(confirm_button_pin);

    if (new_state != confirm_button_state){
        confirm_button_timer = millis();
    }   

    confirm_button_state = new_state;
}

void updateConfirmButtonPressed() // If button state changes, relevant value changes, as described:
{
    if((confirm_button_state == true) && (previous_confirm_button_state == false)){
        if(!confirm_order_button_pressed){ // 1) Runs before order is set, confirms order choice
            if((select_button_value > 0) && (select_button_value < 4)){ // Makes sure order is within acceptable values
                customer_order == select_button_value;
                confirm_order_button_pressed = true;
                Serial.println("Confirm button pressed.");
            }
             
        } else if(ran_out){ // 2) Runs after order is set and customer runs out of money, allows paying with credit and completes order
            allow_credit = true;
            Serial.println("Allowing to pay with credit");
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
                    
                    if(!auto_mode){
                        updateButtonsStates();
                        updateButtonsPressed();
                    } else if (auto_mode) {
                        select_button_value = 1;
                        confirm_order_button_pressed = true;
                    }
                    
                    break;
                case true:
                    customer_order = select_button_value;
            }
            break;
        case 1: // Charge battery to desired level, default 255, changed in node-red
            charge_battery();
            break;
        case 2:
            change_battery(); // Change battery
            break;
        case 3:
            // ??? Add new orders, edit set_orders to allow selection of these.
            // This is left to demonstrate it can be added more modes if needed
            break;
    }
}

// SERVICE
    // CHARGING
void charge_battery()
{   
    Serial.print("Desired Charge: "); Serial.println(desired_charge);
    while((c.batt_status < desired_charge) && !cancel_flag){ // Charges battery to level from node-red (default: 255);

        loopOledCode(); // Run loop functions otherwise they get blocked by the while-loop();
        loopConnectionCode();

        switch(ran_out){ // ran_out == ran out of money in the bank account
            case false: // Charges battery until account runs out of money or battery is charged
                if((c.account_amount - order_cost) < 0){ // Checks if customer can afford a new charge.
                    Serial.println("Customer ran out of money.");
                    ran_out = true;
                    break;
                }
                if((millis() - wait_millis) > 100){ // Delay to simulate charging time
                    charged = false;
                    wait_millis = millis();
                    c.batt_status++;
                    order_cost += power_price;
                            
                    Serial.print("Battery charge: ");
                    Serial.println(c.batt_status);
                }
                break;
            case true: // Customer runs out of money, waits for input
                switch(allow_credit){ // If customer orders to allow credit, it charges battery to desired level
                    case true:
                        if((millis() - wait_millis) > 100){ // Delay as above
                            charged = false;
                            wait_millis = millis();
                            c.batt_status++;
                            credit += power_price;

                            Serial.print("Battery charge: ");
                            Serial.println(c.batt_status);
                        }
                        break;
                    case false: // Waits for input on buttons
                        if(allow_credit_message){ // Sends message in serial to instruct
                            Serial.println("Press 'Confirm' to pay with credit, 'select' to end charge");
                            allow_credit_message = false;
                        }
                        updateButtonsStates();
                        updateButtonsPressed();
                        break;
                }
                break;
        }
    }

    if((c.batt_status == desired_charge) || (c.batt_status >= 255)){ // Confirms order is completed.
        charged = true;
        order_completed = true;

        Serial.println("Battery charged.");
    } else if ((c.batt_status != desired_charge) && (allow_credit = 0)){ // Confirms order if cancelled. 
        charged = true;
        order_completed = true;

        Serial.println("Battery charged as much as client could afford.");
    }


    if(charged = true){
        send_zumo(order_cost, credit, c.batt_status, customer_order);
    } else { // Only occures if unforseen errors makes charge not work
        Serial.print("Error, battery was not charged correctly.");
        send_zumo(order_cost, credit, c.batt_status, customer_order); // Still sends zumo to not interrupt other modules.
    }
}; 


    // CHANGE BATTERY
void change_battery()
{
    if(changed && ((millis()-battery_change_timer) > change_delay)){ // Initiates battery change.
        changed = false;
        Serial.println("Changing customer battery.");
        battery_change_timer = millis();
        order_cost = battery_price;

    } else if (!changed && ((millis()-battery_change_timer) > change_delay)){ // Completes change order. 
        Serial.println(" Completed!");  
        order_completed = true;
        changed = true;
        c.batt_status = 255;
        c.batt_health = 255;

        order_cost = battery_price;

        if((c.account_amount - order_cost) < 0){ // Checks if customer can afford, adds unpaid amount to account credit. 
            Serial.println("Not enough money, adding rest to credit.");
            float leftover = order_cost - c.account_amount + 1; //+1 to prevent rounding errors causing overflow
            order_cost = order_cost - leftover; 
            credit = leftover; //the +1 above is also part of credit
        }

        send_zumo(order_cost, credit, c.batt_status, customer_order);

    } else if (!changed && ((millis()-battery_change_timer) < change_delay)){ // Delay to simulate time to change
        if((millis()-wait_millis)>500){
            wait_millis = millis();
            Serial.print(".");
            //print_to_display("Changing battery...")
        }       
    }
}

// POST-SERVICE
void send_zumo(float order_cost, float credit, int batt_status, int order_type)
{
    loopOledCode(); // Run in order to update display one last time (otherwise it won't show complete message)
    Serial.println("Sending customer away.");
    send_order_details(order_cost, credit, batt_status, order_type); //payment.cc
    resetVariables();
};

void resetVariables() // Resets all temporary variables to prepare for next customer
{
    Serial.println("Resetting variables.");

    confirm_button_state = false;
    select_button_state = false;
    confirm_order_button_pressed = false;

    customer_order = 0;
    select_button_value = 0;

    order_cost = 0;
    credit = 0;
    
    ran_out = false;
    allow_credit = false;
    allow_credit_message = true;

    node_red_call = false;
    customer_waiting = false;
    begin_maintenance = false;
    cancel_flag = false;
    order_completed = false;
}

// oled
void setupOled()
{
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_);
    display.display();
    delay(2000);
}

void print_to_display(String str) // Core display code
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    display.println(str);
    display.display();
};

void loopOledCode(){ // Code that displays messages on Oled based on flags from other functions
    if((millis() - oled_millis) > 500) { // Only updates oled every half second to avoid potential errors or damage
        if(customer_waiting){ // Runs if a customer is waiting
            if(customer_order == 0){ // Runs if order is being selected to show what order is selected
                print_to_display(String(select_button_value));
            } else if(customer_order == 1){ // Runs to show charge percent and price. 
                if(!charged){
                    int batt_percent = map(c.batt_status, 0, 255, 0, 100);
                    String text = String(batt_percent) + "%,   " + String(order_cost) + " Kr";
                    print_to_display(text);
                }
            } else if(customer_order == 2){ // Runs to show battery is being changed
                if(!changed){
                    print_to_display("Changing...");
                    
                }
            }
            if(order_completed){ // Runs if order is completed
                print_to_display("Complete.");
                oled_delay = millis();
            }
        } else if ((millis() - oled_delay) > 3000){ // Clears oled 3s after customer leaves. 
            print_to_display("");
        }
    };
}