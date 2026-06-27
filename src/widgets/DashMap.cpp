/**
 * @file DashMap.cpp
 * @brief Map widget implementation.
 */

#include "DashMap.h"
#include "../core/DashDebug.h"
#include <cstdio>
#include <cmath>

namespace dash {

DashMap::DashMap(const char* title, double* lat, double* lon)
    : DashWidget(title, nullptr, WidgetType::Map)
    , _latPtr(lat)
    , _lonPtr(lon)
    , _lat(0.0)
    , _lon(0.0)
    , _lastLat(999.0)   // Impossible value → force initial dirty
    , _lastLon(999.0)
    , _usePointers(lat != nullptr && lon != nullptr)
{
    if (_usePointers) {
        _lat = *lat;
        _lon = *lon;
    }
    if (lat == nullptr || lon == nullptr) {
        DASH_LOG_WARN("DashMap '%s': null coordinate pointer(s)", _title);
    }
}

void DashMap::setCoordinates(double lat, double lon) {
    _usePointers = false;
    _lat = lat;
    _lon = lon;
    markDirty();
}

bool DashMap::checkForChange() {
    if (_usePointers && _latPtr && _lonPtr) {
        _lat = *_latPtr;
        _lon = *_lonPtr;
    }

    bool changed = (fabs(_lat - _lastLat) > kCoordEpsilon) ||
                   (fabs(_lon - _lastLon) > kCoordEpsilon);

    if (changed) {
        _lastLat = _lat;
        _lastLon = _lon;
        _dirty = true;
    }
    return changed;
}

int DashMap::serializeValue(char* buf, size_t size) const {
    return snprintf(buf, size, "\"lat\":%.6f,\"lon\":%.6f", _lat, _lon);
}

} // namespace dash
