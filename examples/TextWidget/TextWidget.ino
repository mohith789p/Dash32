/**
 * @file TextWidget.ino
 * @brief Demonstrates the use of Text display widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Adding a Text widget to display status messages
 * - Updating text content at runtime (e.g. creating a rolling log)
 * - Using multiline text (with newlines '\\n')
 * - Displaying Unicode characters and symbols
 */

#include <ESP32Dashboard.h>
#include <WiFi.h>

// WiFi credentials (replace with your own)
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// Widget pointers
dash::DashText* consoleLog = nullptr;
dash::DashText* infoBox    = nullptr;

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
    // Adding Text Widgets
    // =========================================================================

    // 1. Text widget acting as a console/log. Default text empty.
    consoleLog = dashboard.addText("System Console");
    if (consoleLog != nullptr) {
        consoleLog->setText("System boot complete.\nWaiting for telemetry data...");
    }

    // 2. Text widget displaying system stats and symbols.
    infoBox = dashboard.addText("Device Information");
    if (infoBox != nullptr) {
        // We can use common symbols inside our text block.
        // DashText automatically escapes special JSON characters (like quotes and newlines)
        infoBox->setText("Model: ESP32-WROOM\nStatus: OK ✔\nTemp: 24.5 °C\nLocation: 37°N 122°W");
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // Dynamic Text Updates
    // =========================================================================
    static unsigned long lastUpdate = 0;
    if (currentMillis - lastUpdate >= 4000) {
        lastUpdate = currentMillis;

        // 1. Update the console log with a rotating message
        static int logCount = 0;
        logCount++;

        char logBuffer[128];
        switch (logCount % 4) {
            case 0:
                snprintf(logBuffer, sizeof(logBuffer), "Checking system integrity...\n[OK] Sensors online.\n[OK] Network stable.");
                break;
            case 1:
                snprintf(logBuffer, sizeof(logBuffer), "WARNING: High CPU load!\nUptime: %lu seconds\nFree heap: %lu bytes", 
                         currentMillis / 1000, ESP.getFreeHeap());
                break;
            case 2:
                snprintf(logBuffer, sizeof(logBuffer), "Executing background tasks...\nCleanup complete.");
                break;
            case 3:
                snprintf(logBuffer, sizeof(logBuffer), "System status: Idle\nReady for new commands.\nPress 'RESET' to restart.");
                break;
        }

        if (consoleLog != nullptr) {
            consoleLog->setText(logBuffer);
        }

        // 2. Update info box with varying telemetry parameters
        float chipTemp = (float)temperatureRead(); // Built-in internal sensor on some ESP32s
        char infoBuffer[128];
        snprintf(infoBuffer, sizeof(infoBuffer), "Hardware: ESP32 Core\nUptime: %lu s\nCPU Temp: %.1f °C 🌡\nWiFi RSSI: %d dBm", 
                 currentMillis / 1000, chipTemp, WiFi.RSSI());

        if (infoBox != nullptr) {
            infoBox->setText(infoBuffer);
        }

        Serial.println("[Text Test] Updated system logs and device info.");
    }

    // 3. Process dashboard updates
    dashboard.update();

    delay(50);
}
