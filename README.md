# Welcome to your Expo app 👋

This is an [Expo](https://expo.dev) project created with [`create-expo-app`](https://www.npmjs.com/package/create-expo-app).

## Get started

1. Install dependencies

   ```bash
   npm install
   ```

2. Start the app

   ```bash
    npx expo start
   ```

In the output, you'll find options to open the app in a

- [development build](https://docs.expo.dev/develop/development-builds/introduction/)
- [Android emulator](https://docs.expo.dev/workflow/android-studio-emulator/)
- [iOS simulator](https://docs.expo.dev/workflow/ios-simulator/)
- [Expo Go](https://expo.dev/go), a limited sandbox for trying out app development with Expo

You can start developing by editing the files inside the **app** directory. This project uses [file-based routing](https://docs.expo.dev/router/introduction).

## Get a fresh project

When you're ready, run:

```bash
npm run reset-project
```

This command will move the starter code to the **app-example** directory and create a blank **app** directory where you can start developing.

## Learn more

To learn more about developing your project with Expo, look at the following resources:

- [Expo documentation](https://docs.expo.dev/): Learn fundamentals, or go into advanced topics with our [guides](https://docs.expo.dev/guides).
- [Learn Expo tutorial](https://docs.expo.dev/tutorial/introduction/): Follow a step-by-step tutorial where you'll create a project that runs on Android, iOS, and the web.

## Join the community

Join our community of developers creating universal apps.

- [Expo on GitHub](https://github.com/expo/expo): View our open source platform and contribute.
- [Discord community](https://chat.expo.dev): Chat with Expo users and ask questions.

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

### Server Setup (Python/Flask)
1. Navigate to server directory:
   ```bash
   cd server
   ```

2. Activate virtual environment:
   ```bash
   source .venv/bin/activate
   ```

3. Initialize the database (first time only):
   ```bash
   python init_db.py
   ```

4. Start the server:
   ```bash
   python app.py
   ```

### Client Setup (React Native)
1. Navigate to client directory:
   ```bash
   cd client
   ```

2. Install dependencies (first time only):
   ```bash
   npm install
   ```

3. Start the React Native app:
   ```bash
   npx expo start
   ```

4. Start the IOS:
  ```bash
  i
  ```

### Testing the Server
You can test the server by sending simulated data using curl:
```bash
curl -X POST http://localhost:3000/test_data -H "Content-Type: application/json" -d '[1, 75, 98.6, 1]'
```

To retrieve data for a specific user (e.g., Abraham):
```bash
curl "http://localhost:3000/receive?user_id=1"
```

### XBee Configuration

The server can run in two modes:
1. Simulation Mode (default) - for testing without XBee hardware
2. XBee Mode - for production with XBee devices

To switch to XBee mode:
1. Connect your XBee device
2. In `server/app.py`, change:
   ```python
   SIMULATION_MODE = False
   ```
3. Verify your XBee port in `app.py`:
   ```python
   PORT = "/dev/tty.usbserial-AL02BZKQ"  # Update this to match your XBee port
   ```
4. Restart the server

Note: Keep SIMULATION_MODE = True for development without XBee hardware.

