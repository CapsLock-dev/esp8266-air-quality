from mqtt_client import MQTTClient
from web_panel import WebPanelServer
from sensor_data_handler import SensorDataHandler
import threading
import os

if __name__ == '__main__':
    mqtt_broker_ip = os.getenv("MQTT_BROKER_IP") 
    mqtt_broker_port = os.getenv("MQTT_BROKER_PORT")
    http_server_port = os.getenv("HTTP_SERVER_PORT")

    if mqtt_broker_ip == None:
        print("MQTT_BROKER_IP isn't set. Exiting")
        exit(1)

    if mqtt_broker_port == None:
        mqtt_broker_port = 1883 
        print(f"MQTT_BROKER_PORT isn't set. Using default {mqtt_broker_port}")
    else:
        mqtt_broker_port = int(mqtt_broker_port)

    if http_server_port == None:
        http_server_port = 8989
        print(f"MQTT_BROKER_PORT isn't set. Using default {http_server_port}")
    else:
        http_server_port = int(http_server_port)

    SensorDataHandler().init_db()
    mqtt_client = MQTTClient(mqtt_broker_ip, mqtt_broker_port)
    mqtt_thread = threading.Thread(target=mqtt_client.start)
    mqtt_thread.start()
    WebPanelServer(http_server_port).start()

#//TODO: secure connection (SSL, HTTPS, authorized clients mqtt)
