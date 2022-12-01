#include "WebInterface.hpp"
#include "Configuration.hpp"
#include "TimeMgmt.hpp"
#include "FeederMechanism.hpp"
#include <ArduinoJson.h>
#include <FS.h>
#include <flash_hal.h>

extern Configuration configuration;
extern TimeMgmt timeManager;
extern FeederMechanism feeder;

namespace
{
    String getContentType(ESP8266WebServer& server, String& filename){
        if(server.hasArg("download")) return "application/octet-stream";
        else if(filename.endsWith(".htm")) return "text/html";
        else if(filename.endsWith(".html")) return "text/html";
        else if(filename.endsWith(".css")) return "text/css";
        else if(filename.endsWith(".js")) return "application/javascript";
        else if(filename.endsWith(".png")) return "image/png";
        else if(filename.endsWith(".gif")) return "image/gif";
        else if(filename.endsWith(".jpg")) return "image/jpeg";
        else if(filename.endsWith(".ico")) return "image/x-icon";
        else if(filename.endsWith(".xml")) return "text/xml";
        else if(filename.endsWith(".pdf")) return "application/x-pdf";
        else if(filename.endsWith(".zip")) return "application/x-zip";
        else if(filename.endsWith(".gz")) return "application/x-gzip";
        return "text/plain";
    }

    static const char successResponseFmtString[] PROGMEM =
        "<META http-equiv=\"refresh\" content=\"3;URL=%s\"><p><center>%s</center></p>";

    static const char updateResponseFmtString[] PROGMEM =
        "<META http-equiv=\"refresh\" content=\"15;URL=/\"><p><center>Update %s! Rebooting...</center></p>";
}

void WebInterface::GET_settings()
{
    auto& config = configuration.get();

    String serializedConfig;
    DynamicJsonDocument jsonDoc(1024);

    jsonDoc["ssid"]     = config.ssid;
    jsonDoc["pswd"]     = "##dummy##";
    jsonDoc["dstEn"]    = config.dstEnabled;
    jsonDoc["ntpSrv"]   = config.ntpServer;
    jsonDoc["tz"]       = config.timezone / 3600;
    jsonDoc["hostname"] = config.hostname;

    serializeJson(jsonDoc, serializedConfig);

    server.send(200, "text/json", serializedConfig);
}

void WebInterface::POST_feed()
{
    if(server.hasArg("portions"))
    {
        feeder.feed(server.arg("portions").toInt());
    }

    char response[256];
    sprintf(response, successResponseFmtString, "/", "Feeding...");
    server.send(200, "text/html", response);
}

void WebInterface::POST_settings()
{
    if(server.arg("pswd") != "##dummy##") configuration.get().pswd = server.arg("pswd");

    configuration.get().ssid       = server.arg("ssid");
    configuration.get().dstEnabled = server.hasArg("dstEn");
    configuration.get().ntpServer  = server.arg("ntpSrv");
    configuration.get().timezone   = server.arg("tz").toInt() * 3600;
    configuration.get().hostname   = server.arg("hostname");

    configuration.save();

    server.sendHeader("Connection", "close");
    char response[256];
    sprintf(response, updateResponseFmtString, "config OK");
    server.client().setNoDelay(true);
    server.send(200, "text/html", response);
    server.stop();
    delay(100);
    ESP.restart();
}

void WebInterface::GET_time()
{
    server.send(200, "text/plain", timeManager.getFmtTimeString());
}

void WebInterface::GET_schedule()
{
    auto& feedings = configuration.getFeedings();

    DynamicJsonDocument jsonDoc(1024);

    for(std::size_t i = 0, e = feedings.size(); i != e; ++i)
    {
        char field[16];

        sprintf(field, "f%d_a", i);
        jsonDoc[field] = feedings[i].enabled;

        sprintf(field, "f%d_h", i);
        jsonDoc[field] = feedings[i].hour;

        sprintf(field, "f%d_m", i);
        jsonDoc[field] = feedings[i].minutes;

        sprintf(field, "f%d_n", i);
        jsonDoc[field] = feedings[i].portions;
    }

    String serialzedFeedings;
    serializeJson(jsonDoc, serialzedFeedings);

    server.send(200, "text/json", serialzedFeedings);
}

void WebInterface::POST_schedule()
{
    auto& feedings = configuration.getFeedings();
    for(std::size_t i = 0, e = feedings.size(); i != e; ++i)
    {
        char field[16];

        sprintf(field, "f%d_a", i);
        feedings[i].enabled = server.hasArg(field) ? true : false;

        sprintf(field, "f%d_h", i);
        feedings[i].hour = server.arg(field).toInt();

        sprintf(field, "f%d_m", i);
        feedings[i].minutes = server.arg(field).toInt();

        sprintf(field, "f%d_n", i);
        feedings[i].portions = server.arg(field).toInt();
    }

    configuration.saveFeedings();
    timeManager.configureTimers();

    char response[256];
    sprintf(response, successResponseFmtString, "/", "Saved ok");
    server.send(200, "text/html", response);
}

void WebInterface::POST_update()
{
    server.sendHeader("Connection", "close");
    char response[256];
    sprintf(response, updateResponseFmtString, (Update.hasError()) ? "FAIL" : "OK");
    server.client().setNoDelay(true);
    server.send(200, "text/html", updateResponseFmtString);
    server.stop();
    delay(100);
    ESP.restart();
}

void WebInterface::UPLOAD_POST_update()
{
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        
        if(upload.name == "filesystem")
        {
            close_all_fs();
            
            if (!Update.begin(FS_PHYS_SIZE, U_FS))//start with max available size
            {
                Update.printError(Serial);
            }
        }
        else
        {
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace, U_FLASH)) //start with max available size
            {
                Update.printError(Serial);
            }
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))// true to set the size to the current progress
        {
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
        else
        {
            Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
    }
    yield();
}

void WebInterface::init()
{
    server.on("/settings", HTTP_GET, [this](){
      GET_settings();
    });

    server.on("/settings", HTTP_POST, [this](){
      POST_settings();
    });

    server.on("/feed", HTTP_POST, [this](){
      POST_feed();
    });

    server.on("/time", HTTP_GET, [this](){
      GET_time();
    });

    server.on("/schedule", HTTP_GET, [this](){
      GET_schedule();
    });

    server.on("/schedule", HTTP_POST, [this](){
      POST_schedule();
    });

    server.on("/update", HTTP_POST,
    [this](){
      POST_update();
    },
    [this](){
      UPLOAD_POST_update();
    }
    );

    server.onNotFound([this](){
      if(!handleFileRead(server.uri()))
        server.send(404, "text/plain", "404: FileNotFound");
    });

    server.begin();
    Serial.println("HTTP server started");
}

bool WebInterface::handleFileRead(String path)
{
  if(path.endsWith("/"))
    path += "index.html";

  path = "/web" + path;

  String contentType = getContentType(server, path);

  File file = SPIFFS.open(path, "r");
  if (file == NULL)
    return false;

  size_t sent = server.streamFile(file, contentType);
  file.close();
  return true;
}

void WebInterface::loop()
{
    server.handleClient();
}