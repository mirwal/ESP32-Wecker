// V 2.0
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "TimeHelper.h"
#include "DisplayHelper.h"
#include "SPIFFS.h"
#include "Wecker.h"
#include "DaylightTime.h"
#include "WLANManager.h"

// ====== defines ======
#define STOP_BUTTON 4
#define RESET_PIN 4
#define MUTE_PIN 25                    // audiomodul mute
#define INFO_INTERVAL 30000            // 5 Sekunden Status-Anzeige durchwechseln
#define WETTER_UPDATE_INTERVAL 3600000 // 1 Stunde = 3.600.000 ms
#define RESET_HOLD_TIME 2000           // 2 Sekunden

// ====== Objekte ======
Wecker wecker;
DaylightTime dst;
DisplayHelper display;
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

unsigned long lastWetterUpdate = 0;
unsigned long lastInfoChange = 0;
unsigned long buttonPressTime = 0;
unsigned long lastTimeUpdate = 0;

int currentInfoIndex = 0;
String wetterText = "unbekannt";
String infoGB = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor ENDE ENDE";
const char *soundNames[] = {
    "Unbekannt",        // Index 0 – wird nie genutzt
    "Klassisch",        // Index 1
    "Vogelgezwitscher", // Index 2
    "Retro-Klingel",    // Index 3
    "Sirene"            // Index 4
};

// ====== Prototypen ====
void showIP();
void showWeckzeit();
void showAlarmStatus();
void showSound();
void updateDisplay();
void showWetter();

// ====== typedef =========
typedef void (*InfoFunc)();
InfoFunc infoFunctions[] = {showIP, showWetter, showWeckzeit, showAlarmStatus, showSound};
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
  display.setLine(3, "IP: " + wlanManager.getMyIP());
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

void showSound()
{
  String soundInfo = soundNames[wecker.getSavedSound()];
  display.setLine(3, soundInfo);
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

    dfPlayer.volume(wecker.getSavedVolume());
    // dfPlayer.volume(15);
    dfPlayer.play(wecker.getSavedSound());
    // dfPlayer.play(1); // Track 001.mp3
    alarmActive = true;
  }
}

void setupMainWebinterface()
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
    doc["sound"] = wecker.getSavedSound();
    doc["volume"] = wecker.getSavedVolume();

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

  server.on("/setSound", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
            {
    String body;
    for (size_t i = 0; i < len; i++) body += (char)data[i];

    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, body);

    if (!error && doc.containsKey("sound"))
    {
        uint8_t sound = doc["sound"];
        if (sound >= 1 && sound <= 31)
        {
          wecker.setSavedSound(sound);

            request->send(200, "text/plain", "🎵 Sound gespeichert: " + String(sound));
        }
        else
        {
            request->send(400, "text/plain", "❌ Ungültiger Sound-Wert");
        }
    }
    else
    {
        request->send(400, "text/plain", "❌ JSON-Fehler");
    } });

  server.on("/setVolume", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
            {
              String body;
              for (size_t i = 0; i < len; i++) body += (char)data[i];

              DynamicJsonDocument doc(128);
              DeserializationError error = deserializeJson(doc, body);

              if (!error && doc.containsKey("volume"))
              {
              uint8_t volume = doc["volume"];
              if (volume >= 1 && volume <= 25)
              {
                wecker.setSavedVolume(volume);

                  request->send(200, "text/plain", "🔊 Volume gespeichert: " + String(volume));
              }
              else
              {
                  request->send(400, "text/plain", "❌ Ungültiger Volume-Wert");
              }
              }
              else
              {
              request->send(400, "text/plain", "❌ JSON-Fehler");
              } });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                dfPlayer.stop();
                request->send(200, "text/plain", "🎵 Sound gestoppt"); });

  server.on("/play", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        if (request->hasParam("sound"))
        {
            int sound = request->getParam("sound")->value().toInt();
            if (sound >= 1 && sound <= 31)
            {
              dfPlayer.volume(wecker.getSavedVolume());
              Serial.printf("🎵 Play Sound %d mit Volume %d\n", sound, wecker.getSavedVolume());
                dfPlayer.play(sound);  // spielt z. B. 002.mp3
                request->send(200, "text/plain", "🎵 Sound wird abgespielt");
            }
            else
            {
                request->send(400, "text/plain", "❌ Ungültiger Sound-Wert");
            }
        }
        else
        {
            request->send(400, "text/plain", "❌ Kein Sound angegeben");
        } });

  server.begin();
}

String getWeckzeitString()
{
  int stunde, minute;
  wecker.getWeckzeit(stunde, minute);
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", stunde, minute);
  return String(buffer);
}
String getTimeString()
{
  struct tm timeinfo;
  if (WiFi.status() == WL_CONNECTED && getCorrectedLocalTime(timeinfo))
  {
    char buffer[21];
    char zeitStr[6];  // "HH:MM" + \0
    char datumStr[9]; // "DD.MM.YY" + \0

    strftime(zeitStr, sizeof(zeitStr), "%H:%M", &timeinfo);
    strftime(datumStr, sizeof(datumStr), "%d.%m.%y", &timeinfo);

    String statusStr = wecker.isActive() ? getWeckzeitString() : " OFF ";
    snprintf(buffer, sizeof(buffer), "%s %s %s",
             zeitStr, statusStr.c_str(), datumStr);

    return String(buffer);
  }
  return "--:--        --.--.--";
}

