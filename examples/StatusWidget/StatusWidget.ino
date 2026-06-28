/**
 * @file StatusWidget.ino
 * @brief Demonstrates the use of Status indicator widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Adding Status widgets to represent system health
 * - Setting different status states using the StatusLevel enumeration
 * - Cycling through states: OK (green), Warning (orange), Error (red), and Info (blue)
 */

#include <ESP32Dashboard.h>
#include <WiFi.h>

// WiFi credentials (replace with your own)
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// Widget pointers
dash::DashStatus* wifiStatus    = nullptr;
dash::DashStatus* sensorStatus  = nullptr;
dash::DashStatus* batteryStatus = nullptr;

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
    // Adding Status Widgets
    // =========================================================================
    
    // Status widgets represent different subsystems or operations.
    // They appear on the dashboard as a colored badge indicating status level.
    
    wifiStatus    = dashboard.addStatus("WiFi Link");
    sensorStatus  = dashboard.addStatus("I2C Sensors");
    batteryStatus = dashboard.addStatus("Backup Battery");

    // Initialize statuses
    if (wifiStatus != nullptr) {
        wifiStatus->setStatus("Connected", StatusLevel::OK);
    }
    if (sensorStatus != nullptr) {
        sensorStatus->setStatus("Initializing...", StatusLevel::Info);
    }
    if (batteryStatus != nullptr) {
        batteryStatus->setStatus("Checking charge", StatusLevel::Info);
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // Update statuses based on simulated conditions
    // =========================================================================
    static unsigned long lastUpdate = 0;
    if (currentMillis - lastUpdate >= 3000) {
        lastUpdate = currentMillis;

        // 1. Cycle Battery status based on charge percentage
        static int simulatedCharge = 100;
        simulatedCharge -= 25; // deplete battery slowly
        if (simulatedCharge < 0) {
            simulatedCharge = 100; // recharge
        }

        if (batteryStatus != nullptr) {
            char badgeText[20];
            snprintf(badgeText, sizeof(badgeText), "%d%% Capacity", simulatedCharge);

            if (simulatedCharge > 50) {
                batteryStatus->setStatus(badgeText, StatusLevel::OK); // Green badge
            } else if (simulatedCharge > 20) {
                batteryStatus->setStatus(badgeText, StatusLevel::Warning); // Amber badge
            } else {
                batteryStatus->setStatus(badgeText, StatusLevel::Error); // Red badge
            }
        }

        // 2. Cycle Sensor status through various fault scenarios
        static int faultIndex = 0;
        faultIndex++;

        if (sensorStatus != nullptr) {
            switch (faultIndex % 4) {
                case 0:
                    sensorStatus->setStatus("All Systems OK", StatusLevel::OK);
                    break;
                case 1:
                    sensorStatus->setStatus("DHT22 CRC Error", StatusLevel::Warning);
                    break;
                case 2:
                    sensorStatus->setStatus("BMP280 Disconnected", StatusLevel::Error);
                    break;
                case 3:
                    sensorStatus->setStatus("Calibrating...", StatusLevel::Info);
                    break;
            }
        }

        // 3. Keep WiFi status updated dynamically
        if (wifiStatus != nullptr) {
            int32_t rssi = WiFi.RSSI();
            char rssiText[32];
            snprintf(rssiText, sizeof(rssiText), "Connected (%d dBm)", rssi);
            
            if (rssi > -67) {
                wifiStatus->setStatus(rssiText, StatusLevel::OK);
            } else if (rssi > -80) {
                wifiStatus->setStatus(rssiText, StatusLevel::Warning);
            } else {
                wifiStatus->setStatus("Critical Low Signal", StatusLevel::Error);
            }
        }

        Serial.println("[Status Test] System statuses updated.");
    }

    // 4. Process dashboard updates
    dashboard.update();

    delay(50);
}
