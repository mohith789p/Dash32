/**
 * @file DashTypes.h
 * @brief Shared type definitions and enumerations for ESP32Dashboard.
 */

#ifndef DASH_TYPES_H
#define DASH_TYPES_H

#include <cstdint>
#include <cstddef>

namespace dash {

// ---------------------------------------------------------------------------
// Widget type enumeration
// ---------------------------------------------------------------------------

/**
 * @enum WidgetType
 * @brief Identifies the kind of widget for serialization and frontend rendering.
 */
enum class WidgetType : uint8_t {
    Card    = 0,
    Gauge   = 1,
    Map     = 2,
    LED     = 3,
    Text    = 4,
    Status  = 5,
    // Reserved for future use
    Chart   = 10,
    Compass = 11
};

// ---------------------------------------------------------------------------
// Value type enumeration
// ---------------------------------------------------------------------------

/**
 * @enum ValueType
 * @brief Identifies the underlying data type held by a ValueHolder.
 */
enum class ValueType : uint8_t {
    None     = 0,
    Float    = 1,
    Double   = 2,
    Int      = 3,
    Long     = 4,
    Bool     = 5,
    StringV  = 6   // Fixed char buffer (not Arduino String)
};

// ---------------------------------------------------------------------------
// Value mode enumeration
// ---------------------------------------------------------------------------

/**
 * @enum ValueMode
 * @brief Whether the value is read from a pointer (auto) or set manually.
 */
enum class ValueMode : uint8_t {
    Unbound  = 0,  ///< No value source assigned
    Pointer  = 1,  ///< Value read from an external pointer
    Manual   = 2   ///< Value set explicitly via API
};

// ---------------------------------------------------------------------------
// Status level
// ---------------------------------------------------------------------------

/**
 * @enum StatusLevel
 * @brief Severity level for DashStatus widgets.
 */
enum class StatusLevel : uint8_t {
    OK      = 0,
    Warning = 1,
    Error   = 2,
    Info    = 3
};

// ---------------------------------------------------------------------------
// Utility: safe string copy
// ---------------------------------------------------------------------------

/**
 * @brief Copy a C-string into a fixed buffer with guaranteed null termination.
 *
 * @param dst   Destination buffer.
 * @param src   Source string (may be nullptr → clears dst).
 * @param size  Size of the destination buffer.
 */
inline void safeCopy(char* dst, const char* src, size_t size) {
    if (size == 0) return;
    if (src == nullptr) {
        dst[0] = '\0';
        return;
    }
    size_t i = 0;
    for (; i < size - 1 && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

} // namespace dash

#endif // DASH_TYPES_H
