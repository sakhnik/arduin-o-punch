#ifdef ESP32

#include <Arduino.h>

const char *index_html PROGMEM = R"html(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <title>Станція Arduin-O-Punch</title>
  <style>
    body {
      font-family: sans-serif;
    }
    .container {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Станція Arduin-O-Punch</h1>
    <br>
    <h3>Налаштування</h3>
    <button id="button" onclick="toggleLED()">LED: OFF</button>
    <br>
    <br>
    <h3>Поновлення</h3>
    <input type="range" id="trackbar" min="0" max="255" step="1" value="0" oninput="setBrightness()">
  </div>
  <script>
    var ledState = false;
    var ledBrightness = 0;
    var button = document.getElementById('button');
    var trackbar = document.getElementById('trackbar');
    function toggleLED() {
      ledState = !ledState;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/led?state=' + (ledState ? '1' : '0'), true);
      xhr.send();
      button.innerHTML = 'LED: ' + (ledState ? 'ON' : 'OFF');
      button.classList.toggle('on', ledState); // Add or remove the 'on' class based on the LED state
    }
    function setBrightness() {
      ledBrightness = trackbar.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/brightness?value=' + ledBrightness, true);
      xhr.send();
    }
  </script>
</body>
</html>
)html";

#endif //ESP32
