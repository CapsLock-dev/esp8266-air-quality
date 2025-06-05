from flask import Flask, render_template, jsonify, request
from waitress import serve
from sensor_data_handler import SensorDataHandler
import datetime

class WebPanelServer:
    def __init__(self, port=8989):
        self.app = Flask(__name__)
        self.port = port
        self.setup_routes()

    def setup_routes(self):
        @self.app.route("/")
        def dashboard():
            return render_template("dashboard.html")

        @self.app.route("/api/sensor-data")
        def sensor_data_api():
            hours = request.args.get("hours")
            print(hours)
            if hours:
                hours = int(hours)
            else: 
                hours = 24
            end_time = datetime.datetime.now()
            start_time = end_time - datetime.timedelta(hours=hours)
            return jsonify(self.sensor_data_handler.get_data_by_time(start_time, end_time))

    
    def start(self):
        self.sensor_data_handler = SensorDataHandler() 
        serve(self.app, host="0.0.0.0", port=self.port)
