/**
 * @file FullDashboard.ino
 * @brief Complete example showing all widget types.
 *
 * Demonstrates:
 * - Cards (pointer + manual mode)
 * - Gauge with range
 * - Map with coordinates
 * - LED indicators
 * - Text widget
 * - Status indicator
 */

#include <ESP32Dashboard.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Sensor data
float temperature = 22.5;
float pressure = 1013.25;
int batteryLevel = 85;
float cpuSpeed = 240.0;
bool motorRunning = false;
bool sensorOK = true;

// GPS data
double latitude = 37.7749;
double longitude = -122.4194;

ESP32Dashboard dashboard;

void setup() {
    Serial.begin(115200);

    if (!dashboard.begin(ssid, password)) {
        Serial.println("Failed to start dashboard");
        while (true) delay(1000);
    }

    // --- Cards ---
    dashboard.addCard("Temperature", &temperature, "°C");
    dashboard.addCard("Pressure", &pressure, "hPa");
    dashboard.addCard("Battery", &batteryLevel, "%");

    // --- Gauges ---
    auto* cpuGauge = dashboard.addGauge("CPU Speed", &cpuSpeed, "MHz");
    cpuGauge->setRange(80, 240);

    // --- Map ---
    dashboard.addMap(&latitude, &longitude, "Vehicle Location");

    // --- LEDs ---
    auto* motorLed = dashboard.addLED("Motor", &motorRunning);
    motorLed->setColors(0xFF6B35, 0x333333); // Orange on, dark off

    dashboard.addLED("Sensor", &sensorOK);

    // --- Text ---
    auto* logWidget = dashboard.addText("System Log");
    logWidget->setText("System initialized. All sensors nominal.");

    // --- Status ---
    auto* wifiStatus = dashboard.addStatus("WiFi");
    wifiStatus->setStatus("Connected", StatusLevel::OK);

    auto* gpsStatus = dashboard.addStatus("GPS");
    gpsStatus->setStatus("Acquiring...", StatusLevel::Warning);

    Serial.printf("Dashboard: http://%s\n", dashboard.getIPAddress());
    Serial.printf("Widgets: %u / %u\n", dashboard.getWidgetCount(), DASH_MAX_WIDGETS);
}

void loop() {
    // Simulate sensor readings
    unsigned long t = millis();
    temperature = 20.0 + 5.0 * sin(t / 5000.0);
    pressure = 1010.0 + 10.0 * cos(t / 8000.0);
    batteryLevel = max(0, 100 - (int)(t / 60000));
    cpuSpeed = 160.0 + 80.0 * (0.5 + 0.5 * sin(t / 2000.0));

    // Simulate GPS movement
    latitude = 37.7749 + 0.001 * sin(t / 10000.0);
    longitude = -122.4194 + 0.001 * cos(t / 10000.0);

    // Toggle motor every 5 seconds
    motorRunning = (t / 5000) % 2 == 0;

    // Update GPS status after 10 seconds
    static bool gpsFixed = false;
    if (!gpsFixed && t > 10000) {
        gpsFixed = true;
        // Find and update status widget
        // (Widgets return pointers, so you can store them in setup)
    }

    dashboard.update();
}
