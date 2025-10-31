## 🌱 ESPHome Soil Sensor Node

A **self-contained**, battery-powered **soil environment monitor** using **ESPHome** and **ESP32**, designed for **long-term, ultra-low-power operation**.  
It measures temperature, humidity, soil moisture, and system voltages, enters deep sleep between updates, and integrates natively with **Home Assistant**.

---

## Project Overview

This project is a **low-power soil monitoring node** for remote or solar-assisted deployments.  
It records:

- Air and soil **temperature**
- **Humidity**
- **Capacitive soil moisture**
- **Battery voltage**

The system dynamically manages sleep cycles, optimizing performance and energy use.

**Design goals:**
- High measurement accuracy using dual SHT31 sensors  
- Reliable analog sensing with stable divider ratios  
- Complete power isolation of sensors via MOSFET switching  
- Multi-hour deep sleep for minimal idle current  
- Full telemetry via Home Assistant

---

## Hardware Used

- **MCU:** ESP32 Dev Board  
- **Sensors:**
  - SHT31 temperature & humidity sensor (I²C, 0x44)  
  - SHT31 enclosure T/H sensor (I²C, 0x45)  
  - [Vegetronix VH400](https://www.vegetronix.com/Products/VH400/) capacitive moisture probe (analog GPIO34)
- **Battery:** Single-cell LiPo (4.2 V max)  
- **Voltage Dividers:** Battery (GPIO32) and 5 V rail (GPIO36)  
- **Wake Logic:** GPIO39 (external wake)  
- **Sensor Power Switch:** GPIO17 → IRLZ44N gate (low-side switch for sensor ground rail)  
- **LED Indicator:** GPIO22 (awake state)  
- **Resistors:** 20 kΩ, 30 kΩ, 36 kΩ, 100 kΩ, 200 Ω  
- **MOSFET:** IRLZ44N logic-level N-channel  
  - Gate: GPIO17 via 200 Ω resistor  
  - Gate pull-down: 100 kΩ → GND  
  - Drain: sensor ground return  
  - Source: system ground  

---

## Wiring Diagram

```
Power & Dividers (left)                          ESP32 (center)                         Sensors & Switched Rail (right)
───────────────────────────────────        ────────────────────────────────        ─────────────────────────────────────────────
+5V SUPPLY                                 ┌──────────────────────────────┐        +3.3V or +5V  ──────►  V+ to SHT31x, VH400
  │                                        │            ESP32             │               │
[20kΩ]                                     │                              │               │
  │                                       ◄┤ GPIO36  ADC_5V  ◄── Node_A ─●────────────────┘
● Node_A (5V sense tap)                    │                              │
  │                                        │                              │
[30kΩ]                                     │                              │
  │                                        │                              │
 GND                                       │                              │
                                           │                              │
LiPo 4.2V max                              │                              │
  │                                       ◄┤ GPIO32  ADC_BATT ◄── Node_C ─●
[36kΩ]                                     │                              │
  │                                        │                              │
● Node_C (LiPo sense tap)                  │                              │
  │                                        │                              │
[100kΩ]                                    │                              │
  │                                        │      ─┤ GPIO34   ADC_SOIL  ──► VH400 analog out
 GND                                       │      ─┤ GPIO21   I2C_SDA   ──► SDA of SHT31 @0x44, @0x45
                                           │      ─┤ GPIO22   LED_OUT   ──► LED (+ series R)
Optional wake source ────────────────►    ◄┤ GPIO39   WAKE_IN
                                           │      ─┤ GPIO17   VDD_SW    ──200Ω──► IRLZ44N GATE
                                           │                              │
                                           │                           [100kΩ] gate pull-down → GND
                                           └──────────────────────────────┘

SENSOR GROUND SWITCH (low-side)
──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
All sensor grounds join Sensor_GND ───────────────► IRLZ44N DRAIN     IRLZ44N SOURCE ─────────────► SYSTEM GND

Notes:
- IRLZ44N acts as a low-side switch for Sensor_GND. GPIO17 HIGH = sensors ON; LOW = sensors OFF.
- Every sensor ground connects to Sensor_GND, not system ground.
- If I²C lines back-power sensors, add 1–2 kΩ series resistors in SDA/SCL near the ESP32.
- Divider ratios: 5 V → 20k/30k; LiPo → 36k/100k.
```

---

## Power Management

- **Run time:** ~20 s active per wake  
- **Sleep intervals:**  
  - 30 min (normal)  
  - 120 min (low battery)  
  - 8 h (nighttime)  
- **Clock speed:** 80 MHz  
- **Active current:** ≈ 15 mA  
- **Sleep current:** < 100 µA  

All sensors and ADCs are powered down through the MOSFET when sleeping.

---

## ESPHome Configuration

**Platform:** ESP32 using **ESP-IDF**

**Core features:**
- Dual SHT31 I²C sensors (0x44, 0x45)
- VH400 analog soil probe (GPIO34)
- Battery & 5 V sense via ADC (GPIO32, GPIO36)
- Adaptive deep sleep intervals
- MOSFET power control on GPIO17
- Sliding window battery smoothing
- Custom event publishing: `esphome.planter_sleep`

**Build and flash:**
```bash
esphome run NewSensor.yaml
```

---

## Home Assistant Integration

Entities:
- `sensor.planter_probe_temperature`
- `sensor.planter_probe_humidity`
- `sensor.enclosure_temperature`
- `sensor.enclosure_humidity`
- `sensor.capacitive_soil_moisture`
- `sensor.soil_moisture_raw`
- `sensor.battery_voltage`
- `sensor.battery_percent`
- `sensor.planter_wifi_signal`
- `esphome.planter_sleep` (event)

These can be used in automations, dashboards, or scripts.

---

## I²C and Power Initialization

On boot:
1. ESP32 asserts GPIO17 HIGH → enables MOSFET → sensors powered.  
2. 100–200 ms delay for VDD stabilization.  
3. I²C bus initializes, ADCs sampled.  
4. ESP enters main loop, then deep sleep after telemetry.

This sequence prevents bus lockups from slow sensor startup.

---

## Enclosure

Optional 3D-printed case fits:
- ESP32 dev board  
- 1000 mAh LiPo  
- SHT31 enclosure sensor (vented section)  

Mount near plants or integrate with solar charging.

---

## Credits

- [ESPHome](https://esphome.io)  
- [Sensirion SHT3x](https://www.sensirion.com)  
- Design by **Zach Whitaker**

---

## License

**GNU General Public License v3.0 (GPL-3.0)**  
[Full text](https://www.gnu.org/licenses/gpl-3.0.en.html)
