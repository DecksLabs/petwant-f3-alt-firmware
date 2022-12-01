#pragma once

#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Timezone.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#include "Configuration.hpp"
#include "FeederMechanism.hpp"

extern Configuration configuration;
extern FeederMechanism feeder;


class TimeMgmt
{
public:
    TimeMgmt();

    void begin();
    void configureTimers();

    inline void loop()
    {
        if(timeClient == nullptr)
            return;

        timeClient->update();
        Alarm.delay(0);

        if(timeClient->isTimeSet())
        {
            auto timeSt = timeStatus();
            if(timeSt == timeNeedsSync || timeSt == timeNotSet)
            {
                
                if(!configuration.get().dstEnabled)
                    setTime(timeClient->getEpochTime() + configuration.get().timezone);
                else
                {
                    setTime(plTime.toLocal(timeClient->getEpochTime()));
                }
            }
        }
    }

    String getFmtTimeString();

private:

    static void feedingTimerCb();

    char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    WiFiUDP ntpUDP;
    NTPClient *timeClient;
    TimeChangeRule plSummer = {"CEST", Last, Sun, Mar, 2, 120};
    TimeChangeRule plWinter = {"CET", Last, Sun, Oct, 2, 60};
    Timezone plTime = Timezone(plSummer, plWinter);
    static AlarmID_t alarms[8];
};
