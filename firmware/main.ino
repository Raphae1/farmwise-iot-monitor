#include <DHT.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

SoftwareSerial sim800(SIM800_RX_PIN, SIM800_TX_PIN);
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastCheckTime = 0;
unsigned long anomalyStartTime = 0;
bool anomalyActive = false;

byte sensorFailureCount = 0;
const byte MAX_SENSOR_FAULTS = 3; 
const unsigned long ANOMALY_TIMEOUT_MS = 600000; // 10 minutes of continuous abnormal temp triggers escalation

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  
  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(HEATER_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);

  dht.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("FarmWise Phase 3");
  lcd.setCursor(0, 1);
  lcd.print("Data & Resilience");

  Serial.println("[INFO] FarmWise Phase 3 Initialized with Advanced Logging & Bounded SMS.");
  
  wdt_enable(WDTO_8S);
  delay(2000);
  lcd.clear();
}

void loop() {
  wdt_reset(); 
  unsigned long currentMillis = millis();

  if (currentMillis - lastCheckTime >= CHECK_INTERVAL_MS) {
    lastCheckTime = currentMillis;
    
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // --- SENSOR FAULT HANDLING ---
    if (isnan(temperature) || isnan(humidity) || temperature < -40.0 || temperature > 80.0) {
      sensorFailureCount++;
      Serial.print("[WARNING] Sensor read failed! Fault count: ");
      Serial.println(sensorFailureCount);

      lcd.setCursor(0, 0);
      lcd.print("SENSOR FAULT!   ");

      if (sensorFailureCount >= MAX_SENSOR_FAULTS) {
        digitalWrite(HEATER_RELAY_PIN, LOW);
        digitalWrite(FAN_RELAY_PIN, LOW);
        
        lcd.setCursor(0, 1);
        lcd.print("HEATER DISABLED ");
        Serial.println("[CRITICAL] Max sensor failures reached. Fail-safe engaged.");
        
        logEventToEEPROM(1); // Error code 1: Sensor Failure
        sendSMSWithBoundedRetry("CRITICAL: FarmWise sensor failed! Heater forced OFF.");
        sensorFailureCount = 0;
      }
      return;
    }

    sensorFailureCount = 0;

    // Display telemetry
    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" C | ");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C H:"); lcd.print(humidity, 0); lcd.print("%  ");
    lcd.setCursor(0, 1);

    // --- ENVIRONMENTAL CONTROL & PROLONGED ANOMALY TRACKING ---
    bool isAbnormal = false;

    if (temperature < (TARGET_BROODING_TEMP - TEMP_HYSTERESIS)) {
      digitalWrite(HEATER_RELAY_PIN, HIGH);
      digitalWrite(FAN_RELAY_PIN, LOW);
      lcd.print("Status: HEATER ON");
      Serial.println("[STATUS] Temp low. Heater ACTIVATED.");
      isAbnormal = true;
    } 
    else if (temperature > (TARGET_BROODING_TEMP + TEMP_HYSTERESIS)) {
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, HIGH);
      lcd.print("Status: FAN ON   ");
      Serial.println("[STATUS] Temp high. Ventilation FAN ACTIVATED.");
      
      EEPROM.update(0, (byte)temperature);
      isAbnormal = true;

      if (temperature > MAX_BROODING_TEMP) {
        sendSMSWithBoundedRetry("CRITICAL ALERT: Overheating! Temp: " + String(temperature) + "C");
      }
    } 
    else {
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, LOW);
      lcd.print("Status: OPTIMAL  ");
      Serial.println("[STATUS] Environment within stable hysteresis band.");
      isAbnormal = false;
    }

    // Prolonged Anomaly Tracking Logic
    if (isAbnormal) {
      if (!anomalyActive) {
        anomalyActive = true;
        anomalyStartTime = currentMillis;
      } else if ((currentMillis - anomalyStartTime) >= ANOMALY_TIMEOUT_MS) {
        Serial.println("[CRITICAL] Prolonged environmental anomaly detected (>10 mins out of bounds).");
        logEventToEEPROM(2); // Error code 2: Prolonged Anomaly
        sendSMSWithBoundedRetry("WARNING: Brooder out of bounds for >10 mins! Check system.");
        anomalyStartTime = currentMillis; // Reset timer to prevent SMS spam every check cycle
      }
    } else {
      anomalyActive = false;
    }
  }
}

// Bounded SMS Retry with strict timeout and failure logging
void sendSMSWithBoundedRetry(String message) {
  Serial.println("[GSM] Initiating bounded SMS transmission...");
  bool deliveryConfirmed = false;
  const byte MAX_RETRIES = 3;

  for (byte attempt = 1; attempt <= MAX_RETRIES; attempt++) {
    wdt_reset();
    sim800.println("AT+CMGF=1");
    delay(300);
    
    sim800.println("AT+CMGS=\"+254700000000\""); // Update with recipient number
    delay(300);
    
    sim800.print(message);
    delay(100);
    sim800.write(26); // Ctrl+Z
    
    // Wait up to 5 seconds for network acknowledgment
    unsigned long timeout = millis();
    bool modemAcknowledged = false;
    
    while (millis() - timeout < 5000) {
      wdt_reset();
      if (sim800.find("OK")) {
        modemAcknowledged = true;
        break;
      }
    }

    if (modemAcknowledged) {
      Serial.print("[GSM] Attempt ");
      Serial.print(attempt);
      Serial.println(": Modem accepted transmission command.");
      deliveryConfirmed = true;
      break;
    } else {
      Serial.print("[WARNING] Attempt ");
      Serial.print(attempt);
      Serial.println(" timed out without modem acknowledgment. Retrying...");
    }
  }

  if (!deliveryConfirmed) {
    Serial.println("[ERROR] SMS transmission failed completely after max retries. Logging failure state.");
    logEventToEEPROM(3); // Error code 3: SMS Delivery Failure
  }
}

void logEventToEEPROM(byte errorCode) {
  // Simple circular logging or state recording in EEPROM addresses 10-20
  byte pointer = EEPROM.read(5); // Address 5 tracks log index
  if (pointer > 20) pointer = 10;
  
  EEPROM.update(pointer, errorCode);
  EEPROM.update(5, pointer + 1);
  Serial.print("[EEPROM] Logged error code ");
  Serial.print(errorCode);
  Serial.print(" at address ");
  Serial.println(pointer);
}
