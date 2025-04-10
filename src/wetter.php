<?php
$apiKey = "760e9eed958010cc360cc4eb35027f7b";  // <-- deinen Key hier eintragen
$city = "Esterwegen";
$units = "metric";  // oder "imperial"
$lang = "de";

$url = "https://api.openweathermap.org/data/2.5/weather?q=$city&appid=$apiKey&units=$units&lang=$lang";
$response = file_get_contents($url);
$data = json_decode($response, true);


$response = file_get_contents($url);
$data = json_decode($response, true);

function wetterSymbol($id)
{
    // OpenWeatherMap Wetter-ID → Symbol-Zuordnung
    if ($id >= 200 && $id < 300) return "🌩️";   // Gewitter
    if ($id >= 300 && $id < 500) return "🌦️";   // Sprühregen
    if ($id >= 500 && $id < 600) return "🌧️";   // Regen
    if ($id >= 600 && $id < 700) return "❄️";   // Schnee
    if ($id >= 700 && $id < 800) return "🌫️";   // Nebel
    if ($id == 800)               return "☀️";   // Klar
    if ($id == 801)               return "🌤️";   // leicht bewölkt
    if ($id >= 802 && $id <= 804) return "☁️";   // bewölkt
    return "🌈"; // Fallback
}


if ($data && isset($data['weather'][0]['description'])) {
    $id = $data['weather'][0]['id'];
    $symbol = wetterSymbol($id);
    $desc = ucfirst($data['weather'][0]['description']);
    $temp = round($data['main']['temp']);
    echo "$city: $desc $temp C";
} else {
    echo "Wetterdaten nicht verfügbar";
}
