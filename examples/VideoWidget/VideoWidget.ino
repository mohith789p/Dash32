/**
 * @file VideoWidget.ino
 * @brief Demonstrates the Video Widget with live streaming sources (MJPEG or video files).
 */

#include <ESP32Dashboard.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// Widget pointers
DashVideo* streamVideo = nullptr;
DashVideo* fileVideo   = nullptr;

void setup() {
    Serial.begin(115200);

    // Start dashboard (connects WiFi + starts servers)
    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard failed to start!");
        while (true) { delay(1000); }
    }

    // Set Dashboard title
    dashboard.setTitle("Live Video Streaming");

    // 1. Live MJPEG stream widget (ideal for ESP32-CAM or IP cameras)
    // Note: The frontend automatically detects common MJPEG stream URLs and renders them via <img>
    streamVideo = dashboard.addVideo("ESP32-CAM Stream");
    streamVideo->setURL("http://192.168.1.50:81/stream"); // Replace with actual stream URL
    streamVideo->setAspectRatio(4, 3);                   // ESP32-CAM default is 4:3 (e.g., VGA/SVGA)
    streamVideo->setReconnectInterval(3000);             // Auto-reconnect overlay tries every 3 seconds if disconnected

    // 2. Standard video file/stream widget (uses HTML5 video element)
    fileVideo = dashboard.addVideo("Big Buck Bunny Demo");
    fileVideo->setURL("https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
    fileVideo->setAspectRatio(16, 9);
    fileVideo->setAutoplay(true);
    fileVideo->setMuted(true);
    fileVideo->showControls(true);
    fileVideo->enableFullscreen(true);

    Serial.print("Dashboard URL: http://");
    Serial.println(dashboard.getIPAddress());
}

void loop() {
    // Update dashboard (non-blocking)
    dashboard.update();
}
