#include <DHT.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

SoftwareSerial sim800(SIM800_RX_PIN, SIM800_TX_PIN);
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns, 2 rows

unsigned long lastCheckTime = 0;
bool heaterState = false;
bool fanState = false;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  
  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  
  digitalWrite(HEATER_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);

  dht.begin();
  
  // Initialize I2C LCD Display
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("FarmWise IoT");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  Serial.println("[INFO] FarmWise Ultimate Brooder Initialized.");
  
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

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("[ERROR] Failed to read from DHT sensor!");
      lcd.setCursor(0, 0);
      lcd.print("Sensor Error!   ");
      return;
    }

    // Print to Serial Monitor
    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" C | ");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

    // Update I2C LCD Dashboard
    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C H:"); lcd.print(humidity, 0); lcd.print("%  ");
    lcd.setCursor(0, 1);

    // Hysteresis Control Logic (Prevents relay chatter)
    if (temperature < (TARGET_BROODING_TEMP - TEMP_HYSTERESIS)) {
      heaterState = true;
      fanState = false;
      digitalWrite(HEATER_RELAY_PIN, HIGH);
      digitalWrite(FAN_RELAY_PIN, LOW);
      lcd.print("Status: HEATER ON");
      Serial.println("[STATUS] Temp low. Heater ACTIVATED.");
    } 
    else if (temperature > (TARGET_BROODING_TEMP + TEMP_HYSTERESIS)) {
      heaterState = false;
      fanState = true;
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, HIGH);
      lcd.print("Status: FAN ON   ");
      Serial.println("[STATUS] Temp high. Ventilation FAN ACTIVATED.");
      
      // EEPROM Runtime State Log simulation
      EEPROM.update(0, (byte)temperature);

      if (temperature > MAX_BROODING_TEMP) {
        sendSMS("CRITICAL ALERT: Overheating in brooder! Temp: " + String(temperature) + "C");
      }
    } 
    else if (temperature >= TARGET_BROODING_TEMP && temperature <= (TARGET_BROODING_TEMP + TEMP_HYSTERESIS)) {
      // Within deadband hysteresis window - hold current state to prevent flickering
      lcd.print("Status: OPTIMAL  ");
      Serial.println("[STATUS] Environment within stable hysteresis band.");
    }
  }
}

void sendSMS(String message) {
  Serial.println("[GSM] Sending SMS emergency alert...");
  sim800.println("AT+CMGF=1");
  delay(100);
  wdt_reset();
  
  sim800.println("AT+CMGS=\"+254700000000\""); // Replace with your target emergency number
  delay(100);
  wdt_reset();
  
  sim800.print(message);
  delay(100);
  sim800.write(26); // Ctrl+Z
  
  for (int i = 0; i < 30; i++) {
    delay(100);
    wdt_reset();
  }
  Serial.println("[GSM] SMS dispatched.");
}
