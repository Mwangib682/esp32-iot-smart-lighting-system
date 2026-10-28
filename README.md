# ESP32 IoT Smart Lighting System

**Design and Implementation of an IoT-Based Smart Lighting Control System Using ESP32, Sound and Motion Detection**

## Overview

A physically tested IoT lighting prototype combining an ESP32 DevKit, HC-SR501 PIR motion sensor, LM393 sound detection module, MQTT, Home Assistant, and a TP-Link Tapo P100 smart plug. The current controller uses **OR logic**: either sensor can switch on the lamp. When both sensors become inactive, the lamp switches off after 30 seconds. A new activity event cancels the pending off-delay.

## System architecture

```text
HC-SR501 PIR (GPIO 27) ---\
                            ESP32 -- Wi-Fi / MQTT --> MQTT broker --> Home Assistant --> Tapo P100 --> Lamp
LM393 DO (GPIO 26) ------/
```

Home Assistant runs in a laptop-hosted virtual machine. The ESP32 never switches mains power directly.

## Hardware and wiring

| Component | Role | ESP32 connection |
|---|---|---|
| ESP32 DevKit | Sensor acquisition and MQTT communication | Controller |
| HC-SR501 PIR | Detects motion | OUT → GPIO 27 |
| LM393 sound module | Digital threshold-based sound events | DO → GPIO 26 |
| Both sensors | Common reference | GND → GND |
| TP-Link Tapo P100 | Switches mains-powered lamp | Via Home Assistant integration |
| Laptop with Home Assistant | MQTT broker and automation logic | Local network |

**Power safety:** Power the PIR according to its board specification (typically 5 V VCC). Power the LM393 module at 3.3 V only if the particular board supports it. Never connect a 5 V output directly to ESP32 GPIO; use level shifting where required. Do not connect mains to the ESP32 or breadboard.

## Software

- Arduino IDE, ESP32 board support, PubSubClient
- Home Assistant OS, MQTT integration and Mosquitto broker
- TP-Link Smart Home integration

## MQTT topics

| Topic | Values | Purpose |
|---|---|---|
| `home/motion` | `ON`, `OFF` | Motion state |
| `home/sound` | `ON`, `OFF` | Sound state |
| `home/esp8266/status` | `online`, `offline` | Connectivity (legacy name) |

Verified entity IDs in this installation:

- `binary_sensor.esp32_room_monitor_room_motion`
- `binary_sensor.esp32_room_monitor_room_sound`
- `switch.brianiot`

Entity IDs can differ on another installation. See [MQTT discovery](home-assistant/mqtt-discovery.md).

## Control truth table

| Motion | Sound | Lamp |
|---|---|---|
| OFF | OFF | OFF after 30 seconds |
| ON | OFF | ON |
| OFF | ON | ON |
| ON | ON | ON |

The [combined Home Assistant automation](home-assistant/combined-lighting.yaml) uses `mode: restart`: a new sensor state cancels any pending off delay. Both sensor states must be valid (`on` or `off`) before the automation acts; it rechecks both are `off` after 30 seconds before switching off.

## Installation

1. Connect the sensors, verifying pinouts, supply voltages and GPIO voltage compatibility.
2. Make a local copy of [ESP32 firmware](firmware/esp32_room_monitor.ino) and fill in the Wi-Fi SSID, password, MQTT broker address and MQTT credentials **locally**. Never commit those secrets.
3. Upload to the ESP32 and verify Wi-Fi and MQTT connectivity in Serial Monitor at 115200 baud.
4. Configure Home Assistant, Mosquitto and the Tapo P100 integration. Verify the smart plug can be controlled manually.
5. Publish both retained MQTT Discovery configurations described in [mqtt-discovery.md](home-assistant/mqtt-discovery.md).
6. Verify Room Motion and Room Sound entities show Detected / Clear and note the actual switch entity ID.
7. Install [combined-lighting.yaml](home-assistant/combined-lighting.yaml), adjusting entity IDs if necessary.
8. Disable separate older lighting automations to avoid conflicting commands; enable the combined controller.
9. Physically test all four sensor-state combinations and the 30-second inactivity delay.

## Reported test results

The developer physically tested the working prototype and reported successful behavior across all four sensor combinations, including automatic switch-off. Formal measurements of response latency, false triggers, power savings and reliability have **not yet been collected**.

Recommended additional tests: renewed activity during countdown, Home Assistant restart, MQTT outage, PIR hold-time variation, and sound sensitivity calibration.

## Limitations

- PIR detects movement, not continuous presence of a stationary person.
- LM393 provides threshold events, not decibel measurement or speech classification.
- The PIR hold-time and sound threshold affect response.
- Wi-Fi, broker, Home Assistant and the Tapo integration must be operational for automation.
- Current logic is Boolean OR, **not** machine-learning sensor fusion.

## Future enhancements

Selectable Motion Only, Sound Only, AND and OR modes; Home Assistant dashboard; event logging; adjustable off-delay; performance and power benchmarks; network-failure handling.

## Security

Never commit Wi-Fi passwords, MQTT passwords, tokens or private network details. The firmware in this repository contains placeholders. If credentials were previously exposed, rotate them.

## Project status

**Functional prototype, physically validated by developer.** Documentation reflects the implemented configuration and distinguishes planned work from completed features.
