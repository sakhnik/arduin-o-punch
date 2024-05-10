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
    <form method='POST' enctype='multipart/form-data' id='upload-form'>
      <input type='file' id='file' name='update'>
      <input type='submit' value='Update'>
    </form>
    <br>
    <div id='prg' style='width:0;color:white;text-align:center'>0%</div>
  </div>
  <script>
    var ledState = false;
    var button = document.getElementById('button');
    function toggleLED() {
      ledState = !ledState;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/led?state=' + (ledState ? '1' : '0'), true);
      xhr.send();
      button.innerHTML = 'LED: ' + (ledState ? 'ON' : 'OFF');
      button.classList.toggle('on', ledState); // Add or remove the 'on' class based on the LED state
    }

    var prg = document.getElementById('prg');
    var uploadForm = document.getElementById('upload-form');
    uploadForm.addEventListener('submit', el=>{
      prg.style.backgroundColor = 'blue';
      el.preventDefault();
      var data = new FormData(uploadForm);
      var req = new XMLHttpRequest();
      var fsize = document.getElementById('file').files[0].size;
      req.open('POST', '/update?size=' + fsize);
      req.upload.addEventListener('progress', p=>{
        let w = Math.round(p.loaded/p.total*100) + '%';
          if(p.lengthComputable){
             prg.innerHTML = w;
             prg.style.width = w;
          }
          if(w == '100%') prg.style.backgroundColor = 'black';
      });
      req.send(data);
     });
  </script>
</body>
</html>
)html";

#endif //ESP32
