// V 2.0
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "TimeHelper.h"
#include "DisplayHelper.h"
#include "SPIFFS.h"
#include "Wecker.h"
#include "DaylightTime.h"
#include "WLANManager.h"

// ====== defines ======
#define STOP_BUTTON 4
#define RESET_PIN 4
#define INFO_INTERVAL 5000   // 5 Sekunden Status-Anzeige durchwechseln
#define RESET_HOLD_TIME 2000 // 2 Sekunden

// ====== Objekte ======
Wecker wecker;
DaylightTime dst;
LiquidCrystal_I2C lcd(0x27, 20, 4);
DisplayHelper display(lcd);
HardwareSerial mp3Serial(1);
DFRobotDFPlayerMini dfPlayer;
AsyncWebServer server(80);
WLANManager wlanManager(server);
Preferences prefs;

// ====== WLAN ======
//
// const char *ssid = "mirwal";
// const char *password = "39942148480659775601";

// ====== bools =====
bool alarmActive = false;

// ====== variablen ===
unsigned long lastInfoChange = 0;
int currentInfoIndex = 0;
unsigned long buttonPressTime = 0;
char TimeString[21];
unsigned long lastTimeUpdate = 0;

// ====== Prototypen ====
void showIP();
void showWeckzeit();
void showAlarmStatus();
void showDate();
void updateDisplay();

// ====== typedef =========
typedef void (*InfoFunc)();
InfoFunc infoFunctions[] = {showIP, showWeckzeit, showAlarmStatus, showDate};
const int infoCount = sizeof(infoFunctions) / sizeof(infoFunctions[0]);

// ====== Helfer ======
String getFormattedTime()
{
  struct tm timeinfo;
  if (WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo))
  {
    int offset = dst.getOffset(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour);
    time_t now = mktime(&timeinfo) + offset;
    struct tm *localTime = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", localTime);
    return String(buffer);
  }
  return "--:--:--";
}

void showIP()
{

  IPAddress ip;

  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
  {
    ip = WiFi.softAPIP(); // Access Point IP
  }
  else
  {
    ip = WiFi.localIP(); // normale IP als Client im WLAN
  }
  display.setLine(3, "IP: " + ip.toString());
}

void showWeckzeit()
{
  int h, m;
  wecker.getWeckzeit(h, m);
  String text = "Weckzeit: " + String(h) + ":" + (m < 10 ? "0" : "") + String(m);
  display.setLine(3, text);
}

void showAlarmStatus()
{
  display.setLine(3, "Alarm: " + String(wecker.isActive() ? "An " : "Aus") + "        ");
}

void showDate()
{

  struct tm timeinfo;
  if (getCorrectedLocalTime(timeinfo))
  {
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y", &timeinfo);
    String Datum = "Datum:" + String(buffer);
    display.setLine(3, String(buffer));
  }
}

void handleButton()
{

  if (alarmActive && digitalRead(STOP_BUTTON) == LOW)
  {
    Serial.println("Alarm gestoppt!");
    dfPlayer.stop();
    alarmActive = false;
    delay(300); // Prellen vermeiden
    updateDisplay();
  }

  // Reset-Funktion: Button 2 Sekunden gedrückt halten
  if (digitalRead(STOP_BUTTON) == LOW)
  {
    if (buttonPressTime == 0)
    {
      buttonPressTime = millis();
    }
    else if (millis() - buttonPressTime > RESET_HOLD_TIME)
    {
      bool neuerStatus = !wecker.isActive();
      wecker.setActive(neuerStatus);
      wecker.saveToPreferences();
      buttonPressTime = 0;
      Serial.println(neuerStatus ? "Wecker aktiviert" : "Wecker deaktiviert");

      display.setLine(3, (neuerStatus ? "Wecker aktiv      " : "Wecker deaktiviert"));

      updateDisplay();
      delay(3000); // Anzeigezeit
    }
  }
  else
  {
    buttonPressTime = 0; // Reset, wenn Taste losgelassen
  }
}
void checkAlarm()
{
  if (wecker.shouldTriggerAlarm() && !alarmActive)
  {
    Serial.println("ALARM! Weckzeit erreicht!");
    dfPlayer.volume(15);
    dfPlayer.play(1); // Track 001.mp3
    alarmActive = true;
  }
}

