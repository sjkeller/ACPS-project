# What is MQTT?

- **MQTT** (Message Queue Telemetry Transport) is a **TCP/IP-based** messaging transport protocol that enables the exchange of **messages** between devices (**clients**) and a server (**broker**). MQTT is particularly suitable and widely used  in industrial, automation, and IoT applications.

- MQTT follows a **Publish/Subscribe** pattern, i.e.  in contrast to a client-sever model, where the client communicates directly with an endpoint, in a pub/sub model communication between different clients is decoupled. Each message sent by the client (the publisher)  must be assigned to a **topic**. Other clients can receive the message by **subscribing** to this topic. All connections are handled by the broker, so that publishers and subscribers never communicate directly. The job of the broker is to filter all incoming messages and distribute them correctly to the subscribers. 

### Further Reading

- [Blog Post Series by HiveMQ "MQTT Essentials"](https://www.hivemq.com/tags/mqtt-essentials/)

- https://mqtt.org/

  

## Setup MQTT Broker (Mosquitto)

For your interest  the instructions of how we have set up the MQTT broker on the Raspbbery Pi are provided in the [MQTT-Broker-Mosquitto.md](./MQTT-Broker-Mosquitto.md). 



## Setup MQTT Client (Python)

### Get Python (Windows)

If you have not used Python before:

- [Download and install  Python](https://www.python.org/downloads/)
- [Install pip packet manager for Python](https://pip.pypa.io/en/stable/installation/)

### Paho MQTT Client

To install [Paho MQTT](https://pypi.org/project/paho-mqtt/),  a MQTT client library for Python, run in terminal:

    pip install paho-mqtt

The [Usage and API](https://pypi.org/project/paho-mqtt/#usage-and-api) provides all information needed to setup an MQTT Subscriber/Publisher. Here is the original code example for a basic MQTT subscriber implementation:

~~~python
import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("$SYS/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("mqtt.eclipseprojects.io", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
~~~

