#!/bin/bash
# Script to reorganize the react_native_jobs project
# Created: 2025-04-13

# Stop on any error
set -e

echo "=== Starting project reorganization ==="
echo "This script will reorganize your project according to the following structure:"
echo "
react_native_jobs/
├── client/                  # React Native app
│   ├── app/                # Your app directory
│   ├── assets/            # All assets
│   ├── components/        # React components
│   ├── constants/         # Constants and configs
│   ├── hook/              # Custom hooks
│   ├── styles/            # Shared styles
│   ├── utils/             # Utility functions
│   └── package.json       # React Native dependencies
│
├── server/                  # Python backend
│   ├── .venv/             # Virtual environment
│   ├── app.py             # Main server file
│   ├── mappings.py        # User and pill mappings
│   ├── health_db.db       # SQLite database
│   └── requirements.txt   # Python dependencies
│
└── README.md                # Project documentation
"

echo "=== Creating backup ==="
# Create a backup just in case
BACKUP_DIR="../react_native_jobs_backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
cp -R ./* "$BACKUP_DIR"
echo "Backup created at: $BACKUP_DIR"

echo "=== Creating new directory structure ==="
# Create the new directory structure
mkdir -p client/app
mkdir -p client/assets
mkdir -p client/components
mkdir -p client/constants
mkdir -p client/hook
mkdir -p client/styles
mkdir -p client/utils
mkdir -p server

echo "=== Moving files to client directory ==="
# Move React Native files to client directory
mv app/* client/app/ 2>/dev/null || true
mv assets/* client/assets/ 2>/dev/null || true
mv components/* client/components/ 2>/dev/null || true
mv constants/* client/constants/ 2>/dev/null || true
mv hook/* client/hook/ 2>/dev/null || true
mv styles/* client/styles/ 2>/dev/null || true
mv utils/* client/utils/ 2>/dev/null || true

# Move package files to client
mv package.json client/ 2>/dev/null || true
mv package-lock.json client/ 2>/dev/null || true
mv app.json client/ 2>/dev/null || true
mv tsconfig.json client/ 2>/dev/null || true
mv expo-env.d.ts client/ 2>/dev/null || true

# Move native folders to client
mv android client/ 2>/dev/null || true
mv ios client/ 2>/dev/null || true
mv .expo client/ 2>/dev/null || true

echo "=== Moving files to server directory ==="
# Move Python server files to server directory
mv Python_Server/app.py server/ 2>/dev/null || true
mv Python_Server/mappings.py server/ 2>/dev/null || true
mv Python_Server/health_db.db server/ 2>/dev/null || true
mv Python_Server/.venv server/ 2>/dev/null || true
mv Python_Server/__pycache__ server/ 2>/dev/null || true
mv Python_Server/init_db.py server/ 2>/dev/null || true

# Create requirements.txt if it doesn't exist
if [ ! -f "server/requirements.txt" ]; then
  echo "# Python dependencies for server" > server/requirements.txt
  echo "flask" >> server/requirements.txt
  echo "flask-cors" >> server/requirements.txt
  echo "digi-xbee" >> server/requirements.txt
fi

echo "=== Copying README and .gitignore to root ==="
# Copy README to root
mv README.md ./ 2>/dev/null || true
# Keep .gitignore at root
mv .gitignore ./ 2>/dev/null || true

echo "=== Removing unnecessary files and directories ==="
# Remove directories that are no longer needed after the move
rm -rf app-example/ 2>/dev/null || true
rm -rf assets-example/ 2>/dev/null || true
rm -rf Desktop/ 2>/dev/null || true
rm -f old_app.py 2>/dev/null || true
rm -rf .venv/ 2>/dev/null || true
rm -rf app/ 2>/dev/null || true
rm -rf assets/ 2>/dev/null || true
rm -rf components/ 2>/dev/null || true
rm -rf constants/ 2>/dev/null || true
rm -rf hook/ 2>/dev/null || true
rm -rf styles/ 2>/dev/null || true
rm -rf utils/ 2>/dev/null || true
rm -rf Python_Server/ 2>/dev/null || true

echo "=== Updating README with new structure information ==="
# Add information about the new structure to README
cat << 'EOF' >> README.md

## Project Structure

The project is organized into two main directories:

### Client (React Native)
- `/client` - React Native mobile application
  - `/app` - App screens and navigation
  - `/assets` - Images, fonts, and other static assets
  - `/components` - Reusable UI components
  - `/constants` - App constants and configuration
  - `/hook` - Custom React hooks
  - `/styles` - Shared styles
  - `/utils` - Utility functions

### Server (Python/Flask)
- `/server` - Python Flask backend server
  - `app.py` - Main server file
  - `mappings.py` - User and pill type mappings
  - `health_db.db` - SQLite database
  - `.venv/` - Python virtual environment

## Development

To run the server:
```bash
cd server
source .venv/bin/activate
python app.py
```

To run the React Native app:
```bash
cd client
npx expo start
```

EOF

echo "=== Reorganization complete ==="
echo "Your project has been successfully reorganized!"
echo "To run the server: cd server && source .venv/bin/activate && python app.py"
echo "To run the React Native app: cd client && npx expo start"

