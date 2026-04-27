#include "WolfEngine/Utilities/WE_Debug.hpp"

#if WE_DEBUG_ENABLED && defined(WE_DEBUG_BACKEND_ESP)

#include "esp_log.h"
#include <stdarg.h>
#include <stdlib.h>

namespace WE_DEBUG {
namespace Backend {

void LogI(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    esp_log_writev(ESP_LOG_INFO, tag, fmt, args);
    va_end(args);
}

void LogW(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    esp_log_writev(ESP_LOG_WARN, tag, fmt, args);
    va_end(args);
}

void LogE(const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    esp_log_writev(ESP_LOG_ERROR, tag, fmt, args);
    va_end(args);
}

[[noreturn]] void Panic(const char* msg) {
    ESP_LOGE("PANIC", "%s", msg);
    abort();
}

[[noreturn]] void AssertFail(const char* expr,
                             const char* msg,
                             const char* file,
                             int line) {
    ESP_LOGE("ASSERT", "%s:%d: assertion failed: %s (%s)", file, line, expr, msg);
    abort();
}

} // namespace Backend
} // namespace WE_DEBUG

#endif
