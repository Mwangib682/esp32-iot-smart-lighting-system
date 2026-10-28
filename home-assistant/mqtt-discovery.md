# MQTT Discovery

In Home Assistant → Settings → Devices & services → MQTT → Configure → Publish a packet, publish each JSON configuration with **Retain enabled** and QoS 0.

## Motion

Topic: `homeassistant/binary_sensor/esp32_room_motion/config`

```json
{"name":"Room Motion","unique_id":"esp32_room_motion_v1","state_topic":"home/motion","payload_on":"ON","payload_off":"OFF","device_class":"motion","device":{"identifiers":["esp32_room_monitor"],"name":"ESP32 Room Monitor"}}
```

## Sound

Topic: `homeassistant/binary_sensor/esp32_room_sound/config`

```json
{"name":"Room Sound","unique_id":"esp32_room_sound_v1","state_topic":"home/sound","payload_on":"ON","payload_off":"OFF","device_class":"sound","device":{"identifiers":["esp32_room_monitor"],"name":"ESP32 Room Monitor"}}
```

If an invalid retained discovery message makes a sensor unavailable, publish the corrected JSON to the same topic with Retain enabled.
