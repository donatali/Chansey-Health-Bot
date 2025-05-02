from flask import Flask, jsonify, request
from flask_cors import CORS
# XBee imports (uncomment when using XBee hardware)
from digi.xbee.devices import XBeeDevice
from digi.xbee.models.address import XBee64BitAddress
import sqlite3, struct
from datetime import datetime, timedelta
from mappings import USER_NAME_MAP, PILL_TYPE_MAP
import json
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    filename='server.log'
)
logger = logging.getLogger(__name__)

#print("Libraries are installed correctly!")

app = Flask(__name__)
CORS(app)

# Flag to track if we're running in simulation mode (without XBee)
# Set to False when using XBee hardware
SIMULATION_MODE = False  # Set to False for using XBee hardware

# XBee configuration
PORT = "/dev/tty.usbserial-AL02BZKQ"
BAUD_RATE = 9600
REMOTE_ADDRESS = "13A200424019CE"

# Only initialize XBee device if not in simulation mode
device = None
remote_device = None
receive_buffer = bytearray()
last_message_time = {}

if not SIMULATION_MODE:
    try:
        device = XBeeDevice(PORT, BAUD_RATE)
        device.open()
        remote_address = XBee64BitAddress.from_hex_string(REMOTE_ADDRESS)
        remote_device = device.get_network().get_device_by_64(remote_address)
        print("XBee device initialized successfully")
    except Exception as e:
        print(f"Error initializing XBee: {e}")
        print("Falling back to simulation mode")
        SIMULATION_MODE = True

DB_FILE = "health_db.db"

# Function to update user stats when an event is received
def update_user_stats(user_id, timestamp):
    try:
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        
        # Insert or update user stats (increment count, update timestamp)
        c.execute('''
            INSERT INTO user_stats (user_id, event_count, last_activity)
            VALUES (?, 1, ?)
            ON CONFLICT(user_id) DO UPDATE SET
                event_count = event_count + 1,
                last_activity = ?
        ''', (user_id, timestamp, timestamp))
        
        conn.commit()

        # Update the caches after stats are updated
        update_recent_users_cache(user_id)
        update_popular_users_cache()

        conn.close()
        
        # Update the caches after stats are updated
        #update_popular_users_cache()
        #update_recent_users_cache(user_id)
        
        logger.info(f"Updated stats for user {user_id}")
    except Exception as e:
        logger.error(f"Error updating user stats: {e}")

# Function to maintain the popular users cache (top 5 by event count)
def update_popular_users_cache():
    try:
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        
        # Clear current popular users cache
        c.execute("DELETE FROM popular_users_cache")
        
        # Get top 5 users by event count
        c.execute('''
            SELECT u.user_id, u.name, s.event_count, s.last_activity
            FROM user_stats s
            JOIN users u ON s.user_id = u.user_id
            WHERE s.event_count > 0
            ORDER BY s.event_count DESC, u.name ASC
            LIMIT 5
        ''')
        
        top_users = c.fetchall()
        
        # Insert new top 5 users with ranks
        for i, (user_id, name, event_count, last_activity) in enumerate(top_users):
            c.execute('''
                INSERT INTO popular_users_cache 
                (user_id, name, event_count, last_activity, rank)
                VALUES (?, ?, ?, ?, ?)
            ''', (user_id, name, event_count, last_activity, i+1))
        
        conn.commit()
        conn.close()
        logger.info(f"Updated popular users cache with {len(top_users)} users")
    except Exception as e:
        logger.error(f"Error updating popular users cache: {e}")

