#include <ESP8266WiFi.h>
#include "TimeMgmt.hpp"
#include "board.h"
#include "LedBlinker.hpp"
#include "FeederMechanism.hpp"
#include "Configuration.hpp"
#include "WebInterface.hpp"
#include <ESP8266mDNS.h>

TimeMgmt timeManager;
FeederMechanism feeder;
Configuration configuration;
WebInterface webInterface;
LedBlinker ledBlinker;

ICACHE_RAM_ATTR void buttonPressInt()
{
    bottomButton.pinIntHandler();
}

void buttonPressIntCb()
{
    Serial.println("BTN PRESS!!!");
    feeder.feed(1);
}

ICACHE_RAM_ATTR void portionDetInt()
{
    portionDetector.pinIntHandler();
}

void portionDetetectorIntCb()
{
    Serial.println("Portion det!!!");
    feeder.portionDetected();
}

void resetMechanism()
{
    if(!portionDetector.read())
        feeder.feed(1);
}

void configureAPMode()
{
    IPAddress localIp(192,168,1,1);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);
  
    Serial.println("Creating AP for configuration");
    WiFi.softAPConfig(localIp, gateway, subnet);
    WiFi.softAP("cat_feeder");
}

void configureSTAMode()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(configuration.get().ssid, configuration.get().pswd);
    Serial.println("");
    
    Serial.print("Connecting to ");
    Serial.println(configuration.get().ssid);
}

void setup(void)
{
    Serial.begin(115200);
    while (!Serial) continue;
    delay(500);
    
    resetMechanism();

    motor.init();
    led.init();
    configuration.init();
    bottomButton.init(buttonPressInt);
    bottomButton.setIntCallback(buttonPressIntCb);
    portionDetector.init(portionDetInt);
    portionDetector.setIntCallback(portionDetetectorIntCb);
    ledBlinker.setPeriod(250);

    configuration.read();
    configuration.loadFeedings();

    if(!configuration.exists() || !bottomButton.read())
    {
        Serial.println("Endering configuration mode (no config file or reset pressed)");
        configureAPMode();
    }
    else
    {
        Serial.println("Attempting connection to WiFi");
        configureSTAMode();

        //wait for 30s to connect to WiFi
        for(uint8_t i = 0; i < 60; ++i)
        {
            if(WiFi.status() != WL_CONNECTED)
            {
                Serial.print(".");
                ledBlinker.loop();
                delay(500);
            }
            else
                break;
        }
    }

    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connection to WiFi failed, creating AP for configuration");
        configureAPMode();
    }
    else //turn on timers only after successful connection to WiFi
    {
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(configuration.get().ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        timeManager.configureTimers();
        timeManager.begin();
        ledBlinker.setPeriod(LedBlinker::on);
    }
    
    MDNS.begin(configuration.get().hostname);
    MDNS.addService("http", "tcp", 80);
    webInterface.init();
}
 
void loop(void)
{
    ledBlinker.loop();
    bottomButton.loop();
    portionDetector.loop();
    jamDetector.loop();
    timeManager.loop();
    webInterface.loop();
    MDNS.update();
}
