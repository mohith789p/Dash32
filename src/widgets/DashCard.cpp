/**
 * @file DashCard.cpp
 * @brief Card widget implementation.
 */

#include "DashCard.h"
#include <cstdio>

namespace dash {

DashCard::DashCard(const char* title, const char* unit)
    : DashWidget(title, unit, WidgetType::Card)
{
}

bool DashCard::checkForChange() {
    if (_value.hasChanged()) {
        _value.snapshot();
        _dirty = true;
        return true;
    }
    return false;
}

int DashCard::serializeValue(char* buf, size_t size) const {
    // Output: ,"value":X
    int prefix = snprintf(buf, size, ",\"value\":");
    if (prefix < 0 || static_cast<size_t>(prefix) >= size) return -1;

    int valLen = _value.toJson(buf + prefix, size - prefix);
    if (valLen < 0) return -1;

    return prefix + valLen;
}

} // namespace dash
