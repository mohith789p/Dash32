/**
 * @file DashText.cpp
 * @brief Text display widget implementation.
 */

#include "DashText.h"
#include "../core/DashTypes.h"
#include "../core/DashDebug.h"
#include <cstdio>
#include <cstring>

namespace dash {

DashText::DashText(const char* title)
    : DashWidget(title, nullptr, WidgetType::Text)
{
    _text[0] = '\0';
    _lastText[0] = '\0';
    // Set lastText to something different to force initial send
    _lastText[0] = 1;
    _lastText[1] = '\0';
}

void DashText::setText(const char* text) {
    safeCopy(_text, text, DASH_TEXT_MAX_LEN);
    markDirty();
}

bool DashText::checkForChange() {
    if (strncmp(_text, _lastText, DASH_TEXT_MAX_LEN) != 0) {
        memcpy(_lastText, _text, DASH_TEXT_MAX_LEN);
        _dirty = true;
        return true;
    }
    return false;
}

int DashText::serializeValue(char* buf, size_t size) const {
    // We need to escape the text for JSON
    // Simple approach: write "value":"escaped_text"
    int pos = snprintf(buf, size, "\"value\":\"");
    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    // Escape text content
    for (size_t i = 0; _text[i] != '\0'; ++i) {
        char c = _text[i];
        if (c == '"' || c == '\\') {
            if (static_cast<size_t>(pos) + 2 >= size - 1) break;
            buf[pos++] = '\\';
            buf[pos++] = c;
        } else if (c == '\n') {
            if (static_cast<size_t>(pos) + 2 >= size - 1) break;
            buf[pos++] = '\\';
            buf[pos++] = 'n';
        } else if (static_cast<unsigned char>(c) >= 0x20) {
            if (static_cast<size_t>(pos) + 1 >= size - 1) break;
            buf[pos++] = c;
        }
    }

    if (static_cast<size_t>(pos) + 1 >= size) return -1;
    buf[pos++] = '"';
    buf[pos] = '\0';
    return pos;
}

} // namespace dash
