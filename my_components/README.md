# ESP-IDF Custom ESPHome I²C Component — Delayed Initialization

This guide explains how to override ESPHome’s built-in **I²C component** to delay bus initialization.  
It fixes issues where sensors (like **BME280** or **SHT3xD**) are powered via GPIOs and not yet ready when ESPHome starts scanning the I²C bus.

---

## ⚙️ Purpose

By default, ESPHome initializes the I²C bus early in the boot sequence with:
```cpp
float get_setup_priority() const override { return setup_priority::BUS; }
```

That can cause "communication failed" errors if sensors aren’t powered yet.  
This custom modification changes it to:
```cpp
float get_setup_priority() const override { return setup_priority::DATA; }
```
so I²C initializes **after** power-control GPIOs have turned on.

---

## 🧩 Requirements

- Home Assistant OS or Supervised install  
- ESPHome add-on installed  
- Advanced SSH & Web Terminal add-on installed  
- Temporary ability to disable **Protection Mode**

---

## 🚨 Disable Protection Mode

Home Assistant blocks access to container files by default.  
To copy the internal ESPHome source, disable protection mode:

1. Go to **Settings → Add-ons → Advanced SSH & Web Terminal**
2. Open **Configuration**
3. Turn off **Protection mode**
4. Restart the add-on

You can re-enable it later after completing the modification.

---

## 🪛 Copy the Built-in I²C Component

Use SSH or the Web Terminal to copy the default I²C code from the container into your configuration folder.

```bash
# 1. Enter the ESPHome container
docker exec -it addon_5c5d3e3b_esphome bash

# 2. Inside the container, copy the I²C component to your config
mkdir -p /config/esphome/my_components/i2c
cp -r /esphome/esphome/components/i2c/* /config/esphome/my_components/i2c/
exit
```

This creates a full local copy of ESPHome’s I²C integration that can be modified safely.

---

## ✏️ Modify the Header File

Open the file:
```
/config/esphome/my_components/i2c/i2c_bus_esp_idf.h
```

Find this line:
```cpp
float get_setup_priority() const override { return setup_priority::BUS; }
```

Replace it with:
```cpp
float get_setup_priority() const override { return setup_priority::DATA; }
```

Save the file.  
This single-line change delays the initialization phase of the I²C driver.

---

## 🧠 Update Your YAML Configuration

Tell ESPHome to use the local component instead of the internal one.

```yaml
external_components:
  - source:
      type: local
      path: my_components
```

Example configuration:

```yaml
esphome:
  name: planter-20
  platformio_options:
    board_build.flash_mode: dio

external_components:
  - source:
      type: local
      path: my_components

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  id: bus_a

sensor:
  - platform: sht3xd
    temperature:
      name: "Planter Temperature"
    humidity:
      name: "Planter Humidity"
    address: 0x44
```

---

## ✅ Verify the Override

Run this command to ensure ESPHome is using your modified component:

```bash
esphome compile planter-20.yaml | grep i2c
```

Expected output:
```
/config/esphome/my_components/i2c/
```

If you still see `/esphome/esphome/components/i2c/`, the override didn’t apply—check the `external_components` path.

---

## 🔒 Re-enable Protection Mode

Once your configuration compiles correctly, restore system security:

1. Go to **Settings → Add-ons → Advanced SSH & Web Terminal**
2. Re-enable **Protection mode**
3. Restart the add-on

---

## 🧹 Summary

| Step | Action | Purpose |
|------|---------|----------|
| 1 | Disable protection mode | Allow access to container files |
| 2 | Copy `/esphome/esphome/components/i2c` | Create editable local version |
| 3 | Modify header file | Delay I²C initialization |
| 4 | Add `external_components` block | Use your local module |
| 5 | Compile project | Verify override is active |
| 6 | Re-enable protection mode | Restore security settings |

---

## ⚠️ Notes

- Keep the same folder structure:
  ```
  /config/esphome/my_components/i2c/
  ```
- The local component fully replaces the built-in `i2c` integration.
- Works for both **ESP-IDF** and **Arduino** backends.
- You can modify `i2c_bus_arduino.h` instead if compiling under Arduino.

---

## 📄 License

MIT License — free to use, modify, and distribute with attribution.
