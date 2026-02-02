// Replacement for Arduino.h

#ifndef _NOARDUINO_H_
#define _NOARDUINO_H_

#include <cstdint>
#include <functional>

#include <String>

#include <algorithm>
#include <cmath>

using std::abs;
using std::isinf;
using std::isnan;
using std::max;
using std::min;
using std::round;

#include "WString.h"

#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_task_wdt.h"

// from esp_hal.h
// set to no ISR_IRAM
//#if CONFIG_ARDUINO_ISR_IRAM
//#define ARDUINO_ISR_ATTR IRAM_ATTR
//#define ARDUINO_ISR_FLAG ESP_INTR_FLAG_IRAM
//#else
#define ARDUINO_ISR_ATTR
#define ARDUINO_ISR_FLAG (0)
//#endif

//forward declaration from freertos/portmacro.h
//void vPortYield(void);
void yield(void);
#define optimistic_yield(u)

#define ESP_REG(addr) *((volatile uint32_t *)(addr))
#define NOP()         asm volatile("nop")

//void yield() __attribute__((weak, alias("__yield")));

//#include "freertos/portmacro.h"

unsigned long ARDUINO_ISR_ATTR micros();
unsigned long ARDUINO_ISR_ATTR millis();
void delay(uint32_t ms);
void ARDUINO_ISR_ATTR delayMicroseconds(uint32_t us);

#endif // _NOARDUINO_H