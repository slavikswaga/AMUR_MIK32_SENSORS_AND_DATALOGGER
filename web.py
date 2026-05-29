from flask import Flask, jsonify, render_template_string
import serial
import time
from datetime import datetime
from collections import deque
from threading import Thread

app = Flask(__name__)

# Храним последние 100 значения
last_values = deque(maxlen=100)

# Настройка Arduino
arduino = serial.Serial('COM10', 9600, timeout=1)
time.sleep(2)

arduino.reset_input_buffer()
arduino.reset_output_buffer()


def serial_reader():
    while True:
        try:
            # отправляем текущее время
            now = datetime.now().strftime('%d.%m.%Y %H:%M:%S')
            arduino.write((now + '\n').encode())

            time.sleep(0.05)

            while arduino.in_waiting:
                response = arduino.readline().decode(errors="ignore").strip()

                if response:
                    print(response)
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


@app.route("/")
def index():
    return render_template_string("""
<!DOCTYPE html>
<html>
<head>
    <title>ELBEAR ACE UNO Monitor</title>
</head>
<body>

<h2>Последние 100 значений</h2>

<table border="1">
<thead>
<tr>
<th>Время</th>
<th>Данные</th>
</tr>
</thead>

<tbody id="table"></tbody>

</table>

<script>

async function updateData() {

    const response = await fetch('/data');
    const data = await response.json();

    let html = "";

    data.slice().reverse().forEach(item => {
        html += `
        <tr>
            <td>${item.time}</td>
            <td>${item.value}</td>
        </tr>
        `;
    });

    document.getElementById("table").innerHTML = html;
}

updateData();

setInterval(updateData, 1000);

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