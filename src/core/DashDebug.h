/**
 * @file DashDebug.h
 * @brief Conditional debug macros for ESP32Dashboard.
 *
 * When DASH_DEBUG is 1, messages are printed to Serial.
 * When DASH_DEBUG is 0, all debug calls compile to nothing (zero overhead).
 */

#ifndef DASH_DEBUG_H
#define DASH_DEBUG_H

#include "DashConfig.h"

#if DASH_DEBUG

#include <Arduino.h>

#define DASH_LOG(fmt, ...)                                              \
    do {                                                                \
        Serial.printf("[DASH] " fmt "\n", ##__VA_ARGS__);               \
    } while (0)

#define DASH_LOG_ERROR(fmt, ...)                                        \
    do {                                                                \
        Serial.printf("[DASH ERROR] " fmt "\n", ##__VA_ARGS__);         \
    } while (0)

#define DASH_LOG_WARN(fmt, ...)                                         \
    do {                                                                \
        Serial.printf("[DASH WARN] " fmt "\n", ##__VA_ARGS__);          \
    } while (0)

#else

#define DASH_LOG(fmt, ...)       ((void)0)
#define DASH_LOG_ERROR(fmt, ...) ((void)0)
#define DASH_LOG_WARN(fmt, ...)  ((void)0)

#endif // DASH_DEBUG

#endif // DASH_DEBUG_H
