#pragma once

#include <ESP8266WebServer.h>

class WebInterface
{
public:
    WebInterface() : server(80)
    {
    }

    void init();
    void loop();

private:
    bool handleFileRead(String path);
    void GET_settings();
    void POST_settings();
    void GET_schedule();
    void POST_schedule();
    void GET_time();
    void POST_feed();
    void POST_update();
    void UPLOAD_POST_update();

    ESP8266WebServer server;
};
