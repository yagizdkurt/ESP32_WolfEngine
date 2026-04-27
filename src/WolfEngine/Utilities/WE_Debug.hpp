#pragma once

#include <stdlib.h>

// =============================================================
//  WE_Debug.hpp
//  Centralized debug interface for WolfEngine.
//
//  Zero-cost contract:
//  - When WE_DEBUG_ENABLED == 0, WE_LOGI/WE_LOGW/WE_LOGE and
//    WE_ASSERT are removed at preprocessing time.
//  - Their arguments are not evaluated.
//  - Their string literals are not emitted into the binary.
//  - No backend function calls are generated.
//
//  WE_PANIC is always available:
//  - Debug: forwards to the selected backend, which may log
//    and then abort.
//  - Release: aborts immediately with no logging requirement.
//
//  Backend selection:
//  - Built-in backend:
//      WE_DEBUG_BACKEND_ESP
//  - Optional custom backend:
//      WE_DEBUG_BACKEND_CUSTOM
//  - A custom microcontroller backend should define
//    WE_DEBUG_BACKEND_CUSTOM and provide a .cpp file that implements:
//      WE_DEBUG::Backend::LogI
//      WE_DEBUG::Backend::LogW
//      WE_DEBUG::Backend::LogE
//      WE_DEBUG::Backend::Panic
//      WE_DEBUG::Backend::AssertFail
//    No changes to this header are required for that path.
// =============================================================

#ifndef WE_DEBUG_ENABLED
    #ifdef NDEBUG
        #define WE_DEBUG_ENABLED 0
    #else
        #define WE_DEBUG_ENABLED 1
    #endif
#endif

#if WE_DEBUG_ENABLED && !defined(WE_DEBUG_BACKEND_ESP) && !defined(WE_DEBUG_BACKEND_CUSTOM)
    #undef WE_DEBUG_ENABLED
    #define WE_DEBUG_ENABLED 0
#endif

namespace WE_DEBUG {
    namespace Backend {
#if WE_DEBUG_ENABLED
        void LogI(const char* tag, const char* fmt, ...);
        void LogW(const char* tag, const char* fmt, ...);
        void LogE(const char* tag, const char* fmt, ...);
        [[noreturn]] void Panic(const char* msg);
        [[noreturn]] void AssertFail(const char* expr,
                                     const char* msg,
                                     const char* file,
                                     int line);
#endif
    }
}

#if WE_DEBUG_ENABLED
    #define WE_LOGI(tag, fmt, ...) WE_DEBUG::Backend::LogI(tag, fmt, ##__VA_ARGS__)
    #define WE_LOGW(tag, fmt, ...) WE_DEBUG::Backend::LogW(tag, fmt, ##__VA_ARGS__)
    #define WE_LOGE(tag, fmt, ...) WE_DEBUG::Backend::LogE(tag, fmt, ##__VA_ARGS__)
    #define WE_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                WE_DEBUG::Backend::AssertFail(#condition, message, __FILE__, __LINE__); \
            } \
        } while (0)
    #define WE_PANIC(message) WE_DEBUG::Backend::Panic(message)
#else
    #define WE_LOGI(tag, fmt, ...) ((void)0)
    #define WE_LOGW(tag, fmt, ...) ((void)0)
    #define WE_LOGE(tag, fmt, ...) ((void)0)
    #define WE_ASSERT(condition, message) ((void)0)
    #define WE_PANIC(message) abort()
#endif
