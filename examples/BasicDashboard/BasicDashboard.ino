/**
 * @file BasicDashboard.ino
 * @brief Minimal example — 3 cards and 1 gauge with simulated data.
 *
 * Demonstrates:
 * - Pointer-mode binding (automatic updates)
 * - Gauge with custom range
 * - LED indicator
 */

#include <ESP32Dashboard.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Data variables
float temperature = 22.5;
int humidity = 65;
float speed = 0.0;
bool wifiOK = true;

// Dashboard instance
ESP32Dashboard dashboard;

void setup() {
    Serial.begin(115200);

    // Start dashboard (connects WiFi + starts servers)
    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard failed to start!");
        while (true) { delay(1000); }
    }

    // Add widgets — pointer mode (values update automatically)
    dashboard.addCard("Temperature", &temperature, "°C");
    dashboard.addCard("Humidity", &humidity, "%");

    // Gauge with custom range
    auto* gauge = dashboard.addGauge("Speed", &speed, "km/h");
    gauge->setRange(0, 200);

    // LED indicator
    dashboard.addLED("WiFi Status", &wifiOK);

    Serial.print("Dashboard: http://");
    Serial.println(dashboard.getIPAddress());
}

void loop() {
    // Simulate changing data
    temperature = 20.0 + (float)(millis() % 10000) / 1000.0;
    humidity = 50 + (millis() / 500) % 40;
    speed = 60.0 + 40.0 * sin(millis() / 3000.0);

    // Update dashboard (non-blocking)
    dashboard.update();
}
