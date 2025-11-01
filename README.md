# 🌱 ESPHome Soil Sensor Node

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
  - [Vegetronix VH400](https://www.vegetronix.com/Products/VH400/) capacitive moisture probe (analog GPIO34) powered through a **boost converter (5 V output)**
- **Battery:** Single-cell LiPo (4.2 V max)  
- **Sensor Power Rail:** Sensors now powered from **battery V+** instead of a regulated 5 V line.  
- **Voltage Dividers:** Battery (GPIO32) and boost rail (GPIO36)  
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
Battery V+ (to sensors)                     ESP32 (center)                   Sensors 
─────────────────────────────        ─────────────────────────────      ────────────────
5V                                   ┌──────────────────────────┐        
  │                                  |                          |
[30 kΩ]                              │            ESP32         │
● Node (5V sense tap)               ◄┤ GPIO36  adc_5v_raw       |   
  |                                 ◄| GPIO39  WAKE_IN          |
[20 kΩ]                              │                          │
  │                                  │                          │
 GND                                 │                          │
                                     │                          │
LiPo 4.2 V max                      ◄│  ----------------------- │ ------► SHT31 (4.2V)  
  │                                  |                          |         VH400 (1.5–4.5V → 5 V) 
[36 kΩ]                              │                          │         
  │                                  │                          │
● Node (LiPo sense tap)             ◄│ GPIO32 adc_batt_raw      │
  │                                  │                          │
[100 kΩ]                             │                          │
  │                                  │    ─┤ GPIO34   ADC_SOIL  ──► VH400 analog out
 GND                                 │    ─┤ GPIO21   I2C_SDA   ──► SDA of SHT31 @0x44, @0x45
                                     │    ─┤ GPIO22   LED_OUT   ──► LED (+ series R)
             ┤                       |
                                     │    ─┤ GPIO17   VDD_SW    ──200 Ω──► IRLZ44N GATE
                                     │                          [100 kΩ] gate pull-down → GND
                                     │                          |
                                     └──────────────────────────┘

SENSOR GROUND SWITCH (low-side)
──────────────────────────────────────────────────────────────────────────────────────────────────
All sensor grounds join Sensor_GND ───────► IRLZ44N DRAIN     IRLZ44N SOURCE ───────► SYSTEM GND

Power Paths:
- Battery V+ ──► SHT31x V+ (3.7–4.2 V)
- Battery V+ ──► Boost Converter (5 V out) ──► VH400 V+
- Boost converter GND, SHT31 GND, and VH400 GND all connect to Sensor_GND (switched by MOSFET)

Notes:
- GPIO17 HIGH = sensors ON; LOW = sensors OFF.  
- Divider ratios: Boost rail → 20 kΩ / 30 kΩ; LiPo → 36 kΩ / 100 kΩ.
```

---

## Power Management

- **Active time:** ~20 s per wake  
- **Sleep intervals:**  
  - 30 min (normal)  
  - 120 min (low battery)  
  - 8 h (nighttime)  
- **Clock speed:** 80 MHz  
- **Active current:** ≈ 15 mA  
- **Sleep current:** < 100 µA  

All sensors and ADCs are depowered through the MOSFET when sleeping.  
VH400’s boost converter is powered from the switched battery line.

---

## ESPHome Configuration

**Platform:** ESP32 using **ESP-IDF**

Features:
- Dual SHT31 sensors (0x44, 0x45)  
- VH400 analog soil probe with boost-supplied V+  
- Battery and boost voltage sensing (GPIO32, GPIO36)  
- Adaptive deep sleep intervals  
- MOSFET control via GPIO17  
- Sliding window battery smoothing  
- Custom event `esphome.planter_sleep`

Build and flash:
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
- `sensor.boost_voltage`
- `sensor.battery_percent`
- `sensor.planter_wifi_signal`
- `esphome.planter_sleep` (event)

---

## Initialization Sequence

1. ESP32 drives GPIO17 HIGH → enables MOSFET → sensors powered.  
2. 150 ms delay for rail stabilization.  
3. Boost converter activates, VH400 initializes.  
4. I²C bus and ADCs start sampling.  
5. ESP32 sends telemetry, then enters deep sleep.

---

## Enclosure

Compact 3D-printed case fits:
- ESP32 dev board  
- 1000 mAh LiPo  
- SHT31 enclosure sensor (vented section)  

Mount near plants or pair with solar charging.

---

## Credits

- [ESPHome](https://esphome.io)  
- [Sensirion SHT3x](https://www.sensirion.com)  
- [Vegetronix VH400](https://www.vegetronix.com/Products/VH400/)  
- Design by **Zach Whitaker**

---

## License

**GNU General Public License v3.0 (GPL-3.0)**  
[Full text](https://www.gnu.org/licenses/gpl-3.0.en.html)
