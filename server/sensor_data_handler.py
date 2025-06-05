import sqlite3
import datetime
import json

TEMP_PRECISION = 2
TEMP_CORRECTION = 5
HUM_PRECISION = 2

class InvalidDataError(Exception): 
    def __init__(self, missing_fields):            
        self.missing_fields = missing_fields 

class SensorDataHandler:
    def __init__(self):
        self.db_path = "db/sensors_data.db" 
        self.db_conn = sqlite3.connect(self.db_path, check_same_thread=False)

    def init_db(self):
        self.db_conn.execute("PRAGMA journal_mode=WAL;")
        self.db_conn.execute("PRAGMA busy_timeout=5000;")
        self.db_conn.execute("CREATE TABLE IF NOT EXISTS air_data(time timestamp, temp real, hum real, aqi integer, tvoc integer, eco2 integer);")

    def _parse_message(self, msg): 
        payload = msg.payload.decode()
        data = json.loads(payload)

        missing_fields = []
        if "temp" not in data:
            missing_fields.append("temp")
        if "hum" not in data: 
            missing_fields.append("hum")
        if"aqi" not in data:
            missing_fields.append("aqi")
        if "tvoc" not in data: 
            missing_fields.append("tvoc")
        if "eco2" not in data:
            missing_fields.append("eco2")
        
        if missing_fields:
            raise InvalidDataError(missing_fields)
  
        data["temp"] = round(data["temp"] / (10 ** TEMP_PRECISION) - TEMP_CORRECTION, TEMP_PRECISION)
        data["hum"] = round(data["hum"] / (10 ** HUM_PRECISION), HUM_PRECISION)
  
        return data

    def write_data(self, msg):
        try:
            data = self._parse_message(msg)
            print(f"writing {data} to db")
            cur = self.db_conn.cursor()
            query = "INSERT INTO air_data VALUES (?, ?, ?, ?, ?, ?);"
            currentDateTime = datetime.datetime.now()
            cur.execute(query, (currentDateTime, data["temp"], data["hum"], data["aqi"], data["tvoc"], data["eco2"])) 
            self.db_conn.commit()
            cur.close()

        except InvalidDataError as e:
            print(f"Message is missing fields: {e.missing_fields}")
        except json.JSONDecodeError as e:
            print(f"Parsing error: {e}")
        except sqlite3.Error as e:
            print(f"Database error: {e}")
        except Exception as e:
            print(f"An error occurred while writing data: {e}")

    def get_data_by_time(self, start, end):
        try:
            cur = self.db_conn.cursor()
            cur.execute("""
                SELECT 
                    strftime('%m.%d %H:%M', time) as time,
                    temp, hum, aqi, tvoc, eco2
                FROM air_data
                WHERE time BETWEEN ? AND ?
                ORDER BY time
            """, (start, end))
            columns = [col[0] for col in cur.description]
            return [dict(zip(columns, row)) for row in cur.fetchall()]

        except sqlite3.Error as e:
            print(f"Database error: {e}")
        except Exception as e:
            print(f"An error occurred while getting data: {e}")
        return []