void setupWebserver()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    DynamicJsonDocument doc(128);
    doc["time"] = getFormattedTime();

    int stunde, minute;
    wecker.getWeckzeit(stunde, minute);
    char weckzeit[6];
    snprintf(weckzeit, sizeof(weckzeit), "%02d:%02d", stunde, minute);
    doc["alarm"] = weckzeit;
    doc["active"] = wecker.isActive();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json); });

  server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, data, len);
    if (!error) {
      String time = doc["time"];
      bool active = doc["active"];

      int stunde = time.substring(0, 2).toInt();
      int minute = time.substring(3, 5).toInt();

      wecker.setWeckzeit(stunde, minute);
      wecker.setActive(active);
      wecker.saveToPreferences();
      
      request->send(200, "text/plain", "Weckzeit gespeichert");
    } else {
      request->send(400, "text/plain", "JSON Fehler");
    } });

  server.begin();
}
String getTimeString()
{
  struct tm timeinfo;
  if (WiFi.status() == WL_CONNECTED && getCorrectedLocalTime(timeinfo))
  {
    char buffer[21];
    if (wecker.isActive())
      strftime(buffer, sizeof(buffer), "%H:%M ALARM %d.%m.%y", &timeinfo);
    else
      strftime(buffer, sizeof(buffer), "%H:%M       %d.%m.%y", &timeinfo);
    return String(buffer);
  }
  return "--:--        --.--.--";
}

String getWeckzeitString()
{
  int stunde, minute;
  wecker.getWeckzeit(stunde, minute);
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", stunde, minute);
  return String(buffer);
}
void updateDisplay()
{
  display.setLine(0, getTimeString());
  display.setLine(1, getWeckzeitString());

  // Zyklische Anzeige auf Zeile 3 des LCDs:
  // IP → Weckzeit → Alarmstatus → Datum
  if (millis() - lastInfoChange > INFO_INTERVAL)
  {
    infoFunctions[currentInfoIndex](); // Funktionszeiger aus dem Array aufrufen → zeigt Info auf Zeile 3
    currentInfoIndex = (currentInfoIndex + 1) % infoCount;
    lastInfoChange = millis();
  }

  display.show();
}

void setup()
{
  pinMode(STOP_BUTTON, INPUT_PULLUP);
  Serial.begin(115200);
  delay(500);
  Serial.println("Starte Wecker...");
  wecker.loadFromPreferences();

  // LCD
  display.begin();
  display.setLine(0, "Wecker startet...");
  updateDisplay(); // display.show();

  // SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS konnte nicht gestartet werden");
    display.setLine(3, "SPIFFS Fehler");
    updateDisplay(); // display.show();

    return;
  }

  // WLAN

  if (digitalRead(RESET_PIN) == LOW)
  { // Taste gedrückt beim Start
    display.setLine(1, "Setup-Taste erkannt");
    updateDisplay(); // display.show();
    Serial.println("Setup-Taste erkannt → WLAN-Daten löschen...");
    wlanManager.resetCredentials();
    Serial.print("WLAN-Daten gelöscht! ");

    // Warte, bis Taste losgelassen wurde
    while (digitalRead(RESET_PIN) == LOW)
    {
      delay(200); // entprellter Poll
    }
    Serial.println("Neustart...");

    delay(1000);

    ESP.restart();
  }

  display.setLine(1, "Verbinde WLAN...");
  updateDisplay(); // display.show();
  wlanManager.begin();

  Serial.println("\nWLAN verbunden");
  display.setLine(1, "WLAN verbunden");

  // IP anzeigen

  IPAddress ip;

  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
  {
    ip = WiFi.softAPIP(); // Access Point IP
  }
  else
  {
    ip = WiFi.localIP(); // normale IP als Client im WLAN
  }

  display.setLine(2, "IP: " + ip.toString());

  // Zeit
  configTime(3600, 0, "pool.ntp.org", "time.nist.gov");

  // DFPlayer
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);
  if (dfPlayer.begin(mp3Serial))
  {
    Serial.println("\nDFPlayer verbunden");
    display.setLine(3, "MP3 bereit");
  }
  else
  {
    Serial.println("\nDFPlayer Fehler");
    display.setLine(3, "MP3 Fehler");
  }

  // Webserver erst NACH WLAN-Start!
  setupWebserver();
}

void loop()
{
  wlanManager.handleWiFi();
  handleButton();
  checkAlarm();

  if (millis() - lastTimeUpdate > 5000)
  {
    lastTimeUpdate = millis();
    updateDisplay();
  }
}
