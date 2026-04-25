#pragma once
// Desktop stub for freertos/FreeRTOS.h

#include <stdint.h>

#define portMAX_DELAY      UINT32_MAX
#define portTICK_PERIOD_MS 1u
#define pdMS_TO_TICKS(ms)  ((uint32_t)(ms))
#define pdTRUE             1
#define pdFALSE            0
#define pdPASS             1
#define pdFAIL             0
#define configTICK_RATE_HZ 1000

typedef uint32_t TickType_t;
typedef uint32_t UBaseType_t;
typedef int      BaseType_t;
