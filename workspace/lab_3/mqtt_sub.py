import paho.mqtt.client as mq
import time

def on_connect(client, userdata, flags, rc):
	print("connected with result code", str(rc))
	client.subscribe("acps/receive_from/lora_8")
	
def on_message(client, userdata, msg):
	print(msg.topic, str(msg.payload))
	
client = mq.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect_async("134.28.36.100", 1883, 60)
client.loop_start()

while True:
	user_input = input("enter a message pls ")
	client.publish("acps/send_to/lora_10", user_input)
	print("published message", user_input)

