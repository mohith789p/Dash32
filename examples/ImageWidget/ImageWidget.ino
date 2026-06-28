/**
 * @file ImageWidget.ino
 * @brief Demonstrates the Image Widget with static, refresh-based, and dynamic URLs.
 */

#include <ESP32Dashboard.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// Widget pointers
DashImage* staticImg = nullptr;
DashImage* refreshImg = nullptr;
DashImage* dynamicImg = nullptr;

unsigned long lastSwitchTime = 0;
bool feedToggle = false;

void setup() {
    Serial.begin(115200);

    // Start dashboard (connects WiFi + starts servers)
    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard failed to start!");
        while (true) { delay(1000); }
    }

    // Configure the dashboard title
    dashboard.setTitle("Media Showcase");

    // 1. Static Image Widget (Standard display, Cover fit, with border)
    staticImg = dashboard.addImage("Static Image (Unsplash)");
    staticImg->setURL("https://images.unsplash.com/photo-1579546929518-9e396f3cc809?auto=format&fit=crop&w=400&q=80");
    staticImg->setFit(ImageFit::Cover);

    // 2. Refresh-based Image Widget (e.g. simulated IP Cam snapshot refreshing every 5s)
    refreshImg = dashboard.addImage("Weather Snapshot (Refreshes 5s)");
    refreshImg->setURL("https://picsum.photos/400/300");
    refreshImg->setRefreshInterval(5000); // 5 seconds
    refreshImg->setFit(ImageFit::Contain);
    refreshImg->showBorder(true);
    refreshImg->enableFullscreen(true);

    // 3. Dynamic Image Widget (URL changed dynamically by the ESP32 program at runtime)
    dynamicImg = dashboard.addImage("Live Camera Feed Switcher");
    dynamicImg->setURL("https://images.unsplash.com/photo-1470071459604-3b5ec3a7fe05?auto=format&fit=crop&w=400&q=80");
    dynamicImg->setFit(ImageFit::Cover);
    dynamicImg->showBorder(false); // No border style test

    Serial.print("Dashboard URL: http://");
    Serial.println(dashboard.getIPAddress());
}

void loop() {
    // Dynamic feed switching demo (every 10 seconds)
    if (millis() - lastSwitchTime > 10000) {
        lastSwitchTime = millis();
        feedToggle = !feedToggle;

        if (feedToggle) {
            dynamicImg->setURL("https://images.unsplash.com/photo-1507525428034-b723cf961d3e?auto=format&fit=crop&w=400&q=80");
            Serial.println("Switched Dynamic Image to Beach View");
        } else {
            dynamicImg->setURL("https://images.unsplash.com/photo-1470071459604-3b5ec3a7fe05?auto=format&fit=crop&w=400&q=80");
            Serial.println("Switched Dynamic Image to Mountain View");
        }
    }

    // Update dashboard (non-blocking)
    dashboard.update();
}
