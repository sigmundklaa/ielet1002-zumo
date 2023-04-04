
#pragma once

#include <Arduino.h>
#include <iostream>

class PubSubClient
{
  public:
    boolean
    publish(const char* topic, const uint8_t* payload, unsigned int plength)
    {

        return 0;
    }

    PubSubClient&
    setCallback(void (*callback)(char*, uint8_t*, unsigned int))
    {
        return *this;
    }
};