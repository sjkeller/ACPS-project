import paho.mqtt.client as mqtt

broker = "134.28.36.100"
port = 1883
topic_receive = "acps/receive_from/lora_8"
topic_publish = "acps/send_to/lora_8"
"""
topic_receive = "acps/group_2"
topic_publish = "acps/group_2"
"""

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(topic_receive)

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

timeout = 60
client.connect_async(broker, port, timeout)
client.loop_start()

while True:
    data = input()
    ret = client.publish(topic_publish, data)