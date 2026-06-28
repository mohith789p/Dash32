/**
 * @file DashLED.cpp
 * @brief LED indicator widget implementation.
 */

#include "DashLED.h"
#include "../core/DashDebug.h"
#include <cstdio>

namespace dash {

DashLED::DashLED(const char* title, bool* state)
    : DashWidget(title, nullptr, WidgetType::LED)
    , _statePtr(state)
    , _state(false)
    , _lastState(true)  // Force initial dirty
    , _usePointer(state != nullptr)
    , _colorOn(0x00E676)   // Material green A400
    , _colorOff(0x616161)  // Material grey 700
{
    if (_usePointer) {
        _state = *state;
    }
}

void DashLED::setColors(uint32_t colorOn, uint32_t colorOff) {
    _colorOn = colorOn;
    _colorOff = colorOff;
    markDirty();
}

void DashLED::setState(bool on) {
    _usePointer = false;
    _state = on;
    markDirty();
}

void DashLED::bindState(bool* ptr) {
    if (!ptr) { DASH_LOG_ERROR("DashLED::bindState: null pointer"); return; }
    _statePtr = ptr;
    _usePointer = true;
    _state = *ptr;
    _lastState = !_state; // Force dirty
    markDirty();
}

bool DashLED::checkForChange() {
    if (_usePointer && _statePtr) {
        _state = *_statePtr;
    }

    if (_state != _lastState) {
        _lastState = _state;
        _dirty = true;
        return true;
    }
    return false;
}

int DashLED::serializeValue(char* buf, size_t size) const {
    return snprintf(buf, size,
        ",\"value\":%s,\"colorOn\":\"#%06X\",\"colorOff\":\"#%06X\"",
        _state ? "true" : "false",
        _colorOn & 0xFFFFFF,
        _colorOff & 0xFFFFFF);
}
} // namespace dash
