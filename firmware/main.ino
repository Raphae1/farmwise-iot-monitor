#include <DHT.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include "config.h"

SoftwareSerial sim800(SIM800_RX_PIN, SIM800_TX_PIN);
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastCheckTime = 0;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  
  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  
  digitalWrite(HEATER_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);

  dht.begin();
  Serial.println("[INFO] FarmWise IoT Brooder Initialized with WDT.");
  wdt_enable(WDTO_8S);
  delay(2000);
}

void loop() {
  wdt_reset();
  unsigned long currentMillis = millis();

  if (currentMillis - lastCheckTime >= CHECK_INTERVAL_MS) {
    lastCheckTime = currentMillis;
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("[ERROR] Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" °C | ");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

    if (temperature < MIN_BROODING_TEMP) {
      digitalWrite(HEATER_RELAY_PIN, HIGH);
      digitalWrite(FAN_RELAY_PIN, LOW);
      Serial.println("[STATUS] Temp low. Heater ACTIVATED.");
    } 
    else if (temperature > MAX_BROODING_TEMP) {
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, HIGH);
      Serial.println("[STATUS] Temp high. Ventilation FAN ACTIVATED.");
      
      if (temperature > (MAX_BROODING_TEMP + 3.0)) {
        sendSMS("CRITICAL ALERT: FarmWise Brooder overheating! Temp: " + String(temperature) + "C");
      }
    } 
    else {
      digitalWrite(HEATER_RELAY_PIN, LOW);
      digitalWrite(FAN_RELAY_PIN, LOW);
      Serial.println("[STATUS] Environment optimal.");
    }
  }
}

void sendSMS(String message) {
  Serial.println("[GSM] Sending SMS emergency alert...");
  sim800.println("AT+CMGF=1");
  delay(100);
  wdt_reset();
  sim800.println("AT+CMGS=\"+1234567890\"");
  delay(100);
  wdt_reset();
  sim800.print(message);
  delay(100);
  sim800.write(26);
  for (int i = 0; i < 30; i++) {
    delay(100);
    wdt_reset();
  }
  Serial.println("[GSM] SMS dispatched.");
}
