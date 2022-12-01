#pragma once

#include <array>

class Configuration
{
public:
    struct Config {
        String ssid;
        String pswd;
        long timezone; //offset in seconds
        bool dstEnabled;
        String ntpServer;
        String hostname;
    };

    struct Feeding {
        bool enabled;
        uint8_t hour;
        uint8_t minutes;
        uint8_t portions;
    };

    Configuration();

    void read();
    void save();
    void init();
    void reset();
    bool exists();
    Config& get();

    std::array<Feeding, 8>& getFeedings();
    void saveFeedings();
    void loadFeedings();

private:
    Config config;
    std::array<Feeding, 8> feedings;
};