# Function to maintain the recent users cache (last 5 unique users)
def update_recent_users_cache(active_user_id=None):
    try:
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        
        # Get active user's details if provided
        active_user = None
        if active_user_id:
            c.execute('''
                SELECT u.user_id, u.name, s.event_count, s.last_activity
                FROM user_stats s
                JOIN users u ON s.user_id = u.user_id
                WHERE u.user_id = ?
            ''', (active_user_id,))
            
            result = c.fetchone()
            if result:
                active_user = {
                    'user_id': result[0],
                    'name': result[1],
                    'event_count': result[2],
                    'last_activity': result[3]
                }
        
        # If we're updating for a specific user
        if active_user:
            # Check if user already exists in recent cache
            c.execute("SELECT id FROM recent_users_cache WHERE user_id = ?", (active_user_id,))
            existing = c.fetchone()
            
            if existing:
                # Update existing entry with new timestamp and rank = 1
                c.execute('''
                    UPDATE recent_users_cache 
                    SET last_activity = ?, rank = 1, event_count = ?
                    WHERE user_id = ?
                ''', (active_user['last_activity'], active_user['event_count'], active_user_id))
                
                # Increment rank for all other users
                c.execute('''
                    UPDATE recent_users_cache 
                    SET rank = rank + 1
                    WHERE user_id != ?
                ''', (active_user_id,))
            else:
                # Insert new user as rank 1
                c.execute('''
                    INSERT INTO recent_users_cache 
                    (user_id, name, event_count, last_activity, rank)
                    VALUES (?, ?, ?, ?, 1)
                ''', (
                    active_user['user_id'], 
                    active_user['name'], 
                    active_user['event_count'], 
                    active_user['last_activity']
                ))
                
                # Increment rank for all existing users
                c.execute('''
                    UPDATE recent_users_cache 
                    SET rank = rank + 1
                    WHERE user_id != ?
                ''', (active_user_id,))
            
            # Delete users beyond rank 5
            c.execute("DELETE FROM recent_users_cache WHERE rank > 5")
        else:
            # Full rebuild of recent users cache
            c.execute("DELETE FROM recent_users_cache")
            
            c.execute('''
                SELECT u.user_id, u.name, s.event_count, s.last_activity
                FROM user_stats s
                JOIN users u ON s.user_id = u.user_id
                WHERE s.event_count > 0
                ORDER BY s.last_activity DESC, u.name ASC
                LIMIT 5
            ''')
            
            recent_users = c.fetchall()
            
            # Insert new recent users
            for i, (user_id, name, event_count, last_activity) in enumerate(recent_users):
                c.execute('''
                    INSERT INTO recent_users_cache 
                    (user_id, name, event_count, last_activity, rank)
                    VALUES (?, ?, ?, ?, ?)
                ''', (user_id, name, event_count, last_activity, i+1))
        
        conn.commit()
        conn.close()
        logger.info("Updated recent users cache")
    except Exception as e:
        logger.error(f"Error updating recent users cache: {e}")

def save_event(name_id, heart_rate, temperature, pill_id):
    user_id = int(round(name_id))
    name = USER_NAME_MAP.get(user_id, f"Unknown ({user_id})")
    pill_dispensed = PILL_TYPE_MAP.get(int(pill_id), f"Unknown ({pill_id})")
    timestamp = datetime.now().isoformat()
    
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()

    try:
        #Ensure this user exist
        c.execute("INSERT OR IGNORE INTO users (user_id, name) VALUES (?, ?)", (user_id, name))

        #INSERT new event
        c.execute('''
            INSERT INTO events (user_id, heart_rate, temperature, pill_dispensed, timestamp)
            VALUES (?, ?, ?, ?, ?)
        ''', (user_id, heart_rate, temperature, int(pill_id), timestamp))

        conn.commit()
        
        # Update user statistics after saving event
        update_user_stats(user_id, timestamp)
        #update_popular_users_cache()
        #update_recent_users_cache(user_id)
        
        logger.info(f"Saved event for user {user_id}: HR={heart_rate}, temp={temperature}, pill={pill_id}")
    except Exception as e:
        logger.error(f"Error saving event: {e}")
    finally:
        conn.close()

# Handle incoming data from XBee
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
                    print(f"Parsed data: name_id={name_id}, HR={heart_rate}, temp={temperature}, pill={pill_id}")
                    # Save the parsed data to database
                    current_time = datetime.now()
                    user_id = int(round(name_id))

                    if user_id in last_message_time:
                        time_diff = current_time - last_message_time[user_id] #checking when the last message came in
                        if time_diff.total_seconds() < 15:
                            print(f"Skipping message - only {time_diff.total_seconds():.1f} seconds since last message for user {name_id}")
                            continue
                    last_message_time[user_id] = current_time
                    save_event(name_id, heart_rate, temperature, pill_id)
            except Exception as inner_e:
                print(f"Error processing data chunk: {inner_e}")
    except Exception as e:    
        print(f"Error handling XBee message: {e}")

# Register XBee callback if not in simulation mode
if not SIMULATION_MODE and device is not None:
    try:
        device.add_data_received_callback(handle_data)
        print("XBee data callback registered")
    except Exception as e:
        print(f"Error registering XBee callback: {e}")

