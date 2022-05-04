import serial
from datetime import datetime as dt

usb_device : str = "/dev/tty.usbmodem11303"

monitor = serial.Serial(usb_device)

current_time = dt.today()
time_str = current_time.strftime("%Y-%m-%d_%H-%M-%S").encode()
monitor.write(time_str)

print(time_str)
monitor.close()