# ESP display

This project is a smart display and media controller built around an ESP32-P4. It features a haptic dial, Spotify control, real-time weather, Google Calendar integration, and live overhead flight tracking.

<img width="385" height="308" alt="image" src="https://github.com/user-attachments/assets/9f32f7bc-603e-46c7-8186-9c991b379ed8" /> <img width="385" height="308" alt="image" src="https://github.com/user-attachments/assets/7940c7a6-cfbf-499e-8f28-806d6265f17b" />

## Features

* **Spotify Controller**: View current playback, control media (play, pause, next, previous, shuffle, repeat), browse playlists, and view your top listening stats.
* **Haptic Dial**: Uses a brushless DC motor and an I2C magnetic encoder to create customizable detents.
* **Volume Control**: Adjust your computer's master volume over Wi-Fi.
* **Dashboard**:
  * **Weather**: Real-time temperature, humidity, and wind using the Open-Meteo API.
  * **Calendar**: Upcoming events fetched directly from Google Calendar.
  * **Airspace**: Live overhead flight tracking via the OpenSky Network.

## Files
1. **`auth.py` (Flask Auth Server)**: A temporary web server used to authenticate and generate tokens for Spotify, Google Calendar, and OpenSky Network.
2. **`main.py` (FastAPI Backend)**: Aggregates data from rge various web APIs, formats it, and serves it to the ESP32 over a REST API.
3. **`main.ino` (ESP32 Firmware)**: The C++ code running on the microcontroller. It drives the ST7796 LCD, handles I2C encoder readings, controls the BLDC motor, and renders the user interface.
4. **`mainHost.py` (PC Volume Listener)**: A lightweight Python script running in the background of your PC to receive UDP volume commands and simulate media keystrokes.
5. **`/CAD` (Housing)**: CAD and STLs for display housing. Designed by [@SathyaKotari](https://github.com/SathyaKotari)

## Hardware Requirements
* **Microcontroller**: ESP32-P4-WIFI6.
* **Display**: ST7796 SPI LCD Display (480x320).
* **Motor Control**:
  * **Motor**: 2804 BLDC Motor.
  * **Encoder**: AS5600 Magnetic Encoder.
  * **Motor Controller** SimpleFOCmini v1.01.
* **Input**: Push button.


## Software Setup

### 1. Install Dependencies

Install the required Python packages on your backend server:

```bash
pip install -r requirements.txt

```

### 2. Configure Credentials & Authentication

Before running the main server, you need to authorize the APIs:

* **Spotify**: Create a Spotify Developer app and get your Client ID and Secret.
* **Google**: Create a Google Cloud Project, enable the Calendar API, create a service account, share calendars with service account.
* **OpenSky**: Register for an OpenSky Network account.
* Run the auth server:
```bash
python auth.py

```

### 3. Start the Backend API

Edit `api.py` to insert your specific `VALID_API_KEY`, Spotify Developer credentials, and Google Service Account file paths. Then, start the FastAPI server:

```bash
uvicorn main:app --host 0.0.0.0 --port 8080

```

### 4. Start the PC Host Listener (Optional)

If you want the smart display's volume mode to control your PC's audio, run the host script on your target computer and start a mobile hotspot:

```bash
python mainHost.py

```

### 5. Flash the ESP32

Open `main.ino` in the Arduino IDE or PlatformIO.

* Install the required libraries: `ArduinoJson`, `U8g2_for_Adafruit_GFX`, `Adafruit_GFX`.
* Update the `WIFI_SSID`, `WIFI_PASS`, `PC_HOTSPOT_IP`, `API_BASE`, and `API_KEY` variables with your specific network and server details.
  * The `PC_HOTSPOT_IP` is usually `192.168.137.1`
* Upload the code to your ESP32 board.


