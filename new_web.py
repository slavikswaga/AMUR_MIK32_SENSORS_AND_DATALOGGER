from flask import Flask, jsonify, render_template_string
import serial
import time
from datetime import datetime
from collections import deque
from threading import Thread

app = Flask(__name__)

# Храним последние 100 значений
last_values = deque(maxlen=100)

# Словарь для хранения последних значений датчиков
latest_sensors = {
    "0BH1750": {"name": "MGS_L75 (BH1750)", "value": "---", "unit": "lx"},
    "MPU6050": {"name": "MGS_A6 (MPU6050)", "value": "---", "unit": "°C"},
    "VL53L0X": {"name": "MGS_D20 (VL53L0X)", "value": "---", "unit": "mm"},
    "INMP504": {"name": "MGS_SND504 (INMP504)", "value": "---", "unit": "dB"},
    "00LM75A": {"name": "LM75A", "value": "---", "unit": "°C"},
    "0BME280": {"name": "MGS_THP80 (BME280)", "value": "---", "unit": "%"},
    "0000WT1": {"name": "MGS_WT1", "value": "---", "unit": "%"},
    "TSL2540": {"name": "MGS_FR403 (TSL2540)", "value": "---", "unit": "UV"}
}

# Настройка Arduino
arduino = serial.Serial('COM10', 9600, timeout=1)
time.sleep(2)

arduino.reset_input_buffer()
arduino.reset_output_buffer()

def parse_and_update(line):
    """Парсит строку и обновляет значения датчиков"""
    parts = line.strip().split()
    if len(parts) >= 6:
        try:
            sensor_name = parts[4]
            sensor_value = parts[5]
            
            if sensor_name in latest_sensors:
                latest_sensors[sensor_name]["value"] = sensor_value
        except:
            pass

def serial_reader():
    while True:
        try:
            now = datetime.now().strftime('%d.%m.%Y %H:%M:%S')
            arduino.write((now + '\n').encode())

            time.sleep(0.05)

            while arduino.in_waiting:
                response = arduino.readline().decode(errors="ignore").strip()
                if response:
                    print(response)
                    # Обновляем значения датчиков
                    parse_and_update(response)
                    # Сохраняем в таблицу
                    last_values.append({
                        "time": now,
                        "value": response
                    })

            time.sleep(0.95)

        except Exception as e:
            print("Serial error:", e)
            time.sleep(1)

@app.route("/data")
def data():
    return jsonify(list(last_values))

@app.route("/sensors")
def sensors():
    return jsonify(latest_sensors)

@app.route("/")
def index():
    return render_template_string("""
<!DOCTYPE html>
<html>
<head>
    <title>ELBEAR ACE UNO Monitor</title>
    <style>
        .main-container {
            display: flex;
            gap: 20px;
        }
        .table-container {
            flex: 1;
        }
        .squares-container {
            width: 200px;
        }
        .sensor-square {
            border: 1px solid #999;
            padding: 5px;
            margin-bottom: 5px;
            text-align: center;
            font-size: 12px;
        }
        .sensor-name {
            font-weight: bold;
        }
        .sensor-value {
            font-size: 14px;
        }
        h3 {
            margin: 0 0 10px 0;
        }
    </style>
</head>
<body>

<h2>Последние 100 значений</h2>

<div class="main-container">
    <div class="table-container">
        <table border="1">
            <thead>
                <tr>
                    <th>Данные</th>
                </tr>
            </thead>
            <tbody id="table"></tbody>
        </table>
    </div>
    
    <div class="squares-container">
        <h3>Текущие значения</h3>
        <div id="squares">
            <!-- Squares will appear here -->
        </div>
    </div>
</div>

<script>

async function updateData() {
    const response = await fetch('/data');
    const data = await response.json();

    let html = "";
    data.slice().reverse().forEach(item => {
        html += `
            <tr>
                <td>${item.value}</td>
            </tr>
        `;
    });
    document.getElementById("table").innerHTML = html;
}

async function updateSquares() {
    const response = await fetch('/sensors');
    const sensors = await response.json();
    
    let squaresHtml = "";
    
    for (const [key, sensor] of Object.entries(sensors)) {
        squaresHtml += `
            <div class="sensor-square">
                <div class="sensor-name">${sensor.name}</div>
                <div class="sensor-value">${sensor.value} ${sensor.unit}</div>
            </div>
        `;
    }
    
    document.getElementById("squares").innerHTML = squaresHtml;
}

updateData();
updateSquares();

setInterval(() => {
    updateData();
    updateSquares();
}, 1000);

</script>

</body>
</html>
""")


if __name__ == "__main__":
    thread = Thread(target=serial_reader, daemon=True)
    thread.start()
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )