## Configuration
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
    roi_center: # the x and y coordinates of the SPAD that should serve as the center of the ROI.
      x: 3
      y: 8
    #roi_center: 167 # Same as above, but with the center specified by the SPAD number instead of x and y coordinates.
    roi_size: # width and height of ROI
      x: 4
      y: 4
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

RIO is region of interest (see [um2555](#um2555)).

Values reported by range status sensor:

| Value | Description                                                              |
|-------|--------------------------------------------------------------------------|
| 0     | Ranging measurement is valid.                   .                        |
| 1     | Raised if sigma estimator check is above the internal defined threshold. |
| 2     | Raised if signal value is below the internal defined threshold.          |
| 3     | Target is below minimum detection threshold.                             |
| 4     | Raised when phase is out of bounds.                                      |
| 5     | Raised in case of HW or VCSEL failure.                                   |
| 6     | The Range is valid but the wraparound check has not been done.           |
| 7     | Wrapped target, not matching phases.                                     |
| 8     | Internal algorithm underflow or overflow.                                |
| 9     | Specific to lite ranging.                                                |
| 10    | 1st interrupt when starting ranging in back to back mode. Ignore data.   |
| 11    | All Range ok but object is result of multiple pulses merging together.   |
| 12    | Used  by RQL  as different to phase fail.                                |
| 13    | User ROI input is not valid e.g. beyond SPAD Array.                      |
| 14    | The reported range is invalid.                                           |
| 255   | No update.                                                               |

Table of SPAD locations:

|   Y\X  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  | 10  | 11  | 12  | 13  | 14  | 15  |
|--------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
|  15    | 128 | 136 | 144 | 152 | 160 | 168 | 176 | 184 | 192 | 200 | 208 | 216 | 224 | 232 | 240 | 248 |
|  14    | 129 | 137 | 145 | 153 | 161 | 169 | 177 | 185 | 193 | 201 | 209 | 217 | 225 | 233 | 241 | 249 |
|  13    | 130 | 138 | 146 | 154 | 162 | 170 | 178 | 186 | 194 | 202 | 210 | 218 | 226 | 234 | 242 | 250 |
|  12    | 131 | 139 | 147 | 155 | 163 | 171 | 179 | 187 | 195 | 203 | 211 | 219 | 227 | 235 | 243 | 251 |
|  11    | 132 | 140 | 148 | 156 | 164 | 172 | 180 | 188 | 196 | 204 | 212 | 220 | 228 | 236 | 244 | 252 |
|  10    | 133 | 141 | 149 | 157 | 165 | 173 | 181 | 189 | 197 | 205 | 213 | 221 | 229 | 237 | 245 | 253 |
|   9    | 134 | 142 | 150 | 158 | 166 | 174 | 182 | 190 | 198 | 206 | 214 | 222 | 230 | 238 | 246 | 254 |
|   8    | 135 | 143 | 151 | 159 | 167 | 175 | 183 | 191 | 199 | 207 | 215 | 223 | 231 | 239 | 247 | 255 |
|   7    | 127 | 119 | 111 | 103 |  95 |  87 |  79 |  71 |  63 |  55 |  47 |  39 |  31 |  23 |  15 |  7  |
|   6    | 126 | 118 | 110 | 102 |  94 |  86 |  78 |  70 |  62 |  54 |  46 |  38 |  30 |  22 |  14 |  6  |
|   5    | 125 | 117 | 109 | 101 |  93 |  85 |  77 |  69 |  61 |  53 |  45 |  37 |  29 |  21 |  13 |  5  |
|   4    | 124 | 116 | 108 | 100 |  92 |  84 |  76 |  68 |  60 |  52 |  44 |  36 |  28 |  20 |  12 |  4  |
|   3    | 123 | 115 | 107 |  99 |  91 |  83 |  75 |  67 |  59 |  51 |  43 |  35 |  27 |  19 |  11 |  3  |
|   2    | 122 | 114 | 106 |  98 |  90 |  82 |  74 |  66 |  58 |  50 |  42 |  34 |  26 |  18 |  10 |  2  |
|   1    | 121 | 113 | 105 |  97 |  89 |  81 |  73 |  65 |  57 |  49 |  41 |  33 |  25 |  17 |  9  |  1  |
|   0    | 120 | 112 | 104 |  96 |  88 |  80 |  72 |  64 |  56 |  48 |  40 |  32 |  24 |  16 |  8  |  0  |


## Home Assistant

Sensors reported to Home Assistant:

![Screenshot of sensors reported to Home Assistant](imgs/vl53l1x_sensors.png?raw=true "Home Assistant screenshot")

## External resources

ST documentation:

- ### [um2356](https://www.st.com/resource/en/user_manual/um2356-vl53l1x-api-user-manual-stmicroelectronics.pdf)
- ### [um2555](https://www.st.com/resource/en/user_manual/um2555-vl53l1x-ultra-lite-driver-multiple-zone-implementation-stmicroelectronics.pdf)