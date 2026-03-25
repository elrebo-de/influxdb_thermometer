#include <string>

/* InfluxDB thermometer */

static const char *tag = "InfluxDB Thermometer";

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

//#include "driver/rtc_io.h"

#include "onboard_led.hpp"
#include "wifi_manager.hpp"
#include "time_sync.hpp"
#include "i2c_master.hpp"
#include "deep_sleep.hpp"
RTC_DATA_ATTR int bootCount = 0;
#define DEVICE "ESP32"
#include "InfluxDbClient.h"
#include "http_config_server.hpp"

OnBoardLed *onBoardLed;

static void timeTask(void *pc){
    TimeSync* timeSync = &TimeSync::getInstance();

    // Synchronize time
    timeSync->obtainTime();

    while (1)
    {
        timeSync->printCalendar();
        vTaskDelay(pdMS_TO_TICKS(timeSync->getSyncIntervalMs())); // Print calendar every 5 minutes
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(tag, "Start");

    onBoardLed = new OnBoardLed(
        std::string("onBoardLed"),
        (gpio_num_t) 8,
        (uint8_t) 0, // activeLevel
        500);
    onBoardLed->setLedState(true);
    onBoardLed->show();

    Wifi wifi( std::string("WifiManager"), // tag for ESP_LOGx
               std::string("ESP32"),       // ssid_prefix for configuration access point
               std::string("de-DE")        // language for configuration access point
             );

    // get ConfigServer instance
    HttpConfigServer* configServer = &HttpConfigServer::getInstance();

    configServer->initialize( std::string("**** HttpServerConfig ****"), // tag for ESP_LOGx
                              wifi.GetIpAddress(), // IP address
                              std::string("config") // nvs namespace
                            );

    // add config parameter "influxdbUrl"
    configServer->addStringParameter( "01_Url",
                                     "InfluxDB server url. Don't use localhost, always server name or ip address. E.g. http://192.168.1.48:8086 (In InfluxDB 2 UI -> Load Data -> Client Libraries)"
                                   );
    // add config parameter "influxdbToken"
    configServer->addStringParameter( "02_Token",
                                     "InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)"
                                   );
    // add config parameter "influxdbOrg"
    configServer->addStringParameter( "03_Org",
                                     "InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )"
                                   );
    // add config parameter "influxdbBucket"
    configServer->addStringParameter( "04_Bucket",
                                     "InfluxDB 2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)"
                                   );

    // isConfigured()
    // connect to nvs_flash
    // if config parameters are already set in nvs_flash -> return true
    // else start http server to set config parameter values and return false
    while (!configServer->isConfigured()) {
        ESP_LOGI(tag, "HttpConfigServer is not yet configured, go to http://%s/config to enter configuration parameter values", wifi.GetIpAddress().c_str());
        vTaskDelay(pdMS_TO_TICKS(10000)); // delay 10 seconds
    }

    // get config parameter values
    ESP_LOGI(tag, "get config parameter values from HttpConfigServer");
    esp_err_t ret;
    std::string influxdbUrl = configServer->getStringParameterValue("01_Url", &ret);
    std::string influxdbToken = configServer->getStringParameterValue("02_Token", &ret);
    std::string influxdbOrg = configServer->getStringParameterValue("03_Org", &ret);
    std::string influxdbBucket = configServer->getStringParameterValue("04_Bucket", &ret);

    ESP_LOGI(tag, "influxdbUrl: %s", influxdbUrl.c_str());
    ESP_LOGI(tag, "influxdbToken: %s", influxdbToken.c_str());
    ESP_LOGI(tag, "influxdbOrg: %s", influxdbOrg.c_str());
    ESP_LOGI(tag, "influxdbBucket: %s", influxdbBucket.c_str());

    /* Initialize TimeSync class */
    ESP_LOGI(tag, "TimeSync");
    TimeSync* timeSync = &timeSync->getInstance();
    timeSync->initializeSntp();
    timeSync->setTimezone(std::string("CET"));
    timeSync->setSyncIntervalMs(300000);

    xTaskCreate(timeTask, "time_task", 4096, NULL, 5, NULL);

    while(!timeSync->isSynchronized()) {
        ESP_LOGI(tag, "time is not yet synchronized");
        vTaskDelay(pdMS_TO_TICKS(1000)); // delay 1 second
    }

    /* First configure the I2C Master Bus */
    ESP_LOGI(tag, "I2cMaster");
    I2cMaster i2c(
		  std::string("I2C Master Bus"), // tag
		  (i2c_port_num_t) 0, // I2C_MASTER_NUM, // i2cPort
	    (gpio_num_t) 4, // sclPin
	    (gpio_num_t) 3 // sdaPin
    );

    // MCP9808
    // add the thermometer device to the I2Cmaster bus
    ESP_LOGI(tag, "I2cDevice");
    I2cDevice thermometerDevice(
        std::string("Thermometer Device"), // tag
        std::string("Thermometer"), // deviceName
        (i2c_addr_bit_len_t) I2C_ADDR_BIT_LEN_7, // devAddrLength
        (uint16_t) 0x18, // deviceAddress (without R/W bit)
        (uint32_t) 50000 // sclSpeedHz
        );
    i2c_master_dev_handle_t thermometerHandle = i2c.AddDevice(&thermometerDevice);

    float temperature = 0.0;
    uint8_t set_resolution_buffer[2] = {0x08, 0x03};
    uint8_t request_temperature_buffer = 0x05;
    uint8_t temperature_buffer[2];

    //mcp9808
    // Set resolution to 0.0625°C
    ESP_ERROR_CHECK(i2c_master_transmit(thermometerHandle, set_resolution_buffer, 2, -1));

    /* Initialize DeepSleep class */
    ESP_LOGI(tag, "DeepSleep");
    DeepSleep deepSleep(
		std::string("DeepSleep"), // tag
		&bootCount // Address of int bootCount in RTC_DATA
    );

    ESP_LOGI(tag, "Start InfluxDB client");
    // InfluxDB client instance
    InfluxDBClient client;
    client.setConnectionParams(influxdbUrl.c_str(), influxdbOrg.c_str(), influxdbBucket.c_str(), influxdbToken.c_str());

    while(!client.validateConnection()) {
        ESP_LOGI(tag, "InfluxDB connection failed: %s\n", client.getLastErrorMessage().c_str());
        ESP_LOGI(tag, "wait 1 sec. for InfluxDB client connection");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Data point
    Point sensor("temperature");

    // Add constant tags - only once
    sensor.addTag("ip_address", wifi.GetIpAddress().c_str());

    char devAddrBuffer[10];
    sprintf(devAddrBuffer, "0x%2x", thermometerDevice.GetConfig().device_address);
    sensor.addTag("device_address", devAddrBuffer);

    // Check server connection
    if (client.validateConnection()) {
        ESP_LOGI(tag, "connected to InfluxDB: %s", client.getServerUrl().c_str());
    } else {
        ESP_LOGI(tag, "InfluxDB connection failed: %s", client.getLastErrorMessage().c_str());
    }

    while(1) {
        // Read the temperature from MCP9808
        ESP_ERROR_CHECK(i2c_master_transmit_receive(thermometerHandle, &request_temperature_buffer, 1, temperature_buffer, 2, -1));
        // wait a moment and read temerature again, because the first measurement is always wrong
        vTaskDelay(1000 / portTICK_PERIOD_MS); // wait for 1 second
        // Read the temperature from MCP9808
        ESP_ERROR_CHECK(i2c_master_transmit_receive(thermometerHandle, &request_temperature_buffer, 1, temperature_buffer, 2, -1));

        temperature_buffer[0] = temperature_buffer[0] & 0x1F; //Clear flag bits
        if ((temperature_buffer[0] & 0x10) == 0x10){ //TA < 0°C
            temperature_buffer[0] = temperature_buffer[0] & 0x0F; //Clear SIGN
            temperature = 256. - (temperature_buffer[0] * 16. + temperature_buffer[1] / 16.);
        } else //TA > 0°C
        temperature = (temperature_buffer[0] * 16. + temperature_buffer[1] / 16.);
        //Temperature = Ambient Temperature (°C)

        char buffer[1000];
        sprintf(buffer, "%8.1f°C", temperature);

        // Store measured value into point
        sensor.clearFields();
        // Report RSSI of currently connected network
        sensor.addField("temperature", temperature);
        // Print what are we exactly writing
        ESP_LOGI(tag, "writing: %s", client.pointToLineProtocol(sensor).c_str());
        // If no Wifi signal, try to reconnect it
        if (!wifi.IsConnected()) {
            ESP_LOGI(tag, "Wifi connection lost");
            wifi.RestartStation();
            // Check server connection
            if (client.validateConnection()) {
              ESP_LOGI(tag, "(Re-)Connected to InfluxDB: %s", client.getServerUrl().c_str());
            } else {
              ESP_LOGI(tag, "InfluxDB connection failed: %s", client.getLastErrorMessage().c_str());
            }
        }
        // Write point
        if (!client.writePoint(sensor)) {
          ESP_LOGI(tag, "InfluxDB write failed: %s", client.getLastErrorMessage().c_str());
        }

        onBoardLed->setLedState(false);
        onBoardLed->show();

        bool rc;

        ESP_LOGI(tag, "EnableTimerWakeup");
        ESP_ERROR_CHECK(deepSleep.EnableTimerWakeup(15, "min"));  // enable wake up after 15 minutes sleep time
        ESP_LOGI(tag, "GoToDeepSleep");
        rc = deepSleep.GoToDeepSleep(); // go to deep sleep

        // this statement will not be reached, if GoToDeepSleep is working
        ESP_LOGI(tag, "GoToDeepSleep did not work! rc=%u", rc);

        vTaskDelay(pdMS_TO_TICKS(15 * 60 * 1000)); // wait for 15 minutes
    }
}
  
