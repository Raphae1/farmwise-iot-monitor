# FarmWise — IoT Poultry Monitoring & Brooding Automation

FarmWise is an enterprise-grade, edge-computed IoT poultry monitoring and brooding automation system designed for 24/7 reliability in remote agricultural environments. Built around an AVR microcontroller architecture, it combines real-time environmental telemetry, intelligent hysteresis control, hardware crash recovery, and robust fault-escalation safety layers.

---

## Architecture & Tech Stack
* **Microcontroller:** AVR ATmega (Arduino compatible)
* **Sensor:** DHT22 (Temperature & Humidity)
* **Actuators:** Optocoupler Relay Modules (Heater & Ventilation Fan)
* **Telemetry & Display:** I2C 16x2 LCD Dashboard (LiquidCrystal_I2C)
* **Cellular Communication:** SIM800L GSM/GPRS Module (SoftwareSerial)
* **Resilience:** Native AVR Hardware Watchdog Timer (WDT) & Non-Volatile EEPROM Logging

---

## Phase 1: Core Automation & Crash Recovery
* **Automated Environmental Control:** Continuous polling of DHT22 sensor to maintain optimal brooding temperatures.
* **Intelligent Hysteresis Deadband:** Implements a temperature deadband (TEMP_HYSTERESIS) to eliminate mechanical relay chatter and premature hardware wear at threshold boundaries.
* **Hardware Watchdog Timer (WDT):** Integrates an 8-second native hardware WDT that automatically reboots the node if execution freezes.
* **Local Telemetry Dashboard:** Renders live temperature, humidity, and relay status directly on a 16x2 I2C LCD screen.

---

## Phase 2: Safety & Reliability Hardening
* **Fail-Safe Sensor Disconnect Protection:** Detects consecutive invalid or NaN sensor reads, instantly dropping heater relays to a safe state (LOW), displaying a warning on the LCD, and dispatching an emergency SMS.
* **Safe Power-On State Machine:** Enforces explicit initial low states on all control pins during startup to prevent erratic switching during brownouts or reboots.

---

## Phase 3: Communication Reliability & Operational Fault Escalation
Phase 3 strengthens FarmWise's fault-handling capabilities by introducing bounded cellular communication retries, persistent error logging, and prolonged environmental anomaly detection.

### 1. Bounded SIM800L SMS Retry Mechanism
* Implemented a maximum of 3 transmission attempts for critical SMS alerts.
* Added a strict 5-second response timeout to prevent the GSM communication loop from blocking indefinitely.
* Added modem response checks to determine whether transmission attempts receive expected acknowledgements.
* Prevents unlimited retry loops and excessive alert attempts during temporary network interruptions.

### 2. Persistent Communication Failure Logging
* Introduced EEPROM error logging for failed SMS transmission attempts.
* Writes error code 3 when the modem fails to acknowledge transmission after exhausting the retry limit.
* Preserves communication failure diagnostics across unexpected power interruptions and microcontroller restarts.

### 3. Prolonged Environmental Anomaly Escalation
* Implemented continuous anomaly tracking for abnormal heating and cooling behavior.
* Monitors whether the heater successfully raises the temperature or the ventilation fan successfully reduces it.
* Triggers an escalation warning when expected environmental correction does not occur within 10 continuous minutes.
* Provides an additional layer of fault awareness beyond basic temperature threshold control.

> **Safety Note:** SMS transmission acknowledgement does not guarantee delivery to the recipient. Independent local safety controls must remain active even when cellular communication fails.

---

## Repository Structure
\\\	ext
farmwise-iot-monitor/
├── firmware/
│   ├── config.h          # Pin mappings and environmental thresholds
│   └── main.ino          # Core embedded control loop, WDT, and fault logic
├── schematics/
│   └── wiring_diagram.md # Hardware pinout and power isolation reference
└── README.md             # Project documentation and specifications
\\\
"@ | Out-File -FilePath "README.md" -Encoding utf8

git add README.md
git commit -m "docs(readme): add comprehensive Phase 3 communication reliability and fault escalation specification"
git push origin main

cd C:\Users\HomePC\Desktop\farmwise-iot-monitor

# 1. Update config.h to include firmware version v1.3.0
@"
#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "v1.3.0"

// Pin Definitions
#define DHT_PIN 2
#define DHT_TYPE DHT22

#define HEATER_RELAY_PIN 4
#define FAN_RELAY_PIN 5

#define SIM800_RX_PIN 10
#define SIM800_TX_PIN 11

// Environmental Thresholds & Hysteresis
#define TARGET_BROODING_TEMP 32.0f
#define TEMP_HYSTERESIS 1.5f       
#define MAX_BROODING_TEMP 35.0f

#define CHECK_INTERVAL_MS 5000     

#endif