# Endpoint to submit test data (for development without XBee hardware)
# DEVELOPMENT ONLY - This endpoint allows testing without XBee hardware
@app.route('/test_data', methods=['POST'])
def submit_test_data():
    # Check if we want to restrict this endpoint in production
    if not SIMULATION_MODE and request.headers.get('X-Development-Mode') != 'true':
        return jsonify({'error': 'This endpoint is only available in development mode'}), 403
        
    try:
        # Get data from request
        data = request.json
        
        if not data or not isinstance(data, list) or len(data) != 4:
            return jsonify({'error': 'Invalid data format. Expected [user_id, heart_rate, temperature, pill_id]'}), 400
        
        # Extract values
        name_id, heart_rate, temperature, pill_id = data
        
        # Validate data types
        try:
            name_id = float(name_id)
            heart_rate = float(heart_rate)
            temperature = float(temperature)
            pill_id = float(pill_id)
        except (ValueError, TypeError):
            return jsonify({'error': 'Invalid data types. All values should be numbers.'}), 400
        
        # Save the data
        save_event(name_id, heart_rate, temperature, pill_id)
        
        return jsonify({
            'success': True, 
            'message': 'Test data saved successfully',
            'data': {
                'user_id': int(round(name_id)),
                'user_name': USER_NAME_MAP.get(int(round(name_id)), f"Unknown ({int(round(name_id))})"),
                'heart_rate': heart_rate,
                'temperature': temperature,
                'pill_dispensed': PILL_TYPE_MAP.get(int(pill_id), f"Unknown ({int(pill_id)})"),
                'timestamp': datetime.now().isoformat()
            }
        })
    except Exception as e:
        print(f"Error saving test data: {e}")
        return jsonify({'error': f'Server error: {str(e)}'}), 500

# Endpoint to get all saved data
@app.route('/receive', methods=['GET'])
def get_all_data():
    user_id = request.args.get('user_id')

    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()
    
    if user_id:
        c.execute('''
            SELECT users.name, events.heart_rate, events.temperature, events.pill_dispensed, events.timestamp
            FROM events 
            JOIN users ON events.user_id = users.user_id     
            WHERE users.user_id = ?
            ORDER BY events.timestamp DESC
        ''', (user_id,))
    else:
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
    print(events)
    return jsonify({'events': events})

# Search endpoint for filtering health data
@app.route('/search', methods=['GET'])
def search_data():
    # Get search parameters from query string
    user_id = request.args.get('user_id')
    name = request.args.get('name')
    start_date = request.args.get('start_date')
    end_date = request.args.get('end_date')
    min_heart_rate = request.args.get('min_heart_rate')
    max_heart_rate = request.args.get('max_heart_rate')
    min_temperature = request.args.get('min_temperature')
    max_temperature = request.args.get('max_temperature')
    pill_id = request.args.get('pill_id')

    # Build the query dynamically based on provided parameters
    query = '''
        SELECT users.name, events.heart_rate, events.temperature, events.pill_dispensed, events.timestamp
        FROM events
        JOIN users ON events.user_id = users.user_id
        WHERE 1=1
    '''
    params = []

    # Add filter conditions based on provided parameters
    if user_id:
        query += " AND users.user_id = ?"
        params.append(user_id)
    
    if name:
        query += " AND users.name LIKE ?"
        params.append(f"%{name}%")
    
    if start_date:
        query += " AND events.timestamp >= ?"
        params.append(start_date)
    
    if end_date:
        query += " AND events.timestamp <= ?"
        params.append(end_date)
    
    if min_heart_rate:
        query += " AND events.heart_rate >= ?"
        params.append(float(min_heart_rate))
    
    if max_heart_rate:
        query += " AND events.heart_rate <= ?"
        params.append(float(max_heart_rate))
    
    if min_temperature:
        query += " AND events.temperature >= ?"
        params.append(float(min_temperature))
        
    if max_temperature:
        query += " AND events.temperature <= ?"
        params.append(float(max_temperature))
        
    if pill_id:
        query += " AND events.pill_dispensed = ?"
        params.append(int(pill_id))
    
    # Add ordering to get most recent events first
    query += " ORDER BY events.timestamp DESC"

    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()
    
    try:
        c.execute(query, params)
        rows = c.fetchall()
        
        events = [{
            'name': row[0],
            'heart_rate': row[1],
            'temperature': row[2],
            'pill_dispensed': PILL_TYPE_MAP.get(row[3], f"Unknown ({row[3]})"),
            'timestamp': row[4]
        } for row in rows]
        
        return jsonify({'events': events})
    except Exception as e:
        print(f"Search error: {e}")
        return jsonify({'error': str(e)}), 500
    finally:
        conn.close()

