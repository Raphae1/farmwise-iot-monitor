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
byte sensorFailureCount = 0;
const byte MAX_SENSOR_FAULTS = 3; // Trigger safe mode after 3 consecutive bad reads

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  
  // Initialize relay pins and immediately force LOW (Safe State)
  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(HEATER_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);

  dht.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("FarmWise Phase 2");
  lcd.setCursor(0, 1);
  lcd.print("Safety Hardened ");

  Serial.println("[INFO] FarmWise Phase 2 Initialized with Fail-Safe Protections.");
  
  // Enable 8-second Hardware Watchdog Timer
  wdt_enable(WDTO_8S);
  
  delay(2000);
  lcd.clear();
}

void loop() {
  wdt_reset(); // Pet the watchdog
  unsigned long currentMillis = millis();

  if (currentMillis - lastCheckTime >= CHECK_INTERVAL_MS) {
    lastCheckTime = currentMillis;
    
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // --- SAFETY CHECK 1: Sensor Failure & Disconnect Handling ---
    if (isnan(temperature) || isnan(humidity) || temperature < -40.0 || temperature > 80.0) {
      sensorFailureCount++;
      Serial.print("[WARNING] Sensor read failed! Fault count: ");
      Serial.println(sensorFailureCount);

      lcd.setCursor(0, 0);
      lcd.print("SENSOR FAULT!   ");

      if (sensorFailureCount >= MAX_SENSOR_FAULTS) {
        // FAIL-SAFE STATE: Cut heater, activate emergency status
        digitalWrite(HEATER_RELAY_PIN, LOW);
        digitalWrite(FAN_RELAY_PIN, LOW);
        
        lcd.setCursor(0, 1);
        lcd.print("HEATER DISABLED ");
        Serial.println("[CRITICAL] Max sensor failures reached. Entering FAIL-SAFE state (Heater OFF).");
        
        sendSMS("CRITICAL ERROR: FarmWise sensor disconnected/failed! Heater forced OFF to prevent hazard.");
        sensorFailureCount = 0; // Reset counter after alert dispatch
      }
      return;
    }

    // Reset failure counter on successful read
    sensorFailureCount = 0;

    // Print to Serial Monitor
    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" C | ");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

    // Update I2C LCD Dashboard
    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C H:"); lcd.print(humidity, 0); lcd.print("%  ");
    lcd.setCursor(0, 1);

    // --- SAFETY CHECK 2: Environmental Control with Hysteresis ---
    if (temperature < (TARGET_BROODING_TEMP - TEMP_HYSTERESIS)) {
      digitalWrite(HEATER_RELAY_PIN, HIGH);
      digitalWrite(FAN_RELAY_PIN, LOW);
      lcd.print("Status: HEATER ON");
      Serial.println("[STATUS] Temp low. Heater ACTIVATED.");
    } 
    else if (temperature > (TARGET_BROODING_TEMP + TEMP_HYSTERESIS)) {
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, HIGH);
      lcd.print("Status: FAN ON   ");
      Serial.println("[STATUS] Temp high. Ventilation FAN ACTIVATED.");
      
      EEPROM.update(0, (byte)temperature);

      if (temperature > MAX_BROODING_TEMP) {
        sendSMS("CRITICAL ALERT: Overheating in brooder! Temp: " + String(temperature) + "C");
      }
    } 
    else {
      // Within deadband hysteresis window
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, LOW);
      lcd.print("Status: OPTIMAL  ");
      Serial.println("[STATUS] Environment within stable hysteresis band.");
    }
  }
}

void sendSMS(String message) {
  Serial.println("[GSM] Initiating emergency SMS dispatch...");
  
  // Retry loop for robust cellular transmission
  for (int attempt = 1; attempt <= 2; attempt++) {
    sim800.println("AT+CMGF=1");
    delay(200);
    wdt_reset();
    
    sim800.println("AT+CMGS=\"+254700000000\""); // Update with target emergency phone number
    delay(200);
    wdt_reset();
    
    sim800.print(message);
    delay(200);
    sim800.write(26); // Ctrl+Z to send
    
    // Wait for transmission while petting the watchdog
    bool transmissionComplete = false;
    for (int i = 0; i < 40; i++) {
      delay(100);
      wdt_reset();
      if (sim800.find("OK")) {
        transmissionComplete = true;
        break;
      }
    }

    if (transmissionComplete) {
      Serial.println("[GSM] SMS successfully dispatched.");
      break;
    } else {
      Serial.print("[WARNING] SMS transmission attempt ");
      Serial.print(attempt);
      Serial.println(" failed. Retrying...");
    }
  }
}
