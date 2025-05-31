import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe("/sensors/environment", qos=1)  # Listen to all devices

def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}, Payload: {msg.payload.decode()}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)  # Replace with broker IP
client.loop_forever()
