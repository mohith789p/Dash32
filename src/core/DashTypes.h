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
// Map theme enumeration
// ---------------------------------------------------------------------------

/**
 * @enum MapTheme
 * @brief Built-in map tile themes for the map widget.
 */
enum class MapTheme : uint8_t {
    DarkMatter = 0,  ///< CartoDB Dark Matter (default)
    Positron   = 1,  ///< CartoDB Positron (light)
    Voyager    = 2,  ///< CartoDB Voyager
    Satellite  = 3,  ///< Esri World Imagery
    Terrain    = 4   ///< OpenTopoMap
};

// ---------------------------------------------------------------------------
// Marker style enumeration
// ---------------------------------------------------------------------------

/**
 * @enum MarkerStyle
 * @brief Built-in marker styles for the map widget.
 */
enum class MarkerStyle : uint8_t {
    Circle     = 0,  ///< Default circle marker (backward compatible)
    Pin        = 1,  ///< Map pin marker
    Car        = 2,  ///< Car / automobile
    Truck      = 3,  ///< Truck / lorry
    Motorcycle = 4,  ///< Motorcycle
    Bicycle    = 5,  ///< Bicycle
    Drone      = 6,  ///< Drone / quadcopter
    Boat       = 7,  ///< Boat / ship
    Aircraft   = 8   ///< Aircraft / airplane
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
