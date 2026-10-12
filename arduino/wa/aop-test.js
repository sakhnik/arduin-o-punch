
// Function to create the HTML table
function createMifareLayout() {
    const numRows = 64; // Total number of blocks in a Mifare Classic 1k card
    const numCols = 16; // Number of bytes in each block

    const table = document.createElement('table');
    table.classList.add('mifare');

    // Loop through each block and add rows to the table
    for (let i = 0; i < numRows; i++) {
        const row = table.insertRow();

        const headerCell = row.insertCell();
        headerCell.textContent = `[${i.toString().padStart(2, '0')}]`;

        if (i == 0) {
            const cell = row.insertCell();
            cell.colSpan = numCols;
            cell.textContent = 'Manufaturer Data';
            cell.classList.add('manufacturerData');
            continue;
        }

        const isControlBlock = i % 4 === 3;
        for (let j = 0; j < numCols; j++) {
            const cell = row.insertCell();
            cell.setAttribute('id', `cell_${i}_${j}`)
            cell.textContent = '00';
            if (isControlBlock) {
                if (j < 6) {
                    cell.classList.add('keyA');
                } else if (j < 9) {
                    cell.classList.add('accessBits');
                } else if (j >= 10) {
                    cell.classList.add('keyB');
                }
            }
        }
    }

    document.getElementById('table-container').appendChild(table);
}

var mifare;
var punchCard;

function initializeAopTest() {
    createMifareLayout();
    mifare = new Module.WebMifare();

    document.getElementById('prepareButton').disabled = false;
    document.getElementById('prepareButton').addEventListener('click', onPrepareButton);
    document.getElementById('punchButton').addEventListener('click', onPunchButton);
    document.getElementById('readoutButton').addEventListener('click', onReadoutButton);
}

function onPrepareButton() {
    var cardId = parseInt(document.getElementById('cardId').value);
    var hexKey = document.getElementById('hexKey').value;

    // Check if the number is valid
    if (isNaN(cardId) || cardId < 0 || cardId >= 65536) {
        alert('Please enter a valid cardId in the range 0-65535.');
        return;
    }

    if (!/^[0-9A-Fa-f]{12}$/.test(hexKey)) {
        alert('Please enter a valid hex key of 6 bytes (12 characters).');
        return;
    }

    if (typeof punchCard !== 'undefined' && punchCard !== null) {
        punchCard.delete();
    }
    if (mifare == null) {
        mifare = new Module.WebMifare();
    }
    const key = new Uint8Array(hexKey.match(/.{1,2}/g).map(byte => parseInt(byte, 16)));
    punchCard = new Module.PunchCard(mifare, key);
    const res = punchCard.Format(cardId);
    if (res != 0) {
        alert("Failed to format");
        return;
    }
}

function onPunchButton() {
    let stationEntry = document.getElementById('stationId');
    var stationId = parseInt(stationEntry.value);

    if (isNaN(stationId) || stationId < 0 || stationId >= 256) {
        alert('Please enter a valid stationId in the range 0-255.');
        return;
    }

    var now = new Date();
    var secondsSinceMidnight = now.getHours() * 3600 + now.getMinutes() * 60 + now.getSeconds();

    const res = punchCard.Punch(stationId, secondsSinceMidnight);
    if (res != 0) {
        alert("Failed to punch");
        return;
    }
    stationEntry.value = stationId + 1;
}

function onReadoutButton() {
    const res = punchCard.ReadOut();
    if (res != 0) {
        alert("Failed to readout");
        return;
    }
    const punches = punchCard.GetLastReadout();
    var report = [];
    for (var i = 0; i < punches.size(); i++) {
        const punch = punches.get(i);
        report.push(`${i+1}. ${punch.stationId} ${secondsToHMS(punch.timestamp)}`);
    }
    var popup = document.getElementById('popup-content');
    popup.innerHTML = '<pre>' + report.join("\n") + '</pre>';
    showPopup();
}

function secondsToHMS(totalSeconds) {
    var hours = Math.floor(totalSeconds / 3600);
    var minutes = Math.floor((totalSeconds % 3600) / 60);
    var seconds = totalSeconds % 60;

    hours = String(hours).padStart(2, '0');
    minutes = String(minutes).padStart(2, '0');
    seconds = String(seconds).padStart(2, '0');

    return hours + ":" + minutes + ":" + seconds;
}

function showPopup() {
    var popup = document.getElementById('popup');
    popup.style.display = 'block';
}

function hidePopup() {
    var popup = document.getElementById('popup');
    popup.style.display = 'none';
}
