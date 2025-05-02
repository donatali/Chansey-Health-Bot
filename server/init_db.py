import sqlite3
from datetime import datetime

conn = sqlite3.connect("health_db.db")
c = conn.cursor()

c.execute('''
CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY,
    name TEXT
)                 
''')

c.execute('''
CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    heart_rate INTEGER,
    temperature REAL,
    pill_dispensed INTEGER,
    timestamp TEXT,
    FOREIGN KEY(user_id) REFERENCES users(user_id)
)                 
''')

# New table for tracking user statistics
c.execute('''
CREATE TABLE IF NOT EXISTS user_stats (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER UNIQUE,
    event_count INTEGER DEFAULT 0,
    last_activity TEXT,
    FOREIGN KEY(user_id) REFERENCES users(user_id)
)
''')

# Table to cache the most popular users (by event count)
c.execute('''
CREATE TABLE IF NOT EXISTS popular_users_cache (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    name TEXT,
    event_count INTEGER,
    last_activity TEXT,
    rank INTEGER,
    FOREIGN KEY(user_id) REFERENCES users(user_id)
)
''')

# Table to cache the most recent users (by last activity)
c.execute('''
CREATE TABLE IF NOT EXISTS recent_users_cache (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    name TEXT,
    event_count INTEGER,
    last_activity TEXT,
    rank INTEGER,
    FOREIGN KEY(user_id) REFERENCES users(user_id)
)
''')

# Initialize user_stats for existing users
c.execute("SELECT user_id FROM users")
existing_users = c.fetchall()

# Use current timestamp for initialization
now = datetime.now().isoformat()

for user in existing_users:
    user_id = user[0]
    # Insert if not exists
    print(user_id)
    c.execute('''
    INSERT OR IGNORE INTO user_stats (user_id, event_count, last_activity) 
    VALUES (?, 0, ?)
    ''', (user_id, now))

conn.commit()
conn.close()
