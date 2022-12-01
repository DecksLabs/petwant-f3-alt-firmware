#include "TimeMgmt.hpp"
#include "Configuration.hpp"
#include "FeederMechanism.hpp"

extern Configuration configuration;
extern FeederMechanism feeder;

#pragma once

#include <TimeLib.h>
#include <TimeAlarms.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>

#include "Configuration.hpp"
#include "FeederMechanism.hpp"

extern Configuration configuration;
extern FeederMechanism feeder;

AlarmID_t TimeMgmt::alarms[8];

TimeMgmt::TimeMgmt() : 
    timeClient(nullptr) //FIXME: adjust for DST
{
    
}

void TimeMgmt::begin()
{
    timeClient = new NTPClient(ntpUDP, configuration.get().ntpServer.c_str(), 0);
    timeClient->begin();
}

void TimeMgmt::configureTimers()
{
    for(uint8_t i = 0, end = configuration.getFeedings().size(); i < end; ++i)
    {
        auto& feeding = configuration.getFeedings()[i];
        if (feeding.enabled)
        {
            Alarm.disable(alarms[i]);
            Alarm.free(alarms[i]);
        }
    }

    for(uint8_t i = 0, end = configuration.getFeedings().size(); i < end; ++i)
    {
        auto& feeding = configuration.getFeedings()[i];
        if (feeding.enabled)
            alarms[i] = Alarm.alarmRepeat(feeding.hour, feeding.minutes, 0, TimeMgmt::feedingTimerCb);
    }
}

String TimeMgmt::getFmtTimeString()
{
    //1995-12-17T03:24:00
    char timeStamp[24];
    sprintf(timeStamp, "%04d-%02d-%02dT%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
    return timeStamp;
}

void TimeMgmt::feedingTimerCb()
{
    AlarmID_t id = Alarm.getTriggeredAlarmId();

    for(uint8_t i = 0, end = configuration.getFeedings().size(); i < end; ++i)
    {
        if(TimeMgmt::alarms[i] == id && configuration.getFeedings()[i].enabled)
        {
            Serial.print("Feeding ");
            Serial.print(configuration.getFeedings()[i].portions);
            Serial.println(" portions...");
            feeder.feed(configuration.getFeedings()[i].portions);
        }
    }
}
