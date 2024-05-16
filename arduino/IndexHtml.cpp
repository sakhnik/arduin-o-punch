#ifdef ESP32

#include <Arduino.h>

const char *index_html PROGMEM = R"html(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width">
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
    <p>Версія <span>)html" PROJECT_VERSION "-" GIT_REVISION R"html(</span></p>
    <h3>Налаштування</h3>
    <form method='POST' action='/settings' id='settings-form' target='_self'>
      <table>
        <tr>
          <td><label for='id'>Номер:</label></td>
          <td><input type='number' id='id' name='id' min='1' max='255'/></td>
        </tr>
        <tr>
          <td><label for='key'>Ключ:</label></td>
          <td><input type='password' id='key' name='key'/>
            <span class='toggle-key' onclick='toggleKeyVisibility()'>👁️</span></td>
        </tr>
        <tr>
          <td><label for='record-size'>Записів у журналі:</label></td>
          <td><input type='number' id='record-size' name='rec-size'/></td>
        </tr>
        <tr>
          <td><label for='record-bits'>Відміток на запис:</label></td>
          <td>
            <div>
              <input type='radio' name='rec-bits' id='record-bits-1' value='1'/>
              <label for='record-bits-1'>0–1</label>
            </div>
            <div>
              <input type='radio' name='rec-bits' id='record-bits-2' value='2'/>
              <label for='record-bits-2'>0–3</label>
            </div>
            <div>
              <input type='radio' name='rec-bits' id='record-bits-4' value='4'/>
              <label for='record-bits-2'>0–15</label>
            </div>
            <div>
              <input type='radio' name='rec-bits' id='record-bits-8' value='8'/>
              <label for='record-bits-8'>0–255</label>
            </div>
          </td>
        </tr>
        <tr>
          <td><label for='record-days'>Період журналу (днів):</label></td>
          <td><input type='number' id='record-days' name='rec-days'/></td>
        </tr>
        <tr>
          <td></td>
          <td><input type='submit' value='Застосувати'></td>
        </tr>
      </table>
    </form>
    <h3>Оновлення</h3>
    <form method='POST' enctype='multipart/form-data' id='upload-form'>
      <input type='file' id='file' name='update' accept='.esp32.bin'/>
      <input type='submit' value='Завантажити'/>
    </form>
    <br/>
    <div id='prg' style='width:0;color:white;text-align:center'>0%</div>
  </div>
  <script>
    function toggleKeyVisibility() {
        var keyInput = document.getElementById("key");
        var toggleIcon = document.querySelector(".toggle-key");

        if (keyInput.type === "password") {
            keyInput.type = "text";
            toggleIcon.innerHTML = "••";
        } else {
            keyInput.type = "password";
            toggleIcon.innerHTML = "👁️";
        }
    }

    document.addEventListener("DOMContentLoaded", function() {
        setupSettingsFormSubmit();
        reloadSettings();
    });

    function setupSettingsFormSubmit() {
        var form = document.getElementById("settings-form");
        form.addEventListener("submit", function(event) {
            event.preventDefault();
            fetch(form.action, {
                method: form.method,
                body: new FormData(form)
            })
            .then(response => {
                if (response.ok) {
                    window.location.reload();
                } else {
                    throw new Error('Form submission failed');
                }
            })
            .catch(error => {
                console.error('Error:', error);
            });
        });
    }

    function reloadSettings() {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/settings', true);
        xhr.onload = function() {
            if (xhr.status >= 200 && xhr.status < 300) {
                var response = xhr.responseText;
                updateSettings(response);
            } else {
                // Request failed
                console.error('Request failed with status:', xhr.status);
            }
        };
        xhr.onerror = function() {
            console.error('Request failed');
        };
        xhr.send();
    }

    function updateSettings(data) {
        var keyValuePairs = data.split('\n');
        keyValuePairs.forEach(function(pair) {
            var keyValue = pair.split('=');
            if (keyValue.length === 2) {
                if (keyValue[0] === 'id') {
                    document.getElementById('id').value = keyValue[1];
                } else if (keyValue[0] === 'key') {
                    document.getElementById('key').value = keyValue[1];
                } else if (keyValue[0] === 'rec-size') {
                    document.getElementById('record-size').value = keyValue[1];
                } else if (keyValue[0] === 'rec-bits') {
                    document.getElementById('record-bits-' + keyValue[1]).checked = true;
                } else if (keyValue[0] === 'rec-days') {
                    document.getElementById('record-days').value = keyValue[1];
                }
            }
        });
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
        req.upload.addEventListener('progress', p => {
            let w = Math.round(p.loaded/p.total*100) + '%';
            if (p.lengthComputable) {
                prg.innerHTML = w;
                prg.style.width = w;
            }
            if (w == '100%') prg.style.backgroundColor = 'black';
        });
        req.send(data);
     });
  </script>
</body>
</html>
)html";

#endif //ESP32
