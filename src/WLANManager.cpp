#include <Arduino.h>
#include "WLANManager.h"

#define AP_SSID "ESP32_Setup"
#define AP_PASSWORD "12345678"

WLANManager::WLANManager(AsyncWebServer &server) : webServer(server) {}

void WLANManager::begin()
{
    preferences.begin("wlan", false);
    loadCredentials();

    if (ssid.length() > 0)
    {
        WiFi.begin(ssid.c_str(), password.c_str());
        unsigned long startAttemptTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
        {
            delay(500);
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        startAPMode();
    }

    // setupWebPortal();
    preferences.end();
}
void WLANManager::resetCredentials()
{
    preferences.clear();
    preferences.end();
    Serial.println("WLAN-Daten gelöscht! Neustart...");
    delay(1000);
    ESP.restart();
}

void WLANManager::handleWiFi()
{
    if (!apMode && WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WLAN nicht verbunden – versuche 10 Sekunden lang eine Verbindung aufzubauen...");

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
        {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nVerbindung erfolgreich!");
        }
        else
        {
            Serial.println("\nVerbindung fehlgeschlagen – Starte Access Point");
            startAPMode();
        }
    }
}

bool WLANManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void WLANManager::startAPMode()
{
    apMode = true;
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.println("Access Point gestartet: " + String(AP_SSID));
}

void WLANManager::loadCredentials()
{
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("pass", "");
}

void WLANManager::saveCredentials(const String &newSsid, const String &newPass)
{
    preferences.putString("ssid", newSsid);
    preferences.putString("pass", newPass);
}

void WLANManager::setupWebPortal()
{
    webServer.on("/status", HTTP_GET, [&](AsyncWebServerRequest *request)
                 {
    String json = "{";
    json += "\"status\":\"" + String(isConnected() ? "Verbunden" : "AP-Modus") + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";
    request->send(200, "application/json", json); });

    webServer.on("/", HTTP_GET, [&](AsyncWebServerRequest *request)
                 {
        String html = R"rawliteral(<!DOCTYPE html>
<html lang="de">

<head>
    <meta charset="UTF-8">
    <title>WLAN Setup</title>
</head>

<body>
    <h2>WLAN Setup</h2>
    <form id="wifiForm">
        SSID: <input type="text" id="ssid"><br><br>
        Passwort: <input type="password" id="pass"><br><br>
        <input type="submit" value="Speichern">
    </form>
    <p id="status"></p>
    <script>
        const form = document.getElementById('wifiForm');
        const status = document.getElementById('status');

        form.addEventListener('submit', (e) => {
            e.preventDefault();
            const ssid = document.getElementById('ssid').value;
            const pass = document.getElementById('pass').value;
            fetch('/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ssid: ssid, pass: pass })
            })
                .then(response => response.text())
                .then(data => {
                    status.innerText = data;
                });
        });

        // Status abrufen
        fetch('/status')
            .then(response => response.json())
            .then(data => {
                status.innerText = "📶 Status: " + data.status + " | IP: " + data.ip;
            });
    </script>
</body>

</html>
        )rawliteral";
        request->send(200, "text/html", html); });

    webServer.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                 {
            String json;
            for (size_t i = 0; i < len; i++) {
                json += (char)data[i];
            }

            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, json);

            if (!error) {
                String newSsid = doc["ssid"].as<String>();
                String newPass = doc["pass"].as<String>();
                saveCredentials(newSsid, newPass);
                request->send(200, "text/plain", "✅ WLAN-Daten gespeichert! ESP startet neu...");
                delay(1000);
                ESP.restart();
            } else {
                request->send(400, "text/plain", "❌ JSON-Parsing fehlgeschlagen");
            } });
}