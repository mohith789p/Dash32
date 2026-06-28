/**
 * @file FullDashboard.ino
 * @brief Comprehensive showcase of all ESP32Dashboard widgets and features working together.
 *
 * This is the flagship example for the ESP32Dashboard library. It sets up:
 * - 3 Card widgets (Temperature, Pressure, Battery level)
 * - 2 Gauge widgets (CPU speed, and a manual Fuel gauge)
 * - 1 Map widget with coordinates, heading rotation, custom marker (Car), trails,
 *     and browser controls (fullscreen, scale, layer switcher, zoom buttons)
 * - 2 LED widgets (a motor running indicator with custom colors, and a sensor status)
 * - 1 Text widget acting as a scrolling console log
 * - 2 Status widgets showing WiFi and GPS severity levels (OK, Warning, Error, Info)
 *
 * Uptime is simulated and variables are updated dynamically in loop().
 */

#include <ESP32Dashboard.h>

// WiFi credentials (replace with your own)
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// -----------------------------------------------------------------------------
// 1. Pointer-Bound Global Variables (State)
// -----------------------------------------------------------------------------
// These variables are updated in loop() and detected automatically by the dashboard.
float  temperature  = 23.5f;   // Celsius
float  airPressure  = 1013.2f; // hPa
int    batteryLevel = 100;     // Percentage
float  cpuSpeed     = 160.0f;  // MHz

double carLatitude  = 37.7749;  // Latitude (San Francisco)
double carLongitude = -122.4194; // Longitude
float  carHeading   = 0.0f;     // Direction of travel (degrees)

bool   motorRunning = false;
bool   sensorStatus = true;

// -----------------------------------------------------------------------------
// 2. Widget Pointers (for manual updates & runtime configuration)
// -----------------------------------------------------------------------------
dash::DashGauge*  pCpuGauge   = nullptr;
dash::DashGauge*  pFuelGauge  = nullptr;
dash::DashMap*    pMap        = nullptr;
dash::DashLED*    pMotorLed   = nullptr;
dash::DashText*   pSystemLog  = nullptr;
dash::DashStatus* pWifiStatus = nullptr;
dash::DashStatus* pGpsStatus  = nullptr;

