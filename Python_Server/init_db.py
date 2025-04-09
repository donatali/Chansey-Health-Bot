import sqlite3

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

conn.commit()
conn.close()