# Add a server status endpoint
@app.route('/status', methods=['GET'])
def server_status():
    try:
        # Test database connection
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        
        # Get counts within the same connection
        c.execute("SELECT COUNT(*) FROM events")
        event_count = c.fetchone()[0]
        c.execute("SELECT COUNT(*) FROM users")
        user_count = c.fetchone()[0]
        c.execute("SELECT COUNT(*) FROM user_stats WHERE event_count > 0")
        active_user_count = c.fetchone()[0]
        
        conn.close()
        
        return jsonify({
            'status': 'online',
            'mode': 'simulation' if SIMULATION_MODE else 'xbee',
            'xbee_connected': not SIMULATION_MODE and device is not None and device.is_open(),
            'database': {
                'events': event_count,
                'users': user_count,
                'active_users': active_user_count
            },
            'server_time': datetime.now().isoformat()
        })
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500

# New endpoint to get most popular users (by event count)
@app.route('/top-users/popular', methods=['GET'])
def get_popular_users():
    try:
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        
        # Check if the cache has data
        c.execute("SELECT COUNT(*) FROM popular_users_cache")
        cache_count = c.fetchone()[0]
        
        # If cache is empty, rebuild it
        if cache_count == 0:
            update_popular_users_cache()
        
        # Get users from cache, ordered by rank
        c.execute('''
            SELECT user_id, name, event_count, last_activity, rank
            FROM popular_users_cache
            ORDER BY rank
        ''')
        
        rows = c.fetchall()
        conn.close()
        
        # Format for client response
        users = []
        for row in rows:
            users.append({
                'id': f"user-{row[0]}",  # Match the client-side expected format
                'name': row[1],
                'statsCount': row[2],
                'lastActive': row[3],
                'rank': row[4]
            })
        return jsonify(users)
    except Exception as e:
        logger.error(f"Error getting popular users: {e}")
        return jsonify({'error': str(e)}), 500

# New endpoint to get most recent users (by last activity)
@app.route('/top-users/recent', methods=['GET'])
def get_recent_users():
    try:
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        
        # Check if the cache has data
        c.execute("SELECT COUNT(*) FROM recent_users_cache")
        cache_count = c.fetchone()[0]
        
        # If cache is empty, rebuild it
        if cache_count == 0:
            update_recent_users_cache()
        
        # Get users from cache, ordered by rank
        c.execute('''
            SELECT user_id, name, event_count, last_activity, rank
            FROM recent_users_cache
            ORDER BY rank
        ''')
        
        rows = c.fetchall()
        conn.close()
        
        # Format for client response
        users = []
        for row in rows:
            users.append({
                'id': f"user-{row[0]}",  # Match the client-side expected format
                'name': row[1],
                'statsCount': row[2],
                'lastActive': row[3],
                'rank': row[4]
            })
            
        logger.info(f"Returned {len(users)} recent users")
        return jsonify(users)
    except Exception as e:
        logger.error(f"Error getting recent users: {e}")
        return jsonify({'error': str(e)}), 500


if __name__ == '__main__':
    # Print startup message 
    print("=" * 50)
    print(f"Server starting in {'SIMULATION' if SIMULATION_MODE else 'XBee'} mode")
    print("=" * 50)
    print("API Endpoints:")
    print("  GET  /receive       - Get all health data")
    print("  GET  /receive?user_id=X - Get health data for specific user")
    print("  GET  /top-users/popular - Get most popular users")
    print("  GET  /top-users/recent  - Get most recent users")
    if SIMULATION_MODE:
        print("  POST /test_data     - Submit test data [user_id, heart_rate, temp, pill_id]")
    print("  GET  /status        - Check server status")
    print("  GET  /search        - Search with filters")
    print("=" * 50)
    
    # Instructions for toggling modes
    print("\nTo use XBee hardware:")
    print("1. Set SIMULATION_MODE = False")
    print("2. Ensure XBee device is connected at the correct port")
    print("3. Restart the server")
    print("=" * 50)
    
    # Initialize caches on startup
    try:
        update_popular_users_cache()
        update_recent_users_cache()
        logger.info("Initialized user stat caches on startup")
    except Exception as e:
        logger.error(f"Error initializing caches: {e}")
    
    app.run(host="0.0.0.0", port=3000)
