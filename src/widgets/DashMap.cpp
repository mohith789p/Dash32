/**
 * @file DashMap.cpp
 * @brief Map widget implementation.
 *
 * Design notes:
 *
 * Dirty flag separation
 *   _dirty      (base class) — set when lat/lon/heading changes.
 *                              Causes a compact delta packet with only
 *                              runtime fields (lat, lon, hdg).
 *   _configDirty              — set when any configuration field changes.
 *                              Causes a full-config packet to be sent.
 *
 * Custom SVG removed
 *   Raw SVG strings are NOT serialized into JSON.  They may contain
 *   characters that break JSON without careful escaping, and the
 *   string is large relative to typical JSON buffer sizes.
 *   All marker graphics are stored in the browser JavaScript.
 *
 * Heading initialization
 *   _lastHeading is initialized to NAN so that any valid heading value
 *   (including 0°) is treated as a change on the first update.
 *
 * Heading wraparound
 *   headingDiff() uses the shortest-angle formula so that a transition
 *   from 359° to 1° is detected as a 2° change, not 358°.
 *
 * Coordinate initialization
 *   _lastLat / _lastLon are initialized to NAN.  std::isnan() detects
 *   the first update and forces an initial dirty state, replacing the
 *   previous magic-value approach (999.0).
 */

#include "DashMap.h"
#include "../core/DashDebug.h"
#include <cstdio>
#include <cmath>
#include <cstring>

