/**
 * @file DashLED.h
 * @brief LED indicator widget — shows an on/off state with configurable colors.
 *
 * Usage:
 * @code
 *   bool isOnline = true;
 *   DashLED* led = dashboard.addLED("WiFi", &isOnline);
 *   led->setColors(0x00FF88, 0x444444); // Green on, gray off
 * @endcode
 */

#ifndef DASH_LED_H
#define DASH_LED_H

#include "DashWidget.h"

namespace dash {

class DashLED : public DashWidget {
public:
    /**
     * @brief Construct an LED widget.
     * @param title  Display label.
     * @param state  Pointer to a bool (may be nullptr for manual mode).
     */
    explicit DashLED(const char* title, bool* state = nullptr);

    /**
     * @brief Set the LED on/off colors (RGB hex, e.g. 0x00FF00).
     * @param colorOn  Color when state is true.
     * @param colorOff Color when state is false.
     */
    void setColors(uint32_t colorOn, uint32_t colorOff);

    /** @brief Manually set the LED state. */
    void setState(bool on);

    /** @brief Bind to an external bool. */
    void bindState(bool* ptr);

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;

private:
    bool*    _statePtr;
    bool     _state;
    bool     _lastState;
    bool     _usePointer;
    uint32_t _colorOn;
    uint32_t _colorOff;
};

} // namespace dash

#endif // DASH_LED_H
