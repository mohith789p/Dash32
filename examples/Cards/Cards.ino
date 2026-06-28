/**
 * @file Cards.ino
 * @brief Demonstrates the use of Card widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Adding cards bound to different pointer types (float, double, int, long, bool)
 * - Adding cards in manual mode (without pointer binding)
 * - Updating manual cards via widget pointer methods
 * - Updating manual cards via dashboard convenience methods
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
// Changes to these variables will be detected automatically by dashboard.update()
// and broadcasted to all connected clients.
float       temperature = 24.5f;   // Float card
double      acVoltage   = 230.15;  // Double card
int         fanSpeed    = 1500;    // Int card
long        pulseCount  = 0L;      // Long card
bool        heaterState = false;   // Bool card (renders as true/false or 1/0)

// -----------------------------------------------------------------------------
// 2. Widget Pointers
// -----------------------------------------------------------------------------
// Pointers to the widgets so we can modify them directly later
dash::DashCard* manualCardPtr = nullptr;

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
    // Adding Pointer-Bound Cards
    // =========================================================================
    // These widgets are bound directly to our global variables. The library
    // checks the value of the pointer on every update() call, meaning you only
    // need to update the variable in loop().
    dashboard.addCard("Temperature", &temperature, "°C");
    dashboard.addCard("AC Line Voltage", &acVoltage, "V");
    dashboard.addCard("Cooling Fan Speed", &fanSpeed, "RPM");
    dashboard.addCard("Pulses Received", &pulseCount, "count");
    dashboard.addCard("Heater Active", &heaterState);

    // =========================================================================
    // Adding Manual-Mode Cards
    // =========================================================================
    // Manual cards do not watch a variable pointer. Instead, they are updated
    // explicitly via method calls.
    
    // Method A: Store the widget pointer and update it via setValue()
    manualCardPtr = dashboard.addCard("System Mode");
    if (manualCardPtr != nullptr) {
        manualCardPtr->setValue("Booting...");
    }

    // Method B: Add the card and update it later by its Title string
    dashboard.addCard("Power Consumption", "kW");
    dashboard.setCardValue("Power Consumption", 1.25f);
}

void loop() {
    // 1. Update pointer-bound variables (simulated data)
    unsigned long currentMillis = millis();

    // Temperature fluctuates slowly
    temperature = 22.0f + 3.0f * sin(currentMillis / 5000.0f);
    
    // Voltage has minor noise
    acVoltage = 230.0 + ((double)(random(-50, 50)) / 100.0);
    
    // Fan speed changes step-wise
    fanSpeed = 1500 + ((currentMillis / 3000) % 3) * 500;
    
    // Pulse count increments
    pulseCount = currentMillis / 100;
    
    // Heater is active if temperature drops below 22.0
    heaterState = (temperature < 22.0f);

    // 2. Update manual cards periodically
    static unsigned long lastManualUpdate = 0;
    if (currentMillis - lastManualUpdate >= 2000) {
        lastManualUpdate = currentMillis;

        // Toggle state text on the manual widget pointer
        static bool stateToggle = false;
        stateToggle = !stateToggle;
        if (manualCardPtr != nullptr) {
            manualCardPtr->setValue(stateToggle ? "RUNNING" : "STANDBY");
        }

        // Update the card by Title string with random power usage
        float powerUsage = 0.5f + (float)random(0, 300) / 100.0f;
        dashboard.setCardValue("Power Consumption", powerUsage);
    }

    // 3. Process dashboard changes (non-blocking)
    // This compares current values against the last broadcast and handles
    // sending updates to clients.
    dashboard.update();

    delay(50); // Small delay to avoid CPU starvation
}
