/**
 * @file LEDs.ino
 * @brief Demonstrates the use of LED indicator widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Adding default green/grey LEDs with pointer binding
 * - Setting custom colors for LEDs (on and off states)
 * - Using manual mode to toggle LEDs without bound variables
 * - Creating dynamic effects like blinking LEDs
 */

#include <ESP32Dashboard.h>

// WiFi credentials (replace with your own)
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// -----------------------------------------------------------------------------
// 1. Pointer Binding Variables
// -----------------------------------------------------------------------------
bool systemPower  = true;  // Bound to default LED (green / grey)
bool pumpActive   = false; // Bound to custom LED (blue / dark-grey)
bool alarmActive  = false; // Bound to blinking LED (red / dark-grey)

// -----------------------------------------------------------------------------
// 2. Widget Pointers
// -----------------------------------------------------------------------------
dash::DashLED* statusLed = nullptr; // Manual mode LED (orange / grey)

void setup() {
    Serial.begin(115200);

    // Start dashboard services
    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard initialization failed!");
        while (true) { delay(1000); }
    }

    Serial.print("Dashboard available at: http://");
    Serial.println(dashboard.getIPAddress());

    // =========================================================================
    // Adding LEDs and Customizing Colors
    // =========================================================================

    // 1. System Power Indicator (Default Green A400 on, Grey 700 off)
    dashboard.addLED("System Power", &systemPower);

    // 2. Water Pump Indicator (Custom Blue: 0x00B0FF on, Dark Grey: 0x212121 off)
    auto* blueLed = dashboard.addLED("Water Pump", &pumpActive);
    if (blueLed != nullptr) {
        blueLed->setColors(0x00B0FF, 0x212121);
    }

    // 3. Alarm Warning Indicator (Custom Red: 0xFF1744 on, Dark Grey: 0x212121 off)
    auto* redLed = dashboard.addLED("Alarm Triggered", &alarmActive);
    if (redLed != nullptr) {
        redLed->setColors(0xFF1744, 0x212121);
    }

    // 4. Manual Status Indicator (Custom Amber: 0xFFC400 on, Dark Grey: 0x212121 off)
    // No variable is bound to this LED. We must update it manually.
    statusLed = dashboard.addLED("Maintenance Mode");
    if (statusLed != nullptr) {
        statusLed->setColors(0xFFC400, 0x212121);
        statusLed->setState(false); // Initial state
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // 1. Update Pointer-Bound Variables
    // =========================================================================

    // System power remains ON (stable)
    systemPower = true;

    // Pump runs periodically (active for 5 seconds, off for 5 seconds)
    pumpActive = (currentMillis / 5000) % 2 == 0;

    // Alarm blinks when pump is inactive (simulated condition)
    if (!pumpActive) {
        // Toggle alarm every 500ms
        alarmActive = (currentMillis / 500) % 2 == 0;
    } else {
        alarmActive = false; // Safe state
    }

    // =========================================================================
    // 2. Update Manual LED
    // =========================================================================
    static unsigned long lastManualUpdate = 0;
    if (currentMillis - lastManualUpdate >= 3000) {
        lastManualUpdate = currentMillis;

        static bool maintenanceState = false;
        maintenanceState = !maintenanceState;

        if (statusLed != nullptr) {
            statusLed->setState(maintenanceState);
        }
        Serial.printf("[LED Test] Maintenance LED toggled to %s\n", maintenanceState ? "ON" : "OFF");
    }

    // 3. Process dashboard updates
    dashboard.update();

    delay(50);
}
