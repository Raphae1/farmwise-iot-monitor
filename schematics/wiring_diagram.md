# FarmWise Hardware Pinout & Wiring Guide

| Component | Pin Type | Connected Arduino Pin / Rail | Notes |
| :--- | :--- | :--- | :--- |
| **DHT22 Sensor** | VCC | 5V | Requires stable regulated power |
| **DHT22 Sensor** | GND | GND | Common ground reference |
| **DHT22 Sensor** | Data | Digital Pin 2 | Pull-up resistor included on module |
| **SIM800L Module** | RXD | Digital Pin 10 | Use voltage divider if using 5V TX logic |
| **SIM800L Module** | TXD | Digital Pin 11 | Direct connection |
| **SIM800L Module** | VCC / GND| Dedicated 3.7V - 4.2V LiPo | *Crucial:* Do not power from Arduino 5V pin (causes brownouts) |
| **Heater Relay** | IN1 | Digital Pin 7 | Active LOW/HIGH depending on relay board |
| **Fan Relay** | IN2 | Digital Pin 8 | Active LOW/HIGH depending on relay board |
