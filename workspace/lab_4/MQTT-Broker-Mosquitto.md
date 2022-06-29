# MQTT Broker (Mosquitto)

This document describes the setup of a Mosquitto MQTT Broker on a **Raspberry Pi.**
You can also find setup instructions for other Linux/Windows Platforms here:  https://mosquitto.org/download/

## Setup 
### Installation

    sudo apt update && sudo apt upgrade
    sudo apt install -y mosquitto
    sudo apt install -y mosquitto-clients

### Test

Subscribe (Terminal 1):

    mosquitto_sub -h localhost -t /test/topic

Publish (Terminal 2):

    mosquitto_pub -h localhost -t /test/topic -m "Hello from the 2nd terminal!"

### Systemd-Service

    sudo systemctl start mosquitto    # Starten
    sudo systemctl stop mosquitto     # Stoppen
    sudo systemctl restart mosquitto  # Neustarten
    sudo systemctl disable mosquitto  # Autostart deaktivieren
    sudo systemctl enable mosquitto   # Autostart aktivieren


## Further Configuration
- Main config file is located in: `/etc/mosquitto/mosquitto.conf`
- In addition, all files ending with `*.conf` located in `/etc/mosquitto/conf.d/` are automatically included

### Enable autostart on boot
To make Mosquitto auto start when the raspberry boots:

    sudo systemctl enable mosquitto.service

### Enable remote access
By default Mosquitto starts in local only mode. Connections will only be possible from clients running on this machine. To allow remote access, create a configuration file which defines the listener port.

Edit config file:

    sudo nano /etc/mosquitto/mosquitto.conf

and add following lines:

    listener 1883
    allow_anonymous true


Reboot the raspberry or restart service for the changes to take effect

    sudo systemctl restart mosquitto.service

Retrieve raspberry's IP adress:

    hostname -I

## References

- [Raspberry Pi MQTT configuration tutorial (in german)](https://plantprogrammer.de/mqtt-auf-dem-raspberry-pi-mosquitto/)
- https://github.com/roppert/mosquitto-python-example
- https://pypi.org/project/paho-mqtt/
- [configure SSL/TLS support for Mosquitto](https://mosquitto.org/man/mosquitto-tls-7.html)