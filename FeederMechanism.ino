#include "FeederMechanism.hpp"
#include "board.h"


void FeederMechanism::feed(uint8_t portions)
{
    portionsToFeedLeft = portions;
    feeding = true;
    motor.setState(true);
}

void FeederMechanism::portionDetected()
{
    if(feeding)
    {
        portionsToFeedLeft--;

        if(portionsToFeedLeft <= 0)
            motor.setState(false);
    }
}