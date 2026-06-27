/**
 * @file ValueHolder.cpp
 * @brief Implementation of the type-erased value container.
 */

#include "ValueHolder.h"
#include "DashDebug.h"
#include <cstdio>
#include <cstring>
#include <cmath>

namespace dash {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ValueHolder::ValueHolder()
    : _type(ValueType::None)
    , _mode(ValueMode::Unbound)
{
    memset(&_ptr, 0, sizeof(_ptr));
    memset(&_current, 0, sizeof(_current));
    memset(&_snapshot, 0, sizeof(_snapshot));
}

// ---------------------------------------------------------------------------
// Pointer binding
// ---------------------------------------------------------------------------

void ValueHolder::bind(float* ptr) {
    if (!ptr) { DASH_LOG_ERROR("bind(float*): null pointer"); return; }
    _type = ValueType::Float;
    _mode = ValueMode::Pointer;
    _ptr.f = ptr;
    _current.f = *ptr;
    _snapshot.f = *ptr - 1.0f; // Force initial dirty
}

void ValueHolder::bind(double* ptr) {
    if (!ptr) { DASH_LOG_ERROR("bind(double*): null pointer"); return; }
    _type = ValueType::Double;
    _mode = ValueMode::Pointer;
    _ptr.d = ptr;
    _current.d = *ptr;
    _snapshot.d = *ptr - 1.0; // Force initial dirty
}

void ValueHolder::bind(int* ptr) {
    if (!ptr) { DASH_LOG_ERROR("bind(int*): null pointer"); return; }
    _type = ValueType::Int;
    _mode = ValueMode::Pointer;
    _ptr.i = ptr;
    _current.i = *ptr;
    _snapshot.i = *ptr - 1; // Force initial dirty
}

void ValueHolder::bind(long* ptr) {
    if (!ptr) { DASH_LOG_ERROR("bind(long*): null pointer"); return; }
    _type = ValueType::Long;
    _mode = ValueMode::Pointer;
    _ptr.l = ptr;
    _current.l = *ptr;
    _snapshot.l = *ptr - 1; // Force initial dirty
}

void ValueHolder::bind(bool* ptr) {
    if (!ptr) { DASH_LOG_ERROR("bind(bool*): null pointer"); return; }
    _type = ValueType::Bool;
    _mode = ValueMode::Pointer;
    _ptr.b = ptr;
    _current.b = *ptr;
    _snapshot.b = !(*ptr); // Force initial dirty
}

// ---------------------------------------------------------------------------
// Manual setters
// ---------------------------------------------------------------------------

void ValueHolder::set(float val) {
    _type = ValueType::Float;
    _mode = ValueMode::Manual;
    _current.f = val;
}

void ValueHolder::set(double val) {
    _type = ValueType::Double;
    _mode = ValueMode::Manual;
    _current.d = val;
}

void ValueHolder::set(int val) {
    _type = ValueType::Int;
    _mode = ValueMode::Manual;
    _current.i = val;
}

void ValueHolder::set(long val) {
    _type = ValueType::Long;
    _mode = ValueMode::Manual;
    _current.l = val;
}

void ValueHolder::set(bool val) {
    _type = ValueType::Bool;
    _mode = ValueMode::Manual;
    _current.b = val;
}

void ValueHolder::set(const char* val) {
    _type = ValueType::StringV;
    _mode = ValueMode::Manual;
    safeCopy(_current.str, val, DASH_STRING_VALUE_MAX_LEN);
}

// ---------------------------------------------------------------------------
// Change detection
// ---------------------------------------------------------------------------

void ValueHolder::readFromPointer() {
    if (_mode != ValueMode::Pointer) return;

    switch (_type) {
        case ValueType::Float:  _current.f = *_ptr.f; break;
        case ValueType::Double: _current.d = *_ptr.d; break;
        case ValueType::Int:    _current.i = *_ptr.i; break;
        case ValueType::Long:   _current.l = *_ptr.l; break;
        case ValueType::Bool:   _current.b = *_ptr.b; break;
        default: break;
    }
}

bool ValueHolder::differs() const {
    switch (_type) {
        case ValueType::Float:
            // Use epsilon comparison for float to avoid noise
            return fabsf(_current.f - _snapshot.f) > 0.001f;
        case ValueType::Double:
            return fabs(_current.d - _snapshot.d) > 0.0001;
        case ValueType::Int:
            return _current.i != _snapshot.i;
        case ValueType::Long:
            return _current.l != _snapshot.l;
        case ValueType::Bool:
            return _current.b != _snapshot.b;
        case ValueType::StringV:
            return strncmp(_current.str, _snapshot.str,
                           DASH_STRING_VALUE_MAX_LEN) != 0;
        default:
            return false;
    }
}

bool ValueHolder::hasChanged() const {
    // For pointer mode, we need a const_cast to read fresh data.
    // This is safe because readFromPointer() only writes to _current.
    if (_mode == ValueMode::Pointer) {
        const_cast<ValueHolder*>(this)->readFromPointer();
    }
    return differs();
}

void ValueHolder::snapshot() {
    switch (_type) {
        case ValueType::Float:   _snapshot.f = _current.f; break;
        case ValueType::Double:  _snapshot.d = _current.d; break;
        case ValueType::Int:     _snapshot.i = _current.i; break;
        case ValueType::Long:    _snapshot.l = _current.l; break;
        case ValueType::Bool:    _snapshot.b = _current.b; break;
        case ValueType::StringV:
            memcpy(_snapshot.str, _current.str, DASH_STRING_VALUE_MAX_LEN);
            break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
// JSON serialization
// ---------------------------------------------------------------------------

/**
 * @brief Escape special JSON characters from src into dst.
 * @return Number of characters written (not including null terminator), or -1 if buffer full.
 */
static int jsonEscapeString(char* dst, size_t dstSize, const char* src) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0'; ++i) {
        char c = src[i];
        const char* escaped = nullptr;
        switch (c) {
            case '"':  escaped = "\\\""; break;
            case '\\': escaped = "\\\\"; break;
            case '\n': escaped = "\\n";  break;
            case '\r': escaped = "\\r";  break;
            case '\t': escaped = "\\t";  break;
            case '\b': escaped = "\\b";  break;
            case '\f': escaped = "\\f";  break;
            default: break;
        }
        if (escaped) {
            size_t len = strlen(escaped);
            if (j + len >= dstSize) return -1;
            memcpy(dst + j, escaped, len);
            j += len;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            // Control characters: encode as \u00XX
            if (j + 6 >= dstSize) return -1;
            snprintf(dst + j, dstSize - j, "\\u%04x", (unsigned)c);
            j += 6;
        } else {
            if (j + 1 >= dstSize) return -1;
            dst[j++] = c;
        }
    }
    if (j < dstSize) dst[j] = '\0';
    return static_cast<int>(j);
}

int ValueHolder::toJson(char* buf, size_t size) const {
    if (!buf || size == 0) return -1;
    if (_mode == ValueMode::Unbound) {
        return snprintf(buf, size, "null");
    }

    // For pointer mode, ensure _current is fresh
    if (_mode == ValueMode::Pointer) {
        const_cast<ValueHolder*>(this)->readFromPointer();
    }

    int written = 0;
    switch (_type) {
        case ValueType::Float:
            written = snprintf(buf, size, "%.2f", _current.f);
            break;
        case ValueType::Double:
            written = snprintf(buf, size, "%.4f", _current.d);
            break;
        case ValueType::Int:
            written = snprintf(buf, size, "%d", _current.i);
            break;
        case ValueType::Long:
            written = snprintf(buf, size, "%ld", _current.l);
            break;
        case ValueType::Bool:
            written = snprintf(buf, size, "%s",
                               _current.b ? "true" : "false");
            break;
        case ValueType::StringV: {
            if (size < 3) return -1; // Need at least: " + char + "
            buf[0] = '"';
            int escaped = jsonEscapeString(buf + 1, size - 2, _current.str);
            if (escaped < 0) return -1;
            size_t pos = 1 + static_cast<size_t>(escaped);
            if (pos + 1 >= size) return -1;
            buf[pos] = '"';
            buf[pos + 1] = '\0';
            written = static_cast<int>(pos + 1);
            break;
        }
        default:
            written = snprintf(buf, size, "null");
            break;
    }
    return written;
}

} // namespace dash
