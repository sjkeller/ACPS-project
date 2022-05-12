import serial
from datetime import datetime as dt

usb_device : str = "/dev/tty.usbmodem1103"

monitor = serial.Serial(usb_device)

current_time = dt.today()
time_str = current_time.strftime("%Y-%m-%d_%H-%M-%Sx").encode("utf-8")
monitor.write(time_str)

print(time_str)
monitor.close()