#pragma once

#include "board.h"
#include <limits.h>

struct LedBlinker
{
    static const int off = INT_MAX;
    static const int on = INT_MAX - 1;
    bool blinking = false;
    unsigned long lastChange;
    int period;

    void setPeriod(int period_ms)
    {
        if(off == period_ms)
        {
            led.setState(false);
            blinking = false;
        }
        else if(on == period_ms)
        {
            led.setState(true);
            blinking = false;
        }
        else
        {
            lastChange = millis();
            period = period_ms;
            blinking = true;
            led.setState(true);
        }
    }

    void loop()
    {
        if(!blinking)
            return;

        unsigned long now = millis();

        if(now - lastChange > period)
        {
            Serial.println("led blink");
            lastChange = now;
            led.setState(!led.outputState);
        }
    }
};
