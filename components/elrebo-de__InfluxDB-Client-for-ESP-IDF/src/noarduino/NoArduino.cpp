// Replacement for Arduino code

#include <NoArduino.h>

// from esp_hal_misc.c modified!
void yield() {
  vPortYield();
}

//void yield() __attribute__((weak, alias("__yield")));

unsigned long ARDUINO_ISR_ATTR micros() {
  return (unsigned long)(esp_timer_get_time());
}

unsigned long ARDUINO_ISR_ATTR millis() {
  return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

void delay(uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

void ARDUINO_ISR_ATTR delayMicroseconds(uint32_t us) {
  uint64_t m = (uint64_t)esp_timer_get_time();
  if (us) {
    uint64_t e = (m + us);
    if (m > e) {  //overflow
      while ((uint64_t)esp_timer_get_time() > e) {
        NOP();
      }
    }
    while ((uint64_t)esp_timer_get_time() < e) {
      NOP();
    }
  }
}
