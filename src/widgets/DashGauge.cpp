/**
 * @file DashGauge.cpp
 * @brief Gauge widget implementation.
 */

#include "DashGauge.h"
#include "../core/DashDebug.h"
#include <cstdio>

namespace dash {

DashGauge::DashGauge(const char* title, const char* unit)
    : DashWidget(title, unit, WidgetType::Gauge)
    , _rangeMin(0.0f)
    , _rangeMax(100.0f)
{
}

void DashGauge::setRange(float min, float max) {
    if (min >= max) {
        DASH_LOG_WARN("Gauge '%s': min >= max, swapping", _title);
        _rangeMin = max;
        _rangeMax = min;
    } else {
        _rangeMin = min;
        _rangeMax = max;
    }
    markDirty();
}

bool DashGauge::checkForChange() {
    if (_value.hasChanged()) {
        _value.snapshot();
        _dirty = true;
        return true;
    }
    return false;
}

int DashGauge::serializeValue(char* buf, size_t size) const {
    int prefix = snprintf(buf, size, ",\"value\":");
    if (prefix < 0 || static_cast<size_t>(prefix) >= size) return -1;

    int valLen = _value.toJson(buf + prefix, size - prefix);
    if (valLen < 0) return -1;

    return prefix + valLen;
}

int DashGauge::serializeConfig(char* buf, size_t size) const {
    return snprintf(buf, size, ",\"min\":%.2f,\"max\":%.2f", _rangeMin, _rangeMax);
}

} // namespace dash
