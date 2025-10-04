# 🌱 ESPHome Soil Sensor Node

A self-contained, battery-powered **soil environment monitor** using **ESPHome** and **ESP32**, designed for long-term, ultra-low-power operation.  
It measures temperature, humidity, and power levels, intelligently enters deep sleep, and integrates seamlessly with **Home Assistant**.

---

## Project Overview

This project is a **low-power soil monitoring node** built for remote or solar-assisted deployments.  
It records **temperature, humidity**, and **battery voltage**, dynamically manages sleep cycles (day vs. night), and reports data to Home Assistant.

The design prioritizes:
- High measurement accuracy (SHT31 sensor)
- Stable ADC-based power sensing
- Multi-hour deep sleep for energy conservation
- Full automation and logging via Home Assistant

---

## Hardware Used

- **Microcontroller:** ESP32 Dev Board  
- **Sensor:** Sensirion SHT31 Temperature & Humidity Sensor (I²C, 0x44)  
- **Battery:** Single-cell LiPo (4.2 V max)  
- **Voltage Dividers:** For battery (GPIO32) and 5 V sensing (GPIO36)  
- **Wake Logic:** GPIO39 (wake from deep sleep)  
- **LED Indicator:** GPIO22 (awake state)  
- **Resistors:** 20 kΩ, 30 kΩ, 36 kΩ, 100 kΩ, 200 Ω  
- **Miscellaneous:** JST connectors, wiring harnesses, optional enclosure  

---

## Wiring Diagram

```
Power & Dividers (left)                             ESP32 (center)
───────────────────────────                   ────────────────────────
          +5V supply                            ┌──────────────────┐
              |                                 │      ESP32       │
           [20kΩ]                               │                  │
              |                                 │                  │
          ──● Node A (GPIO36 sense) ────────────┤◄─ GPIO36 (ADC 5V)
              |                                 │                  │
              |                                 │                  │
            (series link)                       │                  │
              |                                 │                  │
          ──● Node B (GPIO39 wake) ─────────────┤◄─ GPIO39 (wakeup)
              |                                 │                  │
           [30kΩ]                               │                  │
              |                                 │                  │
            GND                                 │                  │
                                                │                  │
      4.2V (LiPo)                               │                  │
              |                                 │                  │
           [36kΩ]                               │                  │
              |                                 │           GPIO21 ───── SDA ───► SHT31 SDA
          ──● Node C (GPIO32 batt) ─────────────┤◄─ GPIO32 (ADC batt)
              |                                 │           GPIO22 ────► SCK ───► SHT31 SCL
           [100kΩ]                              │           3.3V ───────────────► SHT31 VIN
              |                                 │           GND ────────────────► SHT31 GND
            GND                                 └──────────────────┘

Legend:
  [value]   = resistor
  ● Node A  = divider tap for 5V sense (GPIO36)
  ● Node B  = divider mid / wake input (GPIO39)
  ● Node C  = divider tap for LiPo sense (GPIO32)
  LED       = “Awake LED” driven by GPIO22 (add a series resistor)
```

GPIO39 doubles as a **wake pin** for deep sleep. GPIO22 drives an LED to indicate awake cycles.  
Both 5 V and LiPo voltages are measured through resistor dividers.

---

## Power Management

The node uses **ESPHome’s deep sleep** to minimize power consumption:
- **Run time:** ~1 minute per wake cycle  
- **Daytime sleep:** ~10 minutes  
- **Night sleep:** ~8 hours after 21:00  
- **Clock speed:** 80 MHz (underclocked for efficiency)  

Active current: ≈ 15 mA  
Sleep current: < 100 µA  

---

## Software (ESPHome)

Core configuration features:
- **Platform:** ESP32 using **ESP-IDF** framework  
- **Sensors:**  
  - SHT31 (temperature and humidity via I²C)  
  - ADC for battery and 5 V readings  
- **Automation:**  
  - Sleep schedule controlled by real time from Home Assistant  
  - Sends custom `esphome.battery_sleep` events to HA  
- **Battery logic:**  
  - Nonlinear voltage-to-percentage curve tuned for LiPo cells  
  - Voltage smoothing with sliding window average  

Build and flash:
```bash
esphome run soil.yaml
```

---

## Home Assistant Integration

Data is exposed via ESPHome’s native API:
- Temperature (`sensor.sht31_temperature`)
- Humidity (`sensor.sht31_humidity`)
- Battery voltage (`sensor.battery_voltage`)
- Battery percent (`sensor.battery_percent`)
- System sleep events (`esphome.battery_sleep`)

These can trigger Home Assistant automations or be displayed in dashboards.

---

## Enclosure

Optional 3D-printed enclosure fits:
- ESP32 Dev board  
- SHT31 sensor (vented section for airflow)  
- 1000 mAh LiPo battery  

Can be mounted near plant beds or outdoor pots. STL files can be created in Fusion 360 or ordered via Fiverr.

---

## Credits & Inspiration

- [ESPHome Project](https://esphome.io)  
- [Sensirion SHT3x Series](https://www.sensirion.com)  
- Configuration & design by **Zach Whitaker**  

---

## License

**GNU General Public License v3.0 (GPL-3.0)**  
This project is open-source under the GPL.  
See [GNU GPL-3.0 License](https://www.gnu.org/licenses/gpl-3.0.en.html) for full details.

---

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/whitakerz/ESP32-Soil-Sensor.git
   cd ESP32-Soil-Sensor
   ```
2. Update WiFi and API keys in `secrets.yaml`.
3. Flash to your ESP32:
   ```bash
   esphome run soil.yaml
   ```
4. Add the device to Home Assistant.
5. Optionally print and assemble an enclosure.

---
