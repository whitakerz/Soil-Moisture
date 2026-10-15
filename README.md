# 🌱 ESPHome Garden Soil Moisture Node

Battery-powered ESP32 soil moisture node using ESPHome, designed for long-term, low-maintenance garden deployment.  
It measures:

- Soil moisture (Vegetronix VH400 probe, converted to volumetric water content)
- Garden air temperature & humidity
- Enclosure temperature & humidity
- Battery voltage and ESP 5 V rail voltage (prevents Sleep)

All sensors receive **battery positive (V+)**, and their **ground return path is switched through a MOSFET**, ensuring near-zero sensor current during deep sleep.

---

## Design Summary

- ESP32 running ESPHome (ESP-IDF)
- Dual SHT31 sensors for ambient and enclosure temperature/humidity
- VH400 capacitive soil moisture probe powered from a 5 V boost converter
- Hard power removal from sensors using a low-side MOSFET
- Custom ESPHome I²C bus component delays I²C initialization until sensors are powered

This project targets users already familiar with ESPHome and Home Assistant. Electronics knowledge is helpful but not required.

---

## Hardware Overview

### Major Components

- **MCU:** ESP32 dev board
- **Sensors:**
  - SHT31 @ I²C address `0x44` (garden)
  - SHT31 @ I²C address `0x45` (enclosure)
  - Vegetronix VH400 moisture probe (analog output)
- **Power:**
  - Single-cell LiPo (4.2 V max)
  - 5 V boost converter for VH400
  - Sensors connected directly to **battery V+**
  - Sensor ground switched via IRLZ44N MOSFET
- **Switching & Measurement:**
  - MOSFET gate controlled by ESP32 `GPIO17` (via 200 Ω, 100 kΩ pull-down)
  - Boost rail sense (30 kΩ / 20 kΩ divider → GPIO36; ADC scale factor set empirically)
  - Battery sense (36 kΩ / 100 kΩ divider → GPIO32)
- **GPIO Use:**
  - `GPIO21` – I²C SDA  
  - `GPIO22` – I²C SCL  
  - `GPIO33` – VH400 analog input  
  - `GPIO32` – battery voltage ADC  
  - `GPIO36` – 5 V rail ADC  
  - `GPIO17` – MOSFET gate  
  - `GPIO39` – external wake  

---

## Power & Sensor Architecture

The system uses **battery V+** to power all sensors, but prevents any actual current flow until **sensor ground** is connected.

### Key Points

- Sensor V+ is always present.
- Sensor ground is routed through an IRLZ44N MOSFET.
- When the MOSFET is **off**, sensors are effectively unpowered.
- This prevents:
  - I²C bus lockups on boot
  - Sensor leakage current during deep sleep
  - Boost converter idle draw

`GPIO17` HIGH enables the MOSFET and powers all sensors; LOW disables them.

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
  │                                  │    ─┤ GPIO33   ADC_SOIL  ──► VH400 analog out
 GND                                 │    ─┤ GPIO21   I2C_SDA   ──► SDA of SHT31 @0x44, @0x45
                                     |    ─┤ GPIO22   i2C-SCL   ──► SCL of SHT31 @0x44, @0x45
                                     |
                                     │    ─┤ GPIO17   VDD_SW    ──200 Ω──► IRLZ44N GATE
                                     │                            [100 kΩ] gate pull-down → GND
                                     │                          |
                                     └──────────────────────────┘
```

---

## ESPHome Behavior

The ESPHome configuration:

- Uses ESP-IDF for low-level control.
- Manages dual SHT31 sensors on I²C.
- Reads VH400 analog moisture values.
- Measures battery and boost voltages via ADC.
- Controls the MOSFET gate (`GPIO17`) to power-cycle sensors each wake cycle.
- Performs deep sleep for long life, with adaptive intervals.

---

## Custom I²C Bus Component  
### Delayed I²C Initialization

A custom I²C component ensures I²C doesn't initialize until sensors are truly powered.

File location:

```
copy: /homeassistant/esphome/components/i2c/i2c_bus_esp_idf.h 
to:   /homeassistant/esphome/my_components/i2c/i2c_bus_esp_idf.h
```

### Modification

```cpp
// Original:
// float get_setup_priority() const override { return setup_priority::BUS; }

// Modified:
float get_setup_priority() const override { return setup_priority::DATA; }
```

### Reason

- ESPHome orders initialization based on setup priority.
- By switching to `DATA`, the I²C bus initializes **later**, after:
  - `GPIO17` enables the sensor ground MOSFET
  - Sensors and the boost converter stabilize

This prevents intermittent failures such as:

- I²C scan failures  
- Missing devices on wake  
- First-read failures after deep sleep  

[ESPHOME Components Access Instructions](https://github.com/whitakerz/Soil-Moisture/blob/main/esphome_access_instructions.md)

---

## Wake Cycle Behavior

1. ESP32 wakes.
2. `GPIO17` → HIGH to enable the MOSFET and power sensors.
3. Short stabilization delay.
4. Modified I²C bus initializes.
5. Sensors read:
   - SHT31 ×2
   - VH400 analog moisture
   - Battery & 5 V rail (Prevents Sleep)
6. Values published to Home Assistant.
7. Sleep interval chosen (normal, low battery, or night).
8. `GPIO17` → LOW cuts all sensor power.
9. ESP32 enters deep sleep.

---

## Home Assistant Entities

Typical entities exposed:

- Ambient temperature / humidity
- Enclosure temperature / humidity
- Soil moisture (processed)
- Raw VH400 analog voltage
- Battery voltage
- Boost converter voltage
- Battery percentage
- Wi-Fi RSSI
- Sleep/wake event entity

---

## License

**GNU General Public License v3.0 (GPL-3.0)**  
[Full text](https://www.gnu.org/licenses/gpl-3.0.en.html)
