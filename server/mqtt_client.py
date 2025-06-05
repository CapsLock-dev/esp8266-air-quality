from sensor_data_handler import SensorDataHandler
import paho.mqtt.client as mqtt

class MQTTClient:
    def __init__(self, server_ip, port=1883):
        self.client = mqtt.Client()
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.server_ip = server_ip
        self.port = port

    def start(self):
        self.sensor_data_handler = SensorDataHandler() 
        self.client.connect(self.server_ip, self.port, 60)
        self.client.loop_forever()

    def _on_connect(self, client, userdata, flags, rc):
        print("Connected to MQTT broker")
        client.subscribe("/sensors/environment", qos=1)

    def _on_message(self, client, userdata, msg):
        print(f"Got data {msg}")
        self.sensor_data_handler.write_data(msg)
