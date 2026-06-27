/**
 * @file DashStatus.cpp
 * @brief Status indicator widget implementation.
 */

#include "DashStatus.h"
#include "../core/DashTypes.h"
#include "../core/DashDebug.h"
#include <cstdio>
#include <cstring>

namespace dash {

DashStatus::DashStatus(const char* title)
    : DashWidget(title, nullptr, WidgetType::Status)
    , _level(StatusLevel::Info)
    , _lastLevel(StatusLevel::OK) // Force initial dirty
{
    safeCopy(_label, "N/A", DASH_STATUS_LABEL_MAX_LEN);
    _lastLabel[0] = '\0';
}

void DashStatus::setStatus(const char* label, StatusLevel level) {
    safeCopy(_label, label, DASH_STATUS_LABEL_MAX_LEN);
    _level = level;
    markDirty();
}

const char* DashStatus::levelName(StatusLevel level) {
    switch (level) {
        case StatusLevel::OK:      return "ok";
        case StatusLevel::Warning: return "warning";
        case StatusLevel::Error:   return "error";
        case StatusLevel::Info:    return "info";
        default:                   return "info";
    }
}

bool DashStatus::checkForChange() {
    bool changed = (_level != _lastLevel) ||
                   (strncmp(_label, _lastLabel, DASH_STATUS_LABEL_MAX_LEN) != 0);
    if (changed) {
        _lastLevel = _level;
        memcpy(_lastLabel, _label, DASH_STATUS_LABEL_MAX_LEN);
        _dirty = true;
    }
    return changed;
}

int DashStatus::serializeValue(char* buf, size_t size) const {
    return snprintf(buf, size, "\"label\":\"%s\",\"level\":\"%s\"",
                    _label, levelName(_level));
}

} // namespace dash