namespace dash {

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void DashMap::markConfigDirty() {
    _configDirty = true;
    _dirty = true; // Also mark runtime dirty so the engine picks it up
}

float DashMap::headingDiff(float a, float b) {
    float d = fmod((a - b + 540.0f), 360.0f) - 180.0f;
    return d;
}

float DashMap::computeBearing(double lat1, double lon1,
                              double lat2, double lon2) {
    // Convert to radians
    static constexpr double kDeg2Rad = 3.14159265358979323846 / 180.0;
    static constexpr double kRad2Deg = 180.0 / 3.14159265358979323846;

    double dLon = (lon2 - lon1) * kDeg2Rad;
    double lat1r = lat1 * kDeg2Rad;
    double lat2r = lat2 * kDeg2Rad;

    double x = sin(dLon) * cos(lat2r);
    double y = cos(lat1r) * sin(lat2r) -
               sin(lat1r) * cos(lat2r) * cos(dLon);

    double bearing = atan2(x, y) * kRad2Deg;
    // Normalize to [0, 360)
    bearing = fmod(bearing + 360.0, 360.0);
    return static_cast<float>(bearing);
}

const char* DashMap::themeName(MapTheme t) {
    switch (t) {
        case MapTheme::DarkMatter: return "darkmatter";
        case MapTheme::Positron:   return "positron";
        case MapTheme::Voyager:    return "voyager";
        case MapTheme::Satellite:  return "satellite";
        case MapTheme::Terrain:    return "terrain";
        default:                   return "satellite";
    }
}

const char* DashMap::markerName(MarkerStyle s) {
    switch (s) {
        case MarkerStyle::Circle:     return "circle";
        case MarkerStyle::Pin:        return "pin";
        case MarkerStyle::Car:        return "car";
        case MarkerStyle::Truck:      return "truck";
        case MarkerStyle::Motorcycle: return "motorcycle";
        case MarkerStyle::Bicycle:    return "bicycle";
        case MarkerStyle::Drone:      return "drone";
        case MarkerStyle::Boat:       return "boat";
        case MarkerStyle::Aircraft:   return "aircraft";
        default:                      return "circle";
    }
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

DashMap::DashMap(const char* title, double* lat, double* lon)
    : DashWidget(title, nullptr, WidgetType::Map)
    , _latPtr(lat)
    , _lonPtr(lon)
    , _lat(0.0)
    , _lon(0.0)
    , _lastLat(NAN)     // NAN → force initial dirty on first update
    , _lastLon(NAN)
    , _usePointers(lat != nullptr && lon != nullptr)
    , _headingPtr(nullptr)
    , _heading(0.0f)
    , _lastHeading(NAN) // NAN → force initial dirty on first update
    , _hasHeading(false)
    , _rotationEnabled(true)
    , _autoBearing(false)
    , _prevBearLat(0.0)
    , _prevBearLon(0.0)
    , _prevBearValid(false)
    , _theme(MapTheme::DarkMatter)
    , _markerStyle(MarkerStyle::Circle)
    , _zoom(15)
    , _zoomControls(false)
    , _follow(true)
    , _trail(false)
    , _trailLength(50)
    , _fullscreen(false)
    , _scale(false)
    , _layerSwitcher(false)
    , _headingOffset(0.0f)
    , _markerScale(1.0f)
    , _configDirty(false)
{
    if (_usePointers) {
        _lat = *lat;
        _lon = *lon;
    }
    if (lat == nullptr || lon == nullptr) {
        DASH_LOG_WARN("DashMap '%s': null coordinate pointer(s)", _title);
    }
}

// ---------------------------------------------------------------------------
// Coordinate API
// ---------------------------------------------------------------------------

void DashMap::setCoordinates(double lat, double lon) {
    // Switch to manual mode explicitly
    _usePointers = false;
    _lat = lat;
    _lon = lon;
    _dirty = true;
}

void DashMap::bindCoordinates(double* lat, double* lon) {
    if (!lat || !lon) {
        DASH_LOG_WARN("DashMap '%s': bindCoordinates: null pointer(s)", _title);
        return;
    }
    _latPtr = lat;
    _lonPtr = lon;
    _usePointers = true;
    _lat = *lat;
    _lon = *lon;
    _dirty = true;
}

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------

void DashMap::setTheme(MapTheme theme) {
    if (_theme == theme) return;
    _theme = theme;
    markConfigDirty();
}

// ---------------------------------------------------------------------------
// Zoom
// ---------------------------------------------------------------------------

void DashMap::setZoom(uint8_t zoom) {
    if (zoom < 1)  zoom = 1;
    if (zoom > 19) zoom = 19;
    if (_zoom == zoom) return;
    _zoom = zoom;
    markConfigDirty();
}

void DashMap::setZoomControls(bool enabled) {
    if (_zoomControls == enabled) return;
    _zoomControls = enabled;
    markConfigDirty();
}

// ---------------------------------------------------------------------------
// Follow Mode
// ---------------------------------------------------------------------------

void DashMap::setFollow(bool enabled) {
    if (_follow == enabled) return;
    _follow = enabled;
    markConfigDirty();
}

// ---------------------------------------------------------------------------
// Marker
// ---------------------------------------------------------------------------

void DashMap::setMarker(MarkerStyle style) {
    if (_markerStyle == style) return;
    _markerStyle = style;
    markConfigDirty();
}

void DashMap::setHeadingOffset(float degrees) {
    // Normalize to (-180, 180]
    degrees = fmod(degrees + 180.0f, 360.0f) - 180.0f;
    if (fabsf(_headingOffset - degrees) < 0.01f) return;
    _headingOffset = degrees;
    markConfigDirty();
}

void DashMap::setMarkerScale(float scale) {
    if (scale < 0.25f) scale = 0.25f;
    if (scale > 4.0f)  scale = 4.0f;
    if (fabsf(_markerScale - scale) < 0.001f) return;
    _markerScale = scale;
    markConfigDirty();
}

// ---------------------------------------------------------------------------
// Heading
// ---------------------------------------------------------------------------

void DashMap::bindHeading(float* heading) {
    if (!heading) {
        DASH_LOG_WARN("DashMap '%s': bindHeading: null pointer", _title);
        return;
    }
    _headingPtr  = heading;
    _hasHeading  = true;
    _heading     = *heading;
    _lastHeading = NAN; // Force first delta to include heading
    _dirty = true;
}

void DashMap::setHeading(float heading) {
    // Manual heading; pointer updates suspended
    _headingPtr = nullptr;
    _hasHeading = true;
    // Normalize to [0, 360)
    heading = fmod(heading, 360.0f);
    if (heading < 0.0f) heading += 360.0f;
    _heading    = heading;
    _dirty = true;
}

void DashMap::enableRotation(bool enabled) {
    if (_rotationEnabled == enabled) return;
    _rotationEnabled = enabled;
    markConfigDirty();
}

void DashMap::enableAutoBearing(bool enabled) {
    if (_autoBearing == enabled) return;
    _autoBearing = enabled;
    if (enabled) {
        // Reset bearing state so next coord becomes the baseline
        _prevBearValid = false;
    }
}

// ---------------------------------------------------------------------------
// Trail
// ---------------------------------------------------------------------------

void DashMap::enableTrail(bool enabled) {
    if (_trail == enabled) return;
    _trail = enabled;
    markConfigDirty();
}

void DashMap::setTrailLength(uint16_t length) {
    if (length < 2)   length = 2;
    if (length > 500) length = 500;
    if (_trailLength == length) return;
    _trailLength = length;
    markConfigDirty();
}

// ---------------------------------------------------------------------------
// Controls
// ---------------------------------------------------------------------------

void DashMap::setFullscreen(bool enabled) {
    if (_fullscreen == enabled) return;
    _fullscreen = enabled;
    markConfigDirty();
}

void DashMap::setScale(bool enabled) {
    if (_scale == enabled) return;
    _scale = enabled;
    markConfigDirty();
}

void DashMap::enableLayerSwitcher(bool enabled) {
    if (_layerSwitcher == enabled) return;
    _layerSwitcher = enabled;
    markConfigDirty();
}

// ---------------------------------------------------------------------------
// Change detection
// ---------------------------------------------------------------------------

bool DashMap::checkForChange() {
    // Read from bound pointers
    if (_usePointers && _latPtr && _lonPtr) {
        _lat = *_latPtr;
        _lon = *_lonPtr;
    }
    if (_hasHeading && _headingPtr) {
        float raw = *_headingPtr;
        // Normalize to [0, 360)
        raw = fmod(raw, 360.0f);
        if (raw < 0.0f) raw += 360.0f;
        _heading = raw;
    }

    // Detect coordinate change (treat NAN _lastLat as always-changed)
    bool coordChanged = std::isnan(_lastLat) ||
                        std::isnan(_lastLon) ||
                        (fabs(_lat - _lastLat) > kCoordEpsilon) ||
                        (fabs(_lon - _lastLon) > kCoordEpsilon);

    // Auto-bearing: compute heading from GPS movement when no
    // explicit heading source is active.
    if (_autoBearing && !_hasHeading && coordChanged) {
        if (_prevBearValid) {
            double dLat = _lat - _prevBearLat;
            double dLon = _lon - _prevBearLon;
            double dist = dLat * dLat + dLon * dLon;
            // Only update bearing if movement exceeds epsilon
            if (dist > kBearingEpsilon * kBearingEpsilon) {
                _heading = computeBearing(_prevBearLat, _prevBearLon,
                                          _lat, _lon);
                _hasHeading = true;  // Activate heading for serialization
                _prevBearLat = _lat;
                _prevBearLon = _lon;
            }
        } else {
            // First valid coord — record baseline, no bearing yet
            _prevBearLat   = _lat;
            _prevBearLon   = _lon;
            _prevBearValid = true;
        }
    }

    // Detect heading change using shortest-angle comparison.
    // Also treat NAN _lastHeading (first run) as changed.
    bool headingChanged = false;
    if (_hasHeading) {
        headingChanged = std::isnan(_lastHeading) ||
                         (fabsf(headingDiff(_heading, _lastHeading)) > kHeadingEpsilon);
    }

    bool changed = coordChanged || headingChanged;

    if (changed) {
        _lastLat     = _lat;
        _lastLon     = _lon;
        _lastHeading = _heading;
        _dirty = true;
    }
    return changed;
}

// ---------------------------------------------------------------------------
// Value serialization — compact delta (lat, lon, hdg only)
// ---------------------------------------------------------------------------

int DashMap::serializeValue(char* buf, size_t size) const {
    // Include heading whenever heading tracking is active,
    // regardless of the heading value (preserves heading == 0°).
    if (_hasHeading) {
        return snprintf(buf, size,
            "\"lat\":%.6f,\"lon\":%.6f,\"hdg\":%.1f",
            _lat, _lon, _heading);
    }
    return snprintf(buf, size, "\"lat\":%.6f,\"lon\":%.6f", _lat, _lon);
}

// ---------------------------------------------------------------------------
// Full serialization — config (sent on connect / config change)
// ---------------------------------------------------------------------------

int DashMap::serializeFull(char* buf, size_t size) const {
    if (!buf || size < 128) return -1;

    // Serialize theme and marker as human-readable strings.
    // This makes the JSON self-documenting and decouples the browser
    // from enum integer ordering.
    int pos = snprintf(buf, size,
        "{\"id\":%u,\"type\":\"map\",\"title\":\"%s\","
        "\"theme\":\"%s\",\"zoom\":%u,"
        "\"zoomCtrl\":%s,\"follow\":%s,"
        "\"marker\":\"%s\","
        "\"hdgOff\":%.1f,\"mScale\":%.2f,"
        "\"rotate\":%s,"
        "\"trail\":%s,\"trailLen\":%u,"
        "\"fullscreen\":%s,\"scale\":%s,"
        "\"layers\":%s,",
        _id, _title,
        themeName(_theme),
        static_cast<unsigned>(_zoom),
        _zoomControls  ? "true" : "false",
        _follow        ? "true" : "false",
        markerName(_markerStyle),
        _headingOffset,
        _markerScale,
        _rotationEnabled ? "true" : "false",
        _trail         ? "true" : "false",
        static_cast<unsigned>(_trailLength),
        _fullscreen    ? "true" : "false",
        _scale         ? "true" : "false",
        _layerSwitcher ? "true" : "false");

    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    // Runtime value (current coordinates + optional heading)
    int valLen = serializeValue(buf + pos, size - pos);
    if (valLen < 0) return -1;
    pos += valLen;

    // Close object
    if (static_cast<size_t>(pos) + 2 >= size) return -1;
    buf[pos++] = '}';
    buf[pos]   = '\0';

    return pos;
}

} // namespace dash
