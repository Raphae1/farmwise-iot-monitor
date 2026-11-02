# FarmWise — IoT Poultry Monitoring & Brooding Automation

FarmWise is an enterprise-grade, edge-computed IoT poultry monitoring and brooding automation system designed for 24/7 reliability in remote agricultural environments. Built around an AVR microcontroller architecture, it combines real-time environmental telemetry, intelligent hysteresis control, hardware crash recovery, and robust fault-escalation safety layers.

---

## Development Phases
- **Phase 1:** Core Automation & Crash Recovery (DHT22, hysteresis, WDT, I2C LCD)
- **Phase 2:** Safety & Reliability Hardening (Fail-safe sensor disconnect protection)
- **Phase 3:** Communication Reliability & Fault Escalation (Bounded SIM800L SMS retries, EEPROM error logging, 10-minute anomaly tracking)
- **Phase 4:** Hardware Validation & System Testing (Formalized HIL testing protocol)

> **Safety Note:** SMS transmission acknowledgement does not guarantee delivery to the recipient. Independent local safety controls remain active even during cellular network failures.
