# ESP32 Wecker v2.0

🚀 ToDo / Ideen für Version 2.0
🌐 Netzwerk & Setup

    WLAN-Manager einbauen → z. B. mit AutoConnect oder eigener Setup-Seite, um WLAN-Daten bequem einzurichten (statt hardcoded).

    Hostname im Netzwerk sichtbar machen → per WiFi.setHostname("esp-wecker")

🔔 Wecker-Funktion

    Mehrere Weckzeiten speichern → z. B. 3–5 Profile

    Wochentagsabhängige Weckzeiten → nur Mo–Fr, Wochenende aus

    Schlummer-Funktion (Snooze) → per Button kurz pausieren

    Lautstärke-Regelung → über Webinterface oder Button

🌙 Energiesparen

    Display nachts abschalten (z. B. 22–6 Uhr)

    ESP32 in Light Sleep versetzen, wenn kein Alarm aktiv ist

🔥 Optische Erweiterungen

    LED-Statusanzeige → leuchtet, wenn Wecker aktiv ist

    Hintergrundbeleuchtung dimmbar
    (mit PWM über MOSFET, wenn dein LCD das unterstützt)

📄 Software-Aufräumideen

    DisplayHelper um Auto-Scroll & Blinken erweitern

    Webinterface optisch aufhübschen → z. B. mit CSS oder Icons

    API-Token einbauen → damit nicht jeder im Netzwerk deinen Wecker steuern kann

    OTA-Update integrieren → Firmware über das Webinterface aktualisieren

🎶 Musik & Sound

    Verschiedene Weck-Sounds wählbar → per Webinterface

    Lautstärke-Anstieg (Fade-In) → sanftes Wecken

    Zufällige Playlist / Song-Auswahl

☀️ „Nice to have“-Features

    Sonnenaufgang-Simulation → LED langsam heller werden lassen

    Temperatur & Wetter anzeigen → optional mit einem DHT22 oder per Web-API
