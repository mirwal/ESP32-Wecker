#ifndef WLANMANAGER_H
#define WLANMANAGER_H

#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WLANManager
{
public:
    WLANManager(AsyncWebServer &server);

    void begin();
    void handleWiFi();
    bool isConnected();
    void resetCredentials();

private:
    void startAPMode();
    void loadCredentials();
    void saveCredentials(const String &ssid, const String &pass);
    void setupWebPortal();

    AsyncWebServer &webServer;
    Preferences preferences;

    String ssid;
    String password;
    bool apMode = false;
};

#endif
