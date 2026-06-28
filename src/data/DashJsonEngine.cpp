/**
 * @file DashJsonEngine.cpp
 * @brief JSON serialization engine implementation.
 */

#include "DashJsonEngine.h"
#include "../core/DashDebug.h"
#include "../widgets/DashGauge.h"
#include "../widgets/DashLED.h"
#include "../widgets/DashMap.h"
#include <cstdio>
#include <cstring>

namespace dash {

// ---------------------------------------------------------------------------
// Helper: append a single widget
// ---------------------------------------------------------------------------

bool DashJsonEngine::appendWidget(const DashWidget* widget, char* buf,
                                  size_t size, int& pos, bool full,
                                  bool isFirst) const {
    if (!widget) return false;

    // Comma separator
    if (!isFirst) {
        if (static_cast<size_t>(pos) + 1 >= size) return false;
        buf[pos++] = ',';
    }

    int written = 0;
    size_t remaining = size - static_cast<size_t>(pos);

    if (full) {
        // Special handling for widgets that override serializeFull
        if (widget->getType() == WidgetType::Gauge) {
            written = static_cast<const DashGauge*>(widget)
                          ->serializeFull(buf + pos, remaining);
        } else if (widget->getType() == WidgetType::LED) {
            written = static_cast<const DashLED*>(widget)
                          ->serializeFull(buf + pos, remaining);
        } else if (widget->getType() == WidgetType::Map) {
            written = static_cast<const DashMap*>(widget)
                          ->serializeFull(buf + pos, remaining);
        } else {
            written = widget->serializeFull(buf + pos, remaining);
        }
    } else {
        written = widget->serializeDelta(buf + pos, remaining);
    }

    if (written < 0) {
        DASH_LOG_WARN("JSON: widget '%s' serialization overflow",
                      widget->getTitle());
        // Revert the comma
        if (!isFirst) pos--;
        return false;
    }

    pos += written;
    return true;
}

// ---------------------------------------------------------------------------
int DashJsonEngine::serializeConfig(const DashboardSysInfo& sys,
                                    DashWidget* const* widgets, uint8_t count,
                                    char* buf, size_t size) const {
    if (!buf || size < 128 || !widgets) return -1;

    int pos = snprintf(buf, size,
        "{\"type\":\"config\",\"title\":\"%s\",\"sys\":{"
        "\"ip\":\"%s\",\"hostname\":\"%s\",\"http\":%u,\"ws\":%u,"
        "\"clients\":%u,\"rssi\":%d,\"ssid\":\"%s\",\"model\":\"%s\","
        "\"heap\":%u,\"uptime\":%u},\"widgets\":[",
        sys.title, sys.ip, sys.hostname, sys.httpPort, sys.wsPort,
        sys.clients, sys.rssi, sys.ssid, sys.model, sys.heap, sys.uptime);

    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    bool first = true;
    for (uint8_t i = 0; i < count; ++i) {
        if (!widgets[i]) continue;
        if (appendWidget(widgets[i], buf, size, pos, true, first)) {
            first = false;
        }
    }

    // Close array and object
    if (static_cast<size_t>(pos) + 3 >= size) return -1;
    buf[pos++] = ']';
    buf[pos++] = '}';
    buf[pos] = '\0';

    return pos;
}

// ---------------------------------------------------------------------------
// Delta serialization (only changed widgets)
// ---------------------------------------------------------------------------

int DashJsonEngine::serializeDelta(const DashboardSysInfo& sys,
                                   DashWidget* const* widgets, uint8_t count,
                                   char* buf, size_t size) const {
    if (!buf || size < 128 || !widgets) return -1;

    int pos = snprintf(buf, size,
        "{\"type\":\"delta\",\"sys\":{"
        "\"clients\":%u,\"rssi\":%d,\"heap\":%u,\"uptime\":%u},\"widgets\":[",
        sys.clients, sys.rssi, sys.heap, sys.uptime);

    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    bool first = true;
    for (uint8_t i = 0; i < count; ++i) {
        if (!widgets[i] || !widgets[i]->isDirty()) continue;
        if (appendWidget(widgets[i], buf, size, pos, false, first)) {
            first = false;
        }
    }

    if (static_cast<size_t>(pos) + 3 >= size) return -1;
    buf[pos++] = ']';
    buf[pos++] = '}';
    buf[pos] = '\0';

    return pos;
}

// ---------------------------------------------------------------------------
// Full update serialization (periodic resync)
// ---------------------------------------------------------------------------

int DashJsonEngine::serializeFullUpdate(const DashboardSysInfo& sys,
                                        DashWidget* const* widgets,
                                        uint8_t count,
                                        char* buf, size_t size) const {
    if (!buf || size < 128 || !widgets) return -1;

    int pos = snprintf(buf, size,
        "{\"type\":\"update\",\"sys\":{"
        "\"clients\":%u,\"rssi\":%d,\"heap\":%u,\"uptime\":%u},\"widgets\":[",
        sys.clients, sys.rssi, sys.heap, sys.uptime);

    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    bool first = true;
    for (uint8_t i = 0; i < count; ++i) {
        if (!widgets[i]) continue;
        if (appendWidget(widgets[i], buf, size, pos, false, first)) {
            first = false;
        }
    }

    if (static_cast<size_t>(pos) + 3 >= size) return -1;
    buf[pos++] = ']';
    buf[pos++] = '}';
    buf[pos] = '\0';

    return pos;
}

} // namespace dash
