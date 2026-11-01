#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
#define DHT_PIN 2
#define DHT_TYPE DHT22

#define HEATER_RELAY_PIN 4
#define FAN_RELAY_PIN 5

#define SIM800_RX_PIN 10
#define SIM800_TX_PIN 11

// Environmental Thresholds & Hysteresis (Adjusted for Week 2)
#define TARGET_BROODING_TEMP 29.5f
#define TEMP_HYSTERESIS 1.5f       
#define MAX_BROODING_TEMP 33.0f

#define CHECK_INTERVAL_MS 5000     

#endif
