import serial
import sys
from datetime import datetime as dt


if sys.platform == "win32":
    usb_device_win : str = "COM8"
    monitor = serial.Serial(usb_device_win)
else:
    usb_device_un : str = "/dev/tty.usbmodem1103"
    monitor = serial.Serial(usb_device_un)

current_time = dt.today()
time_str = current_time.strftime("%Y-%m-%d_%H-%M-%Sx").encode("utf-8")
monitor.write(time_str)

print(time_str)
monitor.close()