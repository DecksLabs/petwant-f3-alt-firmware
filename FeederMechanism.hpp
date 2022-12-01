#pragma once

class FeederMechanism
{
public:
    void feed(uint8_t portions);
    void portionDetected();

private:
    uint8_t portionsToFeedLeft = 0;
    bool feeding = false;
};