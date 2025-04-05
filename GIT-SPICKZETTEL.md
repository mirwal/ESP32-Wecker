
# 🚀 Git Spickzettel – ESP32-Wecker

## 🔥 Projekt anlegen (hast du schon gemacht)

git init                             # Git im Projektordner starten
git remote add origin  URL           # Verbindung zu GitHub herstellen
git add .                            # Alle Dateien zur Versionskontrolle hinzufügen
git commit -m "Erste Version"        # Lokalen Speicherpunkt setzen
git push -u origin master            # Alles nach GitHub hochladen

## 🟢 Wenn du später Änderungen machst

1. Änderungen speichern:
git add .
git commit -m "Kurze Beschreibung deiner Änderung"

2. Hochladen zu GitHub:
git push

## 🔄 Neuen Stand von GitHub holen (z. B. an anderem Gerät)

git pull

## 📄 Aktuellen Status anzeigen

git status

## ❗️ Letzten Commit zurücknehmen (nur wenn nötig)

git reset --soft HEAD~1

## ⭐️ Neues Release erstellen (optional)

Wenn du z. B. Version 2.0 fertig hast:
git tag v2.0
git push origin v2.0
