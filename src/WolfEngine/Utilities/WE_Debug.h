// No #pragma once — intentional

#undef DebugLog
#undef DebugErr
#ifdef MODULE_DEBUG_ENABLED
#include "esp_log.h"
#define DebugLog(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define DebugErr(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#else
#define DebugLog(tag, format, ...) do {} while(0)
#define DebugErr(tag, format, ...) do {} while(0)
#endif