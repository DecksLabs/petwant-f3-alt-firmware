#pragma once

#include "board.h"

struct InputInt
{
    const uint8_t pin;
    const uint8_t triggerType;
    const uint8_t inputMode;
    bool triggered;
    unsigned long lastTimeTriggered;
    void (*cb)(void);

    void init(void(* intRoutine)(void))
    {
        pinMode(pin, inputMode);
        attachInterrupt(digitalPinToInterrupt(pin), intRoutine, triggerType);
    }

    void pinIntHandler()
    {
        if(!triggered)
        {
            lastTimeTriggered = millis();
            triggered = true;
        }
    }

    bool read()
    {
        return digitalRead(pin) == HIGH;
    }

    void setIntCallback(void(* intRoutine)(void))
    {
        cb = intRoutine;
    }

    void loop()
    {
        if(!triggered || cb == nullptr)
            return;

        const unsigned long now = millis();
        if(now - lastTimeTriggered > 25)
        {
            if(!read())
                cb();

            triggered = false;
        }
    }

    InputInt(const uint8_t pin, const uint8_t triggerType, const uint8_t inputMode) :
        pin(pin), triggerType(triggerType),
        inputMode(inputMode), triggered(false),
        lastTimeTriggered(0), cb(nullptr)
        {}
};

struct Output
{
    enum ELogic
    {
        ELogic_ActiveLow,
        ELogic_ActiveHigh
    };

    const uint8_t pin;
    const uint8_t outputMode;
    const ELogic logic;
    bool outputState;

    void init()
    {
        pinMode(pin, outputMode);
    }

    void setState(bool state)
    {
        outputState = state;

        if(state)
            digitalWrite(pin, logic == ELogic_ActiveHigh ? HIGH : LOW);
        else
            digitalWrite(pin, logic == ELogic_ActiveHigh ? LOW : HIGH);
    }
};
