/**
 * @file Maps.ino
 * @brief Demonstrates the use of Map widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Adding a Map widget to track geographic coordinates
 * - Binding latitude, longitude, and heading (marker rotation) pointers
 * - Configuring map views: themes, marker styles, zoom level, follow mode, trail length
 * - Enabling user controls: scale, zoom buttons, fullscreen, and layer switcher
 * - Simulating GPS movement and marker rotation (wraparound)
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
// GPS coordinates (initialized to San Francisco, CA)
double latitude  = 37.7749;
double longitude = -122.4194;
float  heading   = 0.0f;  // Marker heading in degrees (0 - 360)

// -----------------------------------------------------------------------------
// 2. Widget Pointers
// -----------------------------------------------------------------------------
dash::DashMap* gpsMap = nullptr;

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
    // Adding and Configuring a Map Widget
    // =========================================================================
    
    // Add map bound to latitude and longitude
    gpsMap = dashboard.addMap(&latitude, &longitude, "Fleet Tracker");
    
    if (gpsMap != nullptr) {
        // A. Set Map Theme
        // Options: MapTheme::DarkMatter (default), MapTheme::Positron (light),
        //          MapTheme::Voyager (detailed light), MapTheme::Satellite, MapTheme::Terrain
        gpsMap->setTheme(MapTheme::DarkMatter);

        // B. Set Marker Style
        // Options: MarkerStyle::Circle (default), MarkerStyle::Pin, MarkerStyle::Car,
        //          MarkerStyle::Truck, MarkerStyle::Motorcycle, MarkerStyle::Bicycle,
        //          MarkerStyle::Drone, MarkerStyle::Boat, MarkerStyle::Aircraft
        gpsMap->setMarker(MarkerStyle::Aircraft);

        // C. Set Zoom level (clamped between 1 and 19)
        gpsMap->setZoom(16);

        // D. Set Heading / Rotation pointer
        // Allows the aircraft marker to face the direction of travel.
        // If your SVG marker's front is not facing UP, configure a heading offset.
        gpsMap->bindHeading(&heading);
        gpsMap->setHeadingOffset(0.0f); // Adjust if icon natural direction differs from north

        // E. Enable follow mode
        // True  = Map automatically centers on coordinates as they change.
        // False = User can pan freely; marker moves independently.
        gpsMap->setFollow(true);

        // F. Trail configuration
        // Draw a path behind the marker showing where it has been.
        gpsMap->enableTrail(true);
        gpsMap->setTrailLength(150); // Keep last 150 points

        // G. Marker options
        gpsMap->setMarkerScale(1.5f); // Scale up marker by 1.5x

        // H. Enable Frontend Map UI Controls
        gpsMap->setZoomControls(true);     // Show +/- zoom buttons
        gpsMap->setFullscreen(true);       // Show fullscreen button
        gpsMap->setScale(true);            // Show map distance scale
        gpsMap->enableLayerSwitcher(true); // Allow user to swap themes in the browser
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // Simulate GPS Movement (Circular Flight Path)
    // =========================================================================
    double timeAngle = (double)currentMillis / 15000.0; // Complete full circle every 15s
    
    // Radius of path in decimal degrees (approx. 200 meters)
    double radius = 0.002; 
    
    // Calculate new position
    latitude  = 37.7749 + radius * sin(timeAngle);
    longitude = -122.4194 + radius * cos(timeAngle);

    // Calculate heading (tangent to the circular path)
    // Tangent angle is: (timeAngle + PI/2) or (timeAngle - PI/2) depending on direction
    double tangentRad = timeAngle + (PI / 2.0);
    
    // Convert to degrees and normalize to [0, 360)
    heading = fmod((tangentRad * 180.0 / PI) + 360.0, 360.0);

    // =========================================================================
    // Dynamic Runtime Configurations
    // =========================================================================
    static unsigned long lastConfigChange = 0;
    if (currentMillis - lastConfigChange >= 10000) {
        lastConfigChange = currentMillis;

        // Demonstrate changing marker styles dynamically at runtime
        static int markerIndex = 0;
        markerIndex++;
        
        if (gpsMap != nullptr) {
            switch (markerIndex % 4) {
                case 0:
                    gpsMap->setMarker(MarkerStyle::Aircraft);
                    Serial.println("[Map Test] Marker changed to: Aircraft");
                    break;
                case 1:
                    gpsMap->setMarker(MarkerStyle::Drone);
                    Serial.println("[Map Test] Marker changed to: Drone");
                    break;
                case 2:
                    gpsMap->setMarker(MarkerStyle::Car);
                    Serial.println("[Map Test] Marker changed to: Car");
                    break;
                case 3:
                    gpsMap->setMarker(MarkerStyle::Pin);
                    Serial.println("[Map Test] Marker changed to: Pin");
                    break;
            }
        }
    }

    // Process dashboard updates (non-blocking)
    dashboard.update();

    delay(50);
}
