import serial
from datetime import datetime as dt

usb_device : str = "/dev/ttyUSB0"

monitor = serial.Serial(usb_device)

current_time = dt.today()
time_str = current_time.strftime("%Y-%m-%d_%H-%M-%S")
monitor.write(time_str)

print(time_str)
serial.close()