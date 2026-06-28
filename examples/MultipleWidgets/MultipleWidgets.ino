/**
 * @file MultipleWidgets.ino
 * @brief Demonstrates handling multiple widgets and managing limits in ESP32Dashboard.
 *
 * This example covers:
 * - Adding many widgets and organizing them
 * - Checking widget registration status (always verifying returned pointers)
 * - Exceeding the maximum widget limit (graceful failure via nullptr)
 * - How to override compile-time configuration limits like DASH_MAX_WIDGETS
 */

// =============================================================================
// Override Configuration Options BEFORE Including the Library
// =============================================================================
// By default, DASH_MAX_WIDGETS is set to 20 to conserve RAM.
// Let's override it to 30 so we can register more widgets.
#define DASH_MAX_WIDGETS 30

// We can also increase the JSON buffer size to accommodate more widgets in 
// a single full-sync transmission.
#define DASH_JSON_BUFFER_SIZE 4096

#include <ESP32Dashboard.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// We will register a large number of cards to verify the custom limit works.
const int NUM_CARDS = 25;
dash::DashCard* cards[NUM_CARDS];

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Start dashboard services
    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard failed to start!");
        while (true) delay(1000);
    }

    Serial.print("Dashboard: http://");
    Serial.println(dashboard.getIPAddress());

    // =========================================================================
    // 1. Safe Widget Registration
    // =========================================================================
    // When adding many widgets, it is essential to check if the returned pointer
    // is nullptr. If the dashboard is full, addCard/addGauge will return nullptr
    // instead of crashing.
    
    Serial.println("Registering cards...");
    for (int i = 0; i < NUM_CARDS; ++i) {
        char title[16];
        snprintf(title, sizeof(title), "Sensor #%d", i + 1);

        // Add card widget
        cards[i] = dashboard.addCard(title, "V");

        // Always check if registration succeeded
        if (cards[i] == nullptr) {
            Serial.printf("[FAIL] Could not register widget %d. Array is full.\n", i + 1);
        } else {
            // Set initial manual value
            cards[i]->setValue(0.0f);
        }
    }

    // =========================================================================
    // 2. Proving registration rejection above limit
    // =========================================================================
    // Our custom DASH_MAX_WIDGETS is 30.
    // We registered 25 cards. Let's register 5 more status widgets.
    Serial.println("Registering status badges...");
    for (int i = 0; i < 5; ++i) {
        char title[16];
        snprintf(title, sizeof(title), "Status %d", i + 1);
        auto* status = dashboard.addStatus(title);
        if (status) {
            status->setStatus("OK", StatusLevel::OK);
        } else {
            Serial.printf("[FAIL] Could not register status %d.\n", i + 1);
        }
    }

    // Total registered widgets is now 30.
    // The next registration attempt MUST fail by returning nullptr.
    Serial.println("Attempting to register 31st widget (should fail)...");
    auto* excessCard = dashboard.addCard("Excess Card");
    if (excessCard == nullptr) {
        Serial.println("[SUCCESS] 31st widget rejected correctly (returned nullptr)!");
    } else {
        Serial.println("[ALERT] 31st widget accepted! Check DASH_MAX_WIDGETS override.");
    }

    Serial.printf("Total registered widgets: %u / %d\n", 
                  dashboard.getWidgetCount(), DASH_MAX_WIDGETS);
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // 3. Updating Multiple Widgets Efficiently
    // =========================================================================
    static unsigned long lastUpdate = 0;
    if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis;

        // Update all our registered card values
        for (int i = 0; i < NUM_CARDS; ++i) {
            if (cards[i] != nullptr) {
                // Generate a random reading
                float simulatedVoltage = 3.0f + (float)random(0, 30) / 100.0f;
                cards[i]->setValue(simulatedVoltage);
            }
        }
        Serial.println("[Dashboard] Updated values on all 25 sensor cards.");
    }

    // Process dashboard
    dashboard.update();

    delay(20);
}
