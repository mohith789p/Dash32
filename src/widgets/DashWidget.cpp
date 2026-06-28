/**
 * @file DashWidget.cpp
 * @brief Implementation of the abstract widget base class.
 */

#include "DashWidget.h"
#include "../core/DashDebug.h"
#include <cstdio>
#include <cstring>

namespace dash {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

DashWidget::DashWidget(const char* title, const char* unit, WidgetType type)
    : _type(type)
    , _id(0)
    , _dirty(true) // Always dirty on creation to force initial send
{
    safeCopy(_title, title, DASH_TITLE_MAX_LEN);
    safeCopy(_unit, unit, DASH_UNIT_MAX_LEN);

    if (title && strlen(title) >= DASH_TITLE_MAX_LEN) {
        DASH_LOG_WARN("Title truncated: '%s'", _title);
    }
}

// ---------------------------------------------------------------------------
// Type name mapping
// ---------------------------------------------------------------------------

const char* DashWidget::typeName() const {
    switch (_type) {
        case WidgetType::Card:    return "card";
        case WidgetType::Gauge:   return "gauge";
        case WidgetType::Map:     return "map";
        case WidgetType::LED:     return "led";
        case WidgetType::Text:    return "text";
        case WidgetType::Status:  return "status";
        case WidgetType::Image:   return "image";
        case WidgetType::Video:   return "video";
        case WidgetType::Chart:   return "chart";
        case WidgetType::Compass: return "compass";
        default:                  return "unknown";
    }
}

// ---------------------------------------------------------------------------
// JSON serialization helpers
// ---------------------------------------------------------------------------

/**
 * Escape and write a JSON string (with quotes) into buf.
 * Returns chars written or -1 on overflow.
 */
static int writeJsonString(char* buf, size_t size, const char* str) {
    if (size < 3) return -1;

    size_t pos = 0;
    buf[pos++] = '"';

    for (size_t i = 0; str[i] != '\0'; ++i) {
        char c = str[i];
        if (c == '"' || c == '\\') {
            if (pos + 2 >= size - 1) return -1;
            buf[pos++] = '\\';
            buf[pos++] = c;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            // Skip control characters in widget metadata
            continue;
        } else {
            if (pos + 1 >= size - 1) return -1;
            buf[pos++] = c;
        }
    }

    if (pos + 1 >= size) return -1;
    buf[pos++] = '"';
    buf[pos] = '\0';
    return static_cast<int>(pos);
}

// ---------------------------------------------------------------------------
// Full serialization
// ---------------------------------------------------------------------------

int DashWidget::serializeFull(char* buf, size_t size) const {
    if (!buf || size < 32) return -1;

    // Build: {"id":N,"type":"xxx","title":"xxx","unit":"xxx","value":...}
    int pos = snprintf(buf, size,
                       "{\"id\":%u,\"type\":\"%s\",\"title\":",
                       _id, typeName());
    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    // Title (escaped)
    int titleLen = writeJsonString(buf + pos, size - pos, _title);
    if (titleLen < 0) return -1;
    pos += titleLen;

    // Unit
    int unitPart = snprintf(buf + pos, size - pos, ",\"unit\":");
    if (unitPart < 0) return -1;
    pos += unitPart;

    int unitLen = writeJsonString(buf + pos, size - pos, _unit);
    if (unitLen < 0) return -1;
    pos += unitLen;

    // Comma before value
    if (static_cast<size_t>(pos) + 1 >= size) return -1;
    buf[pos++] = ',';

    // Value (delegated to subclass)
    int valLen = serializeValue(buf + pos, size - pos);
    if (valLen < 0) return -1;
    pos += valLen;

    // Closing brace
    if (static_cast<size_t>(pos) + 2 >= size) return -1;
    buf[pos++] = '}';
    buf[pos] = '\0';

    return pos;
}

// ---------------------------------------------------------------------------
// Delta serialization
// ---------------------------------------------------------------------------

int DashWidget::serializeDelta(char* buf, size_t size) const {
    if (!buf || size < 16) return -1;

    int pos = snprintf(buf, size, "{\"id\":%u,", _id);
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
