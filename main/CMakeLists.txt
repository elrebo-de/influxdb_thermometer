idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES "driver"
                                "esp_driver_gpio"
                                "esp_timer"
                                "esp_event"
                                # WIFI_ACTIVE
                                "elrebo-de__wifi_manager"
                                "78__esp-wifi-connect"
                                "nvs_flash"
                                # LED_ACTIVE
                                "elrebo-de__onboard_led"
                                "espressif__led_strip"
                                # I2C_ACTIVE
                                "elrebo-de__i2c_master"
                                "esp_driver_i2c"
                                # HTTP_ACTIVE
                                "stream"
                                # DEEP_SLEEP_ACTIVE
                                "elrebo-de__deep_sleep"
                                # INFLUXDB_CLIENT_ACTIVE
                                "elrebo-de__InfluxDB-Client-for-ESP-IDF"
                                "sachin42__wstring"
                                "sachin42__httpclient"
                                "mbedtls"
                                )
