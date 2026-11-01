#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
#define DHT_PIN 2
#define DHT_TYPE DHT22

#define HEATER_RELAY_PIN 4
#define FAN_RELAY_PIN 5

#define SIM800_RX_PIN 10
#define SIM800_TX_PIN 11

// Environmental Thresholds & Hysteresis
#define TARGET_BROODING_TEMP 32.0f
#define TEMP_HYSTERESIS 1.5f       // Prevents rapid relay flickering
#define MAX_BROODING_TEMP 35.0f

#define CHECK_INTERVAL_MS 5000     // Sensor poll interval (5 seconds)

#endif
