
#ifndef BREAKER
#define BREAKER

#include <common.hh>

// Variables
    // PINS
    inline const int sonar_pin = 34;

    // STATUS
    inline unsigned long sonar_delay = 5000;
    inline unsigned long round_millis = 0;
    inline unsigned long previous_round_millis = 0;

    inline bool build_phase = true;

// Functions
void loopBreakerCode();
void updateTime();
void sendTime(unsigned long new_time);

#endif // BREAKER