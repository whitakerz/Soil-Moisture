# Accessing ESPHome Internal Files via SSH

Quick steps for Home Assistant OS

## 1. Install SSH & Web Terminal

1.  Open **Settings → Add-ons → Add-on store**.\
2.  Install **SSH & Web Terminal** (official version).\
3.  Open the **Configuration** tab.\
4.  Disable **Protection mode**.\
5.  Start the add-on and click **OPEN WEB TERMINAL**.

## 2. Enter the ESPHome Container

List running containers:

``` bash
docker ps
```

Identify the container named similar to:

    addon_XXXXXX_esphome

Enter it:

``` bash
docker exec -it addon_XXXXXX_esphome bash
```

## 3. Locate the File

Inside the container:

``` bash
cd /homeassistant/esphome/components/i2c
ls
```

You should see:

    i2c_bus_esp_idf.h

## 4. Copy the File Out to Your Config

Your Home Assistant `/config` directory appears in the container as
`/homeassistant`.

Create a custom component folder:

``` bash
mkdir -p /homeassistant/esphome/my_components/i2c
```

Copy the file:

``` bash
cp /homeassistant/esphome/components/i2c/i2c_bus_esp_idf.h    /homeassistant/esphome/my_components/i2c/i2c_bus_esp_idf.h
```

Verify:

``` bash
ls /homeassistant/esphome/my_components/i2c
```

## 5. Access the File in Home Assistant File Editor

In File Editor, open:

    /config/esphome/my_components/i2c/i2c_bus_esp_idf.h

(`"/homeassistant"` inside the container corresponds to `"/config"` in
File Editor.)

Gracious thanks to ChatGPT for writing this file, use at your own risk