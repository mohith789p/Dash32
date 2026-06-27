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
    , _min(0.0f)
    , _max(100.0f)
{
}

void DashGauge::setRange(float min, float max) {
    if (min >= max) {
        DASH_LOG_WARN("Gauge '%s': min >= max, swapping", _title);
        _min = max;
        _max = min;
    } else {
        _min = min;
        _max = max;
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
    int prefix = snprintf(buf, size, "\"value\":");
    if (prefix < 0 || static_cast<size_t>(prefix) >= size) return -1;

    int valLen = _value.toJson(buf + prefix, size - prefix);
    if (valLen < 0) return -1;

    return prefix + valLen;
}

int DashGauge::serializeFull(char* buf, size_t size) const {
    // We need to inject min/max into the full serialization.
    // Build the base, but we handle it manually to include range.
    if (!buf || size < 64) return -1;

    int pos = snprintf(buf, size,
        "{\"id\":%u,\"type\":\"gauge\",\"title\":\"%s\",\"unit\":\"%s\","
        "\"min\":%.2f,\"max\":%.2f,",
        _id, _title, _unit, _min, _max);

    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    int valLen = serializeValue(buf + pos, size - pos);
    if (valLen < 0) return -1;
    pos += valLen;

    if (static_cast<size_t>(pos) + 2 >= size) return -1;
    buf[pos++] = '}';
    buf[pos] = '\0';

    return pos;
}

} // namespace dash
