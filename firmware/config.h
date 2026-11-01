#ifndef CONFIG_H
#define CONFIG_H

// Pin Configurations
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define HEATER_RELAY_PIN 7
#define FAN_RELAY_PIN 8

// SIM800L Software Serial Pins
#define SIM800_RX_PIN 10
#define SIM800_TX_PIN 11

// Brooding Environmental Limits
const float MIN_BROODING_TEMP = 31.0; // Critical low (°C)
const float MAX_BROODING_TEMP = 36.0; // Critical high (°C)
const unsigned long CHECK_INTERVAL_MS = 5000; // Sensor read frequency

#endif