void updateDisplay()
{
  display.setLine(0, getTimeString());
  display.setScrollingLineWords(1, wetterText, 5000);
  // Zyklische Anzeige auf Zeile 3 des LCDs:
  // IP → Weckzeit → Alarmstatus → Datum ...
  if (millis() - lastInfoChange > INFO_INTERVAL)
  {
    infoFunctions[currentInfoIndex](); // Funktionszeiger aus dem Array aufrufen → zeigt Info auf Zeile 3
    currentInfoIndex = (currentInfoIndex + 1) % infoCount;
    lastInfoChange = millis();
  }

  display.show();
}
String getLetzterEintrag()
{
  WiFiClientSecure client;
  client.setInsecure(); // das ist hier gültig!
  HTTPClient http;
  http.begin("https://mirwal.de/api/letzter_eintrag.php");
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    int length = http.getSize();
    if (length < 800)
    {
      infoGB = http.getString();

      // Ersetze verschiedene Arten von Zeilenumbrüchen
      infoGB.replace("\r\n", "  ");
      infoGB.replace("\n", "    ");
      infoGB.replace("\r", " ");

      if (infoGB.length() > 400)
      {
        infoGB = infoGB.substring(0, 397) + "...   * * *   ";
      }
      else
      {
        infoGB += "   * * *   ";
      }

      http.end();
      return infoGB;
    }
  }
  else
  {
    http.end();
    return "Fehler HTTP" + String(httpCode);
  }
  http.end();
  return "length < 200";
}

void showWetter()
{
  Serial.print("Wetter: ");
  Serial.println(wetterText);
  display.setScrollingLine(3, wetterText);
}

void updateWetter(String &wetterText)
{
  HTTPClient http;
  http.begin("http://mirwal.de/api/wetter.php");
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    WiFiClient *stream = http.getStreamPtr();
    char buffer[122];
    int len = 0;

    while (http.connected() && stream->available() && len < 120)
    {
      char c = stream->read();
      buffer[len++] = c;
    }
    buffer[len] = '\0';

    // Zeilenumbrüche escapen
    //  wetterText = String(buffer);

    // API Müll entfernen am amfang und am ende
    String raw = String(buffer);
    int start = raw.indexOf('\n') + 1;
    int end = raw.indexOf('\n', start);

    wetterText = raw.substring(start, end);

    wetterText.replace("\r\n", " ");
    wetterText.replace("\n", " ");
    wetterText.replace("\r", " ");

    wetterText.replace("🌩️", " "); // Gewitter
    wetterText.replace("🌦️", " "); // Sprühregen
    wetterText.replace("🌧️", " "); // Regen
    wetterText.replace("❄️", " ");  // Schnee
    wetterText.replace("🌫️", " "); // Nebel
    wetterText.replace("☀️", " ");  // Klar
    wetterText.replace("🌤️", " "); // leicht bewölkt
    wetterText.replace("☁️", " ");  // bewölkt
    wetterText.replace("🌈", " "); // Fallback
    wetterText.replace("°", "\x04");
    // 1. Umlaute vorher auf Custom-LCD-Zeichen mappen
    wetterText.replace("ä", "\x00"); // LCD.write(0) = ä
    wetterText.replace("ö", "\x01"); // LCD.write(1) = ö
    wetterText.replace("ü", "\x02"); // LCD.write(2) = ü
    wetterText.replace("ß", "\x03"); // LCD.write(3) = ß
  }
  for (int i = 0; i < wetterText.length(); i++)
  {
    if ((uint8_t)wetterText[i] > 127 && wetterText[i] < 0xF0)
    {
      wetterText.remove(i, 1);
      i--; // da Zeichen verschoben wurde
    }
  }
  lastWetterUpdate = millis();

  http.end();
}
void setup()
{
  pinMode(MUTE_PIN, OUTPUT);
  digitalWrite(MUTE_PIN, HIGH);
  pinMode(STOP_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);
  delay(500);
  Serial.println("Starte Wecker...");
  wecker.loadFromPreferences();

  // LCD
  display.begin();
  display.setLine(3, " baud: 115200 ");
  display.show();
  delay(8000);

  // SPIFFS
  // pio run --target uploadfs
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS konnte nicht gestartet werden");
    display.setLine(3, "SPIFFS Fehler");
    display.show();

    return;
  }

  // WLAN

  if (digitalRead(RESET_PIN) == LOW)
  { // Taste gedrückt beim Start
    display.setLine(1, "Setup-Taste erkannt");
    display.show();
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
  display.show();
  wlanManager.begin();

  Serial.println("\nWLAN verbunden");
  display.setLine(1, "WLAN verbunden");

  // IP anzeigen

  display.setLine(2, "IP: " + wlanManager.getMyIP());

  // Zeit
  configTime(3600, 0, "pool.ntp.org", "time.nist.gov");

  // DFPlayer
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);
  if (dfPlayer.begin(mp3Serial))
  {
    Serial.println("\nDFPlayer verbunden");
    display.setLine(3, "MP3 bereit");
    dfPlayer.play(31);
    digitalWrite(MUTE_PIN, LOW);
  }
  else
  {
    Serial.println("\nDFPlayer Fehler");
    display.setLine(3, "MP3 Fehler");
  }

  display.show();
  // Webserver erst NACH WLAN-Start!
  setupMainWebinterface();
  delay(5000);
  display.setScrollingLine(2, getLetzterEintrag());
  updateWetter(wetterText);
  dfPlayer.stop();
}

void loop()
{
  wlanManager.handleWiFi();
  handleButton();
  checkAlarm();

  delay(200);

  if (millis() - lastTimeUpdate > INFO_INTERVAL)
  {
    lastTimeUpdate = millis();
    updateDisplay();
  }
  if (millis() - lastWetterUpdate > WETTER_UPDATE_INTERVAL)
  {
    updateWetter(wetterText);
  }
  display.show();
}
