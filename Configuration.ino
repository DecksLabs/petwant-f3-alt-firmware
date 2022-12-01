#include "Configuration.hpp"
#include <FS.h>
#include <ArduinoJson.h>

const char* configFilePath = "/config.json";
const char* feedingsFilePath = "/feedings.json";

void Configuration::read()
{
    File configFile = SPIFFS.open(configFilePath, "r");
    if (!configFile)
    {
        Serial.println("WRN: Can't open config file, setting defaults");

        config.hostname = "cat_feeder";
        config.ntpServer = "pool.ntp.org";
        config.dstEnabled = true;
        config.timezone = 3600;
        config.ssid = "";
        config.pswd = "";

        return;
    }

    StaticJsonDocument<1024> jsonDoc;
    deserializeJson(jsonDoc, configFile);

    config.pswd       = jsonDoc["pswd"].as<String>();
    config.ssid       = jsonDoc["ssid"].as<String>();
    config.dstEnabled = jsonDoc["dstEn"].as<bool>();
    config.ntpServer  = jsonDoc["ntpSrv"].as<String>();
    config.timezone   = jsonDoc["tzOffset"].as<int>();
    config.hostname   = jsonDoc["hostname"].as<String>();

    configFile.close();
}

void Configuration::save()
{
    SPIFFS.remove(configFilePath);

    File configFile = SPIFFS.open(configFilePath, "w");
    if (!configFile)
    {
        return;
    }

    DynamicJsonDocument jsonDoc(1024);

    jsonDoc["pswd"]     = config.pswd;
    jsonDoc["ssid"]     = config.ssid;
    jsonDoc["dstEn"]    = config.dstEnabled;
    jsonDoc["ntpSrv"]   = config.ntpServer;
    jsonDoc["tzOffset"] = config.timezone;
    jsonDoc["hostname"] = config.hostname;

    serializeJson(jsonDoc, configFile);

    configFile.close();
}

bool Configuration::exists()
{
    return SPIFFS.exists(configFilePath);
}

Configuration::Config& Configuration::get()
{
    return config;
}

void Configuration::init()
{
    if(!SPIFFS.begin())
    {
        Serial.println("WRN SPIFFS doesn't exist, formatting");
        SPIFFS.format();
    }

    SPIFFS.begin();
}

Configuration::Configuration()
{
    
}

std::array<Configuration::Feeding, 8>& Configuration::getFeedings()
{
    return feedings;
}

void Configuration::reset()
{
    SPIFFS.remove(configFilePath);
    SPIFFS.remove(feedingsFilePath);
}

void Configuration::saveFeedings()
{
    SPIFFS.remove(feedingsFilePath);

    File feedingsFile = SPIFFS.open(feedingsFilePath, "w");
    if (!feedingsFile)
    {
        return;
    }

    DynamicJsonDocument jsonDoc(1024);

    for(std::size_t i = 0, e = feedings.size(); i != e; ++i)
    {
        jsonDoc["feedings"][i]["enabled"]  = feedings[i].enabled;
        jsonDoc["feedings"][i]["hour"]     = feedings[i].hour;
        jsonDoc["feedings"][i]["minutes"]  = feedings[i].minutes;
        jsonDoc["feedings"][i]["portions"] = feedings[i].portions;
    }

    serializeJson(jsonDoc, feedingsFile);

    feedingsFile.close();
}

void Configuration::loadFeedings()
{
    File feedingsFile = SPIFFS.open(feedingsFilePath, "r");
    if (!feedingsFile)
    {
        Serial.println("WRN: Can't open feedings file");
        return;
    }

    StaticJsonDocument<1024> jsonDoc;
    deserializeJson(jsonDoc, feedingsFile);

    for(std::size_t i = 0, e = feedings.size(); i != e; ++i)
    {
        feedings[i].enabled  = jsonDoc["feedings"][i]["enabled"].as<bool>();
        feedings[i].hour     = jsonDoc["feedings"][i]["hour"].as<uint8_t>();
        feedings[i].minutes  = jsonDoc["feedings"][i]["minutes"].as<uint8_t>();
        feedings[i].portions = jsonDoc["feedings"][i]["portions"].as<uint8_t>();
    }

    feedingsFile.close();
}
