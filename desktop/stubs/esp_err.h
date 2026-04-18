#pragma once
// Desktop stub for esp_err.h

#include <stdint.h>
#include <cstdio>
#include <cstdlib>

typedef int esp_err_t;

#define ESP_OK              0
#define ESP_FAIL           -1
#define ESP_ERR_INVALID_ARG -2
#define ESP_ERR_TIMEOUT    -3
#define ESP_ERR_NOT_FOUND  -4
#define ESP_ERR_NO_MEM     -5

#define ESP_ERROR_CHECK(x) do { \
    esp_err_t _err = (x); \
    if (_err != ESP_OK) { \
        fprintf(stderr, "[ESP_ERROR_CHECK] FAILED (err=0x%x) at %s:%d\n" \
                        "Try running on ESP32 for full diagnostic output.\n", \
                        _err, __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)
