/**
 * @file Gauges.ino
 * @brief Demonstrates the use of Gauge widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Adding gauges bound to variables (float and int)
 * - Setting min/max ranges for gauges
 * - Creating gauges in manual mode
 * - Demonstrating out-of-range behavior (underflow and overflow)
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
float fuelLevel = 100.0f;  // Float gauge (bound, 0% to 100%)
int   engineRpm = 0;       // Int gauge (bound, 0 to 8000 RPM)

// -----------------------------------------------------------------------------
// 2. Widget Pointers
// -----------------------------------------------------------------------------
dash::DashGauge* fuelGauge = nullptr;
dash::DashGauge* rpmGauge  = nullptr;
dash::DashGauge* tempGauge = nullptr; // Manual gauge (with negative range support)

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
    // Adding Gauges and Setting Ranges
    // =========================================================================
    
    // 1. Fuel Gauge (Float pointer-bound, range 0 to 100)
    fuelGauge = dashboard.addGauge("Fuel Level", &fuelLevel, "%");
    if (fuelGauge != nullptr) {
        fuelGauge->setRange(0.0f, 100.0f); // Default is 0 to 100, but good to set explicitly
    }

    // 2. RPM Gauge (Int pointer-bound, range 0 to 8000)
    rpmGauge = dashboard.addGauge("Engine Speed", &engineRpm, "RPM");
    if (rpmGauge != nullptr) {
        rpmGauge->setRange(0.0f, 8000.0f);
    }

    // 3. Temperature Gauge (Manual-mode, range -20 to 120 °C)
    // Useful for showing that gauges can handle negative limits
    tempGauge = dashboard.addGauge("Coolant Temp", "°C");
    if (tempGauge != nullptr) {
        tempGauge->setRange(-20.0f, 120.0f);
        tempGauge->setValue(20.0f); // Start value
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // 1. Update Pointer-Bound Variables
    // =========================================================================

    // Simulate fuel level depletion (decreases over time, then resets)
    fuelLevel = 100.0f - (float)((currentMillis / 1000) % 100);

    // Simulate engine acceleration (RPM sweeps up and down)
    engineRpm = 1000 + (int)(3500.0f + 3500.0f * sin(currentMillis / 5000.0f));

    // =========================================================================
    // 2. Update Manual Gauges & Validate Ranges
    // =========================================================================
    static unsigned long lastUpdate = 0;
    if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis;

        // Simulate coolant temperature starting cold and climbing, then spiking
        static int cycleStep = 0;
        cycleStep++;

        float simulatedTemp = 20.0f;
        switch (cycleStep % 6) {
            case 0:
                simulatedTemp = -30.0f; // Underflow test: should cap at -20 on frontend
                Serial.println("[Gauge Test] Sending -30°C (Underflow, should clamp to -20)");
                break;
            case 1:
                simulatedTemp = 0.0f;
                break;
            case 2:
                simulatedTemp = 45.5f;
                break;
            case 3:
                simulatedTemp = 90.0f;
                break;
            case 4:
                simulatedTemp = 115.0f;
                break;
            case 5:
                simulatedTemp = 150.0f; // Overflow test: should cap at 120 on frontend
                Serial.println("[Gauge Test] Sending 150°C (Overflow, should clamp to 120)");
                break;
        }

        if (tempGauge != nullptr) {
            tempGauge->setValue(simulatedTemp);
        }
    }

    // 3. Process dashboard changes
    dashboard.update();

    delay(50);
}
