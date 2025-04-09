from flask import Flask, jsonify
from flask_cors import CORS
from digi.xbee.devices import XBeeDevice
from digi.xbee.models.address import XBee64BitAddress
import sqlite3, struct
from datetime import datetime
from mappings import USER_NAME_MAP, PILL_TYPE_MAP

#print("Libraries are installed correctly!")

app = Flask(__name__)
CORS(app)

PORT = "/dev/tty.usbserial-AL02BZKQ"
BAUD_RATE = 9600;
REMOTE_ADDRESS = "13A200424019CE"

device = XBeeDevice(PORT, BAUD_RATE)
device.open()

remote_address = XBee64BitAddress.from_hex_string(REMOTE_ADDRESS)
remote_device = device.get_network().get_device_by_64(remote_address)
receive_buffer = bytearray()

DB_FILE = "health_db.db"

def save_event(name_id, heart_rate, temperature, pill_id):
    user_id = int(round(name_id))
    name = USER_NAME_MAP.get(user_id, f"Unknown ({user_id})")
    pill_dispensed = PILL_TYPE_MAP.get(int(pill_id), f"Unknown ({pill_id})")
    timestamp = datetime.now().strftime("%Y-%m-%d %I:%M %p")

    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()

    #Ensure this user exist
    c.execute("INSERT OR IGNORE INTO users (user_id, name) VALUES (?, ?)", (user_id, name))

    #INSERT new event
    c.execute('''
        INSERT INTO events (user_id, heart_rate, temperature, pill_dispensed, timestamp)
        VALUES (?, ?, ?, ?, ?)
    ''', (user_id, heart_rate, temperature, int(pill_id), timestamp))

    conn.commit()
    conn.close()

# Handle incoming data
def handle_data(xbee_message):
    global receive_buffer
    try:
        raw = xbee_message.data
        print(f"Received raw data: {raw}")
        if raw.startswith(b'Test from STM32'):
            raw = raw[len(b'Test from STM32\r\n'):]

        receive_buffer += raw
        chunk = 0
        while len(receive_buffer) >= 16:
            chunk = receive_buffer[:16]
            receive_buffer = receive_buffer[16:]
        try:
            if chunk:
                name_id, heart_rate, temperature, pill_id = struct.unpack('ffff', chunk)
                print(f"Parsed: id={name_id}, HR={heart_rate}, Temp={temperature}, Pill={pill_id}")
                save_event(name_id, heart_rate, temperature, pill_id)
        except struct.error as err:
            print(f"Struct unpack error: {err}")
    except Exception as e:    
        print(f"Error parsing binary data: {e}")
device.add_data_received_callback(handle_data)

# Endpoint to get all saved data
@app.route('/receive', methods=['GET'])
def get_all_data():
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()
    c.execute('''
        SELECT users.name, events.heart_rate, events.temperature, events.pill_dispensed, events.timestamp
        FROM events 
        JOIN users ON events.user_id = users.user_id     
        ORDER BY events.timestamp DESC
    ''')
    rows = c.fetchall()
    conn.close()

    events = [{
        'name': row[0],
        'heart_rate': row[1],
        'temperature': row[2],
        'pill_dispensed': PILL_TYPE_MAP.get(row[3], f"Unknown ({row[3]})"),
        'timestamp': row[4]
    } for row in rows]

    return jsonify({'events': events})

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=3000)