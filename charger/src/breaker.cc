
#include <breaker.hh>
#include <connection.hh>

// Breaker: Start/Finish line for zumo with Sonar breaker beam. 
void loopBreakerCode()
{
    switch(build_phase){
        case true: // Not running breaker code so sensor can be set up, only spams ired_value.
            if((millis() - round_millis) > 500){
                round_millis = millis();
                Serial.println(analogRead(sonar_pin));
            }
            break;
        case false:
            if((millis() - round_millis) > sonar_delay ){ // Delay so sonar does not double activate.
                if(analogRead(sonar_pin)<2000){ // Saves round time if sonar sees Zumo.
                    Serial.print("Broke beam.");
                    updateTime();
                }
            }
            break;
    }
    
}

void updateTime()
{
    // Uses millis to get round time in milliseconds.
    round_millis = millis();
    unsigned long new_time = round_millis - previous_round_millis;
    Serial.print("Round time: "); Serial.println(new_time);

    // Sends round time in milliseconds to Node-red, which processes and displays on dashboard.
    sendTime(new_time);

    previous_round_millis = round_millis;
}

void sendTime(unsigned long new_time) // Sends updated time in millis to Node-red for processing.
{
    char timeString[8];
    dtostrf(new_time, 1, 2, timeString);
    Serial.println("Sending new time recorded update");
    client.publish("/breaker/in", timeString);
}
