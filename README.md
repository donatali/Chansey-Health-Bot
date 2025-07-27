# Chansey Health Bot
### Daily health monitoring home device to make health checkups fun for all!

Our project is an at-home health monitoring device in a fun form factor to cater to a wide audience from kids to Pokemon-loving adults. The device monitors basic health metrics such as heart rate and body temperature as well as environmental data such as temperature and humidity of the room it is placed in. After the user's diagnostic scan, Chansey will dispense pills (multivitamins or fever reduction pills) based on the result of the body temperature. Chansey then wirelessly stores your full health history into our server. This information is made accessible with our PokeCenter app!
<p align="center">
  <img src="https://github.com/user-attachments/assets/e186d882-ac28-400c-911e-c493140237ad" alt="Expo Poster" width=75%>
</p>

## How It's Made:
- Microcontroller: STM32L4RZI-P (Nucleo Board)
- LCD Screen: 480x320 TFT LCD Breakout Board : 4 -wire Resistive Touchscreen
- Distance Sensor: VL53LO/1XV2
- Body Temperature Sensor: MLX90614 Infrared Temperature Sensors 
- Temperature & Humidity Sensor: SHT31-D
- Heart Rate sensor: Max30105
- RFID Reader (with NTAG213): RFID-RC522
- Amplifier: PAM8302
- Speaker: P1314A
- Stepper Motor & Drivers: ROHS stepper motor 28BYJ-48
- Vibration Motor
- Wireless communcation : Digi XBee 3
- Mobile Application : Python server and client-side app using react native

## Demo:
[![Video Title](https://img.youtube.com/vi/hXiWXc6-l3U/0.jpg)](https://www.youtube.com/watch?v=hXiWXc6-l3U)
## Creators:
<p align="center">
  <img src="https://github.com/user-attachments/assets/cb69dc84-8b30-41d6-a491-d0635e904a63" alt="Natalie Do" width=20% height=20%>
  <img src="https://github.com/user-attachments/assets/48ffa2cd-6232-49dc-ab59-d49bf1657685" alt="Ava Chang" width=20% height=20%>
  <img src="https://github.com/user-attachments/assets/fd8ae601-eaab-481a-82d9-3279626b37d9" alt="Abraham Vega" width=20% height=20%>
  <img src="https://github.com/user-attachments/assets/52934278-8449-435a-93e7-6df8fb7c5e14" alt="Kris Viana" width=20% height=20%>
  <br>
  Natalie Do,  Ava Chang,  Abraham Vega,  Kris Viana
  </p>

### Special thanks to the EECS373 Staff for all the help and support
Junyi Zhu,  Matt Smith,  James Carl,  Anna Huang,  Joseph Maffetone,  Alec Zettel,  and John McCloskey
