| Supported Targets | ESP32-C3 |
| ----------------- | -------- |

# InfluxDB Thermometer

The InfluxDB thermometer is an ESP32C3 Supermini Board with a MCP9808 thermometer.

* If neccesary it opens a configuration on http://192.168.4.1 to set the Wifi credentials (SSID and password) with component elrebo-de/wifi-manager.
* It connects to Wifi with component elrebo-de/wifi-manager.
* It synchronizes time with an NTP server with component elrebo-de/time_sync.
* It starts an I2C bus to connect with the MCP9808 with component elrebo-de/i2c_master.
* It measures the temperature with the MCP9808 with component elrebo-de/i2c_master.
* It sends the temperature to an InfluxDB server with component elrebo-de/InfluxDB-Client-for-ESP-IDF.
* It goes to DeepSleep for 15 minutes with component elrebo-de/deep_sleep.
* If DeepSleep does not work, it waits for 15 minutes and measures the temperature again.

## How to use the program

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target esp32c3`.

### Hardware Required

* An ESP32C3 Supermini board.
* An MCP9808 thermometer board.

### Configure the Project

The parameters for connecting the InfluxDB must be set in file secrets.h:

* Copy secrets_template.h to secrets.h
* provide INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG and INFLUXDB_BUCKET to connect your InfluxDB.

### Build and Flash

Run `idf.py build` to build the project.

Run `idf.py -p PORT flash monitor` to flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]`` or `Ctrl-Option-6`on a Mac.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

((tbc))

```

((tbc))

```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/espressif/esp-idf/issues) on GitHub. We will get back to you soon.
