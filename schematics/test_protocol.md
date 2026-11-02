# FarmWise Hardware Validation & System Testing Protocol (Phase 4)

This document outlines the systematic test procedures required to validate the safety, fault tolerance, and reliability of the FarmWise IoT poultry monitoring node under realistic field conditions prior to full-scale deployment.

---

## Test Suite

### 1. DHT22 Sensor Fault & Disconnect Test
* **Objective:** Verify that a disconnected or shorted sensor triggers immediate fail-safe protocols without latching heaters ON.
* **Procedure:** 
  1. Power on the system and verify normal telemetry on the I2C LCD.
  2. Physically disconnect the DHT22 data wire from Digital Pin 2.
  3. Observe the system for 3 consecutive poll cycles (~15 seconds).
* **Expected Result:** 
  - LCD displays \SENSOR FAULT!\.
  - Heater relay forces \LOW\ (Emergency shutdown).
  - Serial monitor logs sensor failure count and triggers an SMS alert.

### 2. Hysteresis & Relay Boundary Test
* **Objective:** Ensure relay chatter is eliminated and environmental thresholds operate correctly.
* **Procedure:**
  1. Expose the DHT22 sensor to a gentle heat source (e.g., warm air from a hand or hair dryer on cool setting) past \TARGET + HYSTERESIS\ (\>33.5°C).
  2. Allow temperature to drop naturally below \TARGET - HYSTERESIS\ (\<30.5°C).
* **Expected Result:**
  - Fan relay engages when temperature exceeds upper bound; heater remains OFF.
  - Heater relay engages when temperature drops below lower bound; fan remains OFF.
  - No rapid switching or relay clicking when temperature hovers right at the threshold boundary.

### 3. SIM800L Cellular Retry Exhaustion Test
* **Objective:** Validate bounded retry limits and timeout mechanisms during network interruptions.
* **Procedure:**
  1. Power down or disconnect the SIM800L module's TX/RX or power supply.
  2. Trigger an event that requires an SMS alert (e.g., simulated sensor fault or severe overheating).
* **Expected Result:**
  - Firmware attempts up to 3 retries with a strict 5-second response window per attempt.
  - Execution does not freeze or hang indefinitely.
  - After max retries fail, error code \3\ is logged to EEPROM.

### 4. Non-Volatile EEPROM Persistence Test
* **Objective:** Verify diagnostic error codes survive power interruptions.
* **Procedure:**
  1. Induce a fault condition (such as a sensor disconnect or communication timeout) to write error codes to EEPROM.
  2. Unplug the Arduino power entirely for 10 seconds.
  3. Reconnect power and read EEPROM memory logs via serial/debug output.
* **Expected Result:**
  - Stored error codes and state pointer remain intact across the power cycle.

### 5. Prolonged Anomaly Escalation Test
* **Objective:** Test secondary escalation warnings when temperature remains out of bounds for extended durations.
* **Procedure:**
  1. Keep the sensor environment outside optimal thresholds continuously for over 10 minutes.
* **Expected Result:**
  - Timer tracks continuous abnormal state.
  - Triggers an escalation warning message after 10 minutes and resets the anomaly timer.

### 6. Hardware Watchdog Timer (WDT) Recovery Test
* **Objective:** Verify that the system automatically recovers from fatal lockups.
* **Procedure:**
  1. Temporarily modify code with a blocking infinite loop (\while(1);\ inside a test build) *without* calling \wdt_reset()\.
  2. Flash and observe system behavior.
* **Expected Result:**
  - After 8 seconds, the AVR hardware watchdog timer times out and forces a clean hardware reboot, re-initializing the system safely.
