# 🌱 ESPHome Soil Sensor Node

A self-contained, battery-powered **soil environment monitor** using **ESPHome** and **ESP32**, designed for long-term, ultra-low-power operation.
It measures temperature, humidity, raw soil moisture, and power levels, intelligently enters deep sleep, and integrates seamlessly with **Home Assistant**.

---

## Project Overview

This project is a **low-power soil monitoring node** built for remote or solar-assisted deployments.
It records **temperature, humidity, capacitive soil moisture**, and **battery voltage**, dynamically manages sleep cycles (day vs. night), and reports data to Home Assistant.

The design prioritizes:
- High measurement accuracy (dual SHT31 sensors with managed power sequencing)
- Stable ADC-based power sensing and raw-to-percent conversion for capacitive probes
- Multi-hour deep sleep for energy conservation
- Full automation and logging via Home Assistant

---

## Hardware Used

- **Microcontroller:** ESP32 Dev Board  
- **Sensors:**
  - Sensirion SHT31 temperature & humidity sensor for planter probe readings (I²C, 0x44)
  - Sensirion SHT31 temperature probe dedicated to enclosure monitoring (I²C, 0x45)
  - [Vegetronix VH400](https://www.vegetronix.com/Products/VH400/) capacitive soil moisture probe with analog readout (GPIO34)
- **Battery:** Single-cell LiPo (4.2 V max)  
- **Voltage Dividers:** For battery (GPIO32) and 5 V sensing (GPIO36)  
- **Wake Logic:** GPIO39 (wake from deep sleep)  
- **Sensor Power Gate:** GPIO17 (switchable VDD rail for probe warm-up)
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
            GND                                 │           GPIO34 ────────────► Capacitive Probe Output
                                                │           GND    ────────────► SHT31 GND      ───► SHT31 GND
      4.2V (LiPo)                               │           3.3V   ────────────► SHT31 VIN      ───► SHT31 VIN 
              |                                 │           GPIO17 ───────► Sensor VDD Switch   ───► Probe Power Rail
           [36kΩ]                               │           GPIO22 ────►         SCK ───► SHT31 SCL(x2)  ───► SHT31 SCL
              |                                 │           GPIO21 ────►         SDA ───► SHT31 SDA(x2)  ───► SHT31 SDA
          ──● Node C (GPIO32 batt) ─────────────┤◄─ GPIO32 (ADC batt)
              |                                 │                  |
           [100kΩ]                              │                  |
              |                                 │                  |
            GND                                 │                  |
                                                │                  |
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
- **Run time:** ~20 seconds per wake cycle (enough to connect and publish)
- **Daytime sleep:** ~10 minutes when the battery is healthy
- **Extended daytime sleep:** ~20 minutes when the battery drops below 30%
- **Night sleep:** ~8 hours after 21:00
- **Clock speed:** 80 MHz (underclocked for efficiency)

Active current: ≈ 15 mA  
Sleep current: < 100 µA  

---

## Software (ESPHome)

Core configuration features:
- **Platform:** ESP32 using **ESP-IDF** framework
- **Sensors:**
  - SHT31 (Planter probe temperature & humidity via I²C at 0x44)
  - Secondary SHT31 for enclosure temperature & humidity (I²C at 0x45)
  - Capacitive soil moisture probe on ADC (GPIO34) with software scaling to percentage
  - ADC for battery (GPIO32) and 5 V rail (GPIO36) readings
- **Automation:**
  - Sleep schedule controlled by real time from Home Assistant
  - Adaptive daytime sleep that doubles the interval when the battery falls below 30%
  - Sends custom `esphome.planter_sleep` events to HA, including a low-battery reason when the extended sleep is used
- **Networking:** Wi-Fi runs at full power with fast connect enabled to maintain a stable link to distant access points.
- **Battery logic:**
  - Nonlinear voltage-to-percentage curve tuned for LiPo cells
  - Voltage smoothing with sliding window average
- **Local components:** External helpers in `my_components/` are loaded via `external_components` for custom sensor handling.

Build and flash:
```bash
esphome run NewSensor.yaml
```

---

## Home Assistant Integration

Data is exposed via ESPHome’s native API:
- Planter probe temperature (`sensor.planter_probe_temperature`)
- Planter probe humidity (`sensor.planter_probe_humidity`)
- Enclosure temperature (`sensor.enclosure_temperature`)
- Enclosure humidity (`sensor.enclosure_humidity`)
- Capacitive soil moisture percent (`sensor.capacitive_soil_moisture`)
- Raw soil moisture voltage (`sensor.soil_moisture_raw`, internal)
- Battery voltage (`sensor.battery_voltage`)
- Battery percent (`sensor.battery_percent`)
- Wi-Fi RSSI (`sensor.planter_wifi_signal`)
- System sleep events (`esphome.planter_sleep`)

These can trigger Home Assistant automations or be displayed in dashboards.

---

## Enclosure

Optional 3D-printed enclosure fits:
- ESP32 Dev board  
- SHT31 sensor (vented section for airflow)  
- 1000 mAh LiPo battery  

Can be mounted near plant beds or outdoor pots. STL files can be created in Fusion 360 or ordered via Fiverr.

---

## I²C Initialization Notes

The SHT31 probes and the capacitive soil sensor share the switched sensor power rail. On boot, the firmware asserts the `sensor_vdd` switch (GPIO17) before the I²C bus becomes active, then immediately runs a second `on_boot` stage for ADC warm-up and manual sensor polls once power is stable. This custom workaround avoids bus lockups from slow probe ramp-up; if you add more devices, keep them behind the same power switch or adjust the boot priorities so VDD settles ahead of any I²C transactions.

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
   esphome run NewSensor.yaml
   ```
4. Add the device to Home Assistant.
5. Optionally print and assemble an enclosure.

---
