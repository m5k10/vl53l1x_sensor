You can integrate this component into your ESPHome project by adding the following lines to your YAML file:
```yaml
external_components:
  - source: github://soldierkam/vl53l1x_sensor
    refresh: 1s
```

Basic configuration:
```yaml
sensor:
  - platform: vl53l1x_sensor
    id: distance_sensor
    name: Distance sensor    
```

Advanced configuration:
```yaml
i2c:
  - id: bus_a
    sda: GPIO8
    scl: GPIO9
    # Higher data rates reduce the time required for read/write operations, which
    # should prevent the "component took a long time for an operation" warning in the logs.
    frequency: 400kHz 

sensor:
  - platform: vl53l1x_sensor
    id: distance_sensor
    name: Distance sensor
    enable_pin: GPIO10 # connected to XSHUT
    distance_mode: MEDIUM # LOW (up to 1.3m, better ambient immunity), MEDIUM (up to 3m), HIGH (up to 4 m, maximum distance)
    # Time required by the sensor to perform one range measurement.
    # If fast response time is critical (e.g., obstacle avoidance in robots), use 20ms-33ms.
    # For general-purpose use, 50ms-100ms provides a good balance. For high accuracy at long distances, use 200ms-500ms.
    timing_budget: 200ms
    # Signal threshold recomendations:
    # - Use a low threshold (500-1000 kcps) if detecting dark or distant objects.
    # - Use a medium threshold (1000-2000 kcps) for most applications.
    # - Use a high threshold (2000+ kcps) for precise distance measurement with strong signals.
    signal_threshold: 512
    update_interval: 1s
    address: 0x32 # default is 0x29
    accuracy_decimals: 2
    ambient_rate_sensor: # creates sensor to report ambient IR light 
      id: "ambient_rate"
      name: "Ambient Rate [MCPS]"
    avg_signal_rate_sensor: # creates sensor to report average signal 
      id: "avg_signal_rate"
      name: "Average Signal Rate [MCPS]"
    peak_signal_rate_sensor: # creates sensor to report peak signal
      id: "peak_signal_rate"
      name: "Peak Signal Rate [MCPS]"
    range_status_sensor:
      id: "range_status"
      name: "Range Status"
```
MCPS (Mega Counts Per Second) measures the number of detected photon events per second.

Sensors reported to Home Assistant:

![Screenshot of sensors reported to Home Assistant](imgs/vl53l1x_sensors.png?raw=true "Home Assistant screenshot")


ST documentation:
- https://www.st.com/resource/en/user_manual/um2356-vl53l1x-api-user-manual-stmicroelectronics.pdf
- https://www.st.com/resource/en/user_manual/um2555-vl53l1x-ultra-lite-driver-multiple-zone-implementation-stmicroelectronics.pdf