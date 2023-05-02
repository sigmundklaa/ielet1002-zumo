
#ifndef BREAKER
#define BREAKER

#include <common.hh>

// Variables
    // PINS
    inline const int ired_pin = 34;
    inline const int ired_diode = 32; // Might not be wired to this ESP32 due to distance.

    // STATUS
    inline unsigned long ired_delay = 5000;
    inline unsigned long round_millis = 0;
    inline unsigned long previous_round_millis = 0;

    inline bool build_phase = true;

// Functions
void loopBreakerCode();
void updateTime();
void sendTime(unsigned long new_time);

#endif // BREAKER