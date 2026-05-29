import serial
import time
from datetime import datetime

arduino = serial.Serial('COM10', 9600, timeout=1)
time.sleep(2)
arduino.reset_input_buffer()
arduino.reset_output_buffer()

while True:
    # Отправляем время
    now = datetime.now().strftime('%d.%m.%Y %H:%M:%S')  
    arduino.write((now + '\n').encode())
    time.sleep(0.05)
    # Читаем ответ
    while arduino.in_waiting:
        response = arduino.readline().decode().strip()
        if response:
            print(response)
    time.sleep(0.95)