void setup() {
    // Start serial communications for debugging
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n--- Starting ESP32Dashboard Flagship Demonstration ---");

    // Initialize WiFi and start HTTP / WebSocket servers
    // mDNS is also configured, allowing access via http://dash32.local (or the IP)
    dashboard.setTitle("Full Flagship Monitor");
    if (!dashboard.begin(ssid, password, "dash32")) {
        Serial.println("Dashboard failed to start!");
        while (true) { delay(1000); }
    }

    // Print access info to serial console
    Serial.println("Dashboard started successfully!");
    Serial.printf("Access URL: http://%s/\n", dashboard.getIPAddress());
    Serial.printf("mDNS hostname: http://dash32.local/\n");

    // =========================================================================
    // 3. Widget Initialization & Configuration
    // =========================================================================

    // --- Card Widgets ---
    // Added in pointer binding mode; units specified.
    dashboard.addCard("Ambient Temp", &temperature, "°C");
    dashboard.addCard("Barometric Pressure", &airPressure, "hPa");
    dashboard.addCard("Battery Charge", &batteryLevel, "%");

    // --- Gauge Widgets ---
    // Gauge 1: Pointer-bound to CPU speed with range 80 to 240 MHz
    pCpuGauge = dashboard.addGauge("CPU Speed", &cpuSpeed, "MHz");
    if (pCpuGauge != nullptr) {
        pCpuGauge->setRange(80.0f, 240.0f);
    }

    // Gauge 2: Manual-mode Fuel Level gauge, range 0 to 100%
    pFuelGauge = dashboard.addGauge("Fuel Level", "%");
    if (pFuelGauge != nullptr) {
        pFuelGauge->setRange(0.0f, 100.0f);
        pFuelGauge->setValue(100.0f); // Start full
    }

    // --- Map Widget ---
    // Added with GPS coordinate pointers, heading pointer, and detailed options.
    pMap = dashboard.addMap(&carLatitude, &carLongitude, "Vehicle Location");
    if (pMap != nullptr) {
        pMap->setTheme(MapTheme::Voyager);     // Default theme in browser
        pMap->setMarker(MarkerStyle::Car);     // Car icon marker
        pMap->setZoom(16);                     // Initial zoom level
        pMap->bindHeading(&carHeading);        // Bind marker rotation
        pMap->setHeadingOffset(90.0f);         // Offset since car icon faces right
        pMap->setMarkerScale(1.4f);            // Scale marker up slightly
        pMap->enableTrail(true);               // Enable breadcrumb trail
        pMap->setTrailLength(100);             // Retain last 100 path points
        pMap->setFollow(true);                 // Auto-center map on update
        pMap->setZoomControls(true);           // Enable +/- buttons
        pMap->setFullscreen(true);             // Enable fullscreen button
        pMap->setScale(true);                  // Enable map scale bar
        pMap->enableLayerSwitcher(true);       // Enable browser map theme selector
    }

    // --- LED Widgets ---
    // LED 1: Pointer-bound with custom colors (Orange on, Dark Grey off)
    pMotorLed = dashboard.addLED("Engine Motor", &motorRunning);
    if (pMotorLed != nullptr) {
        pMotorLed->setColors(0xFF5722, 0x222222);
    }

    // LED 2: Pointer-bound with default colors (Green on, Grey off)
    dashboard.addLED("Internal Sensors", &sensorStatus);

    // --- Text Widget ---
    // Used as an output terminal for system logs
    pSystemLog = dashboard.addText("System Console");
    if (pSystemLog != nullptr) {
        pSystemLog->setText("System boot completed.\nConnected to WiFi.\nAll systems green.");
    }

    // --- Status Widgets ---
    // Status indicators displaying badge colors indicating severity
    pWifiStatus = dashboard.addStatus("WiFi Network");
    if (pWifiStatus != nullptr) {
        pWifiStatus->setStatus("Connected", StatusLevel::OK);
    }

    pGpsStatus = dashboard.addStatus("GPS Receiver");
    if (pGpsStatus != nullptr) {
        pGpsStatus->setStatus("Searching...", StatusLevel::Warning);
    }

    Serial.printf("Registered Widgets: %u / %u\n", 
                  dashboard.getWidgetCount(), DASH_MAX_WIDGETS);
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // 4. Simulate Sensor Reading Updates
    // =========================================================================
    
    // Simulate slow fluctuations in temperature
    temperature = 22.5f + 1.8f * sin(currentMillis / 10000.0f);
    
    // Barometric pressure fluctuates around nominal standard
    airPressure = 1013.2f + 2.5f * cos(currentMillis / 15000.0f);

    // Slowly discharge battery over time
    batteryLevel = max(0, 100 - (int)(currentMillis / 60000));

    // Dynamic CPU Speed (simulates heavy workloads raising speed)
    cpuSpeed = 160.0f + 80.0f * (0.5f + 0.5f * sin(currentMillis / 3000.0f));

    // =========================================================================
    // 5. Simulate Map Movement (Vehicle driving in a loop)
    // =========================================================================
    double angle = (double)currentMillis / 20000.0; // circle every 20 seconds
    double travelRadius = 0.0015; // Decimal degrees (approx 150 meters)
    
    carLatitude  = 37.7749 + travelRadius * sin(angle);
    carLongitude = -122.4194 + travelRadius * cos(angle);
    
    // Tangent angle is heading (radians to degrees, normalized 0-360)
    double angleRad = angle + (PI / 2.0);
    carHeading = fmod((angleRad * 180.0 / PI) + 360.0, 360.0);

    // =========================================================================
    // 6. State Machine for Timed Events (Manual Updates)
    // =========================================================================
    static unsigned long lastEventTime = 0;
    if (currentMillis - lastEventTime >= 5000) {
        lastEventTime = currentMillis;

        static int eventCycle = 0;
        eventCycle++;

        // A. Toggle motor running state
        motorRunning = !motorRunning;

        // B. Update fuel level manually on the gauge
        float fuel = 100.0f - (float)((eventCycle * 5) % 100);
        if (pFuelGauge != nullptr) {
            pFuelGauge->setValue(fuel);
        }

        // C. Update statuses and logging based on cycle steps
        if (pSystemLog != nullptr && pGpsStatus != nullptr && pWifiStatus != nullptr) {
            char logBuf[128];
            int rssi = WiFi.RSSI();

            switch (eventCycle % 4) {
                case 0:
                    pGpsStatus->setStatus("3D Fix (8 Sats)", StatusLevel::OK);
                    pSystemLog->setText("GPS: Latitude/Longitude locked.\nSignal: Strong.");
                    break;
                case 1:
                    sensorStatus = true;
                    pSystemLog->setText("Sensors: Performing automatic calibration...\nCalibration PASSED.");
                    break;
                case 2:
                    // Simulate a transient sensor warning
                    sensorStatus = false;
                    pSystemLog->setText("WARNING: I2C bus collision detected on address 0x68.\nRetrying read...");
                    break;
                case 3:
                    // Check RSSI and update WiFi status badge
                    snprintf(logBuf, sizeof(logBuf), "WiFi Link: RSSI %d dBm\nConnected clients: %d", 
                             rssi, dashboard.getClientCount());
                    pSystemLog->setText(logBuf);
                    
                    if (rssi > -70) {
                        pWifiStatus->setStatus("Excellent", StatusLevel::OK);
                    } else {
                        pWifiStatus->setStatus("Weak Signal", StatusLevel::Warning);
                    }
                    break;
            }
        }

        Serial.printf("[Uptime: %lu s] Temp: %.1f°C | Battery: %d%% | Fuel: %.0f%%\n", 
                      currentMillis / 1000, temperature, batteryLevel, 
                      pFuelGauge ? pFuelGauge->getMin() : 0.0f); // getMin/getMax API test
    }

    // =========================================================================
    // 7. Update Dashboard (Core Execution Loop)
    // =========================================================================
    // Call this at the end of loop(). It handles checking for variable changes,
    // managing internal timers, and sending updates to WebSocket clients.
    dashboard.update();

    delay(30); // Yield CPU
}
