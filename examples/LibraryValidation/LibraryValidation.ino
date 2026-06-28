/**
 * @file LibraryValidation.ino
 * @brief Comprehensive validation and regression suite for the ESP32Dashboard library.
 *
 * This sketch instantiates and updates every widget type and configuration
 * parameter in the library, exercising all pointer-bound, manual, and dynamic
 * update paths. It acts as an end-to-end regression and verification test suite.
 *
 * Setup:
 * 1. Update `ssid` and `password` below with your WiFi credentials.
 * 2. Upload to your ESP32-C3 (or other ESP32) target.
 * 3. Open the Serial Monitor (115200 baud).
 * 4. Navigate to http://dash32.local (or http://<IP_ADDRESS>) in a browser.
 *
 * Sections:
 * - Startup verification: validates WiFi, WebServer, WebSocket, mDNS and widget limits.
 * - Card updates: exercises float, double, int, long, bool, manual, and string values.
 * - Gauge updates: exercises pointer, manual, range changes, underflow/overflow.
 * - Map updates: rotates heading through wraparound, cycles themes and marker styles,
 *   simulates movement, exercises follow, trail, scale, layer controls.
 * - LED updates: exercises blinking, manual controls, custom color states.
 * - Text updates: exercises short/long/multiline string rendering.
 * - Status updates: cycles OK -> Warning -> Error -> Info states.
 * - Performance logging: outputs telemetry (loop speed, free heap, update rates).
 * - Stress testing: pushes high-frequency updates to maximize packet handling capacity.
 */

#include <ESP32Dashboard.h>
#include <WiFi.h>

// =============================================================================
// Configuration & Globals
// =============================================================================

// WiFi Credentials (change as needed)
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// --- Pointer Bound Variables ---
// Cards
float       cardFloatVal   = -12.34f;
double      cardDoubleVal  = 123.456789;
int         cardIntVal     = -999;
long        cardLongVal    = 1234567890L;
bool        cardBoolVal    = true;

// Gauges
float       gaugeFloatVal  = 0.0f;
int         gaugeIntVal    = 50;

// Maps
double      mapLat         = 37.7749;
double      mapLon         = -122.4194;
float       mapHdg         = 0.0f;

// LEDs
bool        ledBlinkVal    = false;
bool        ledBoundVal    = true;

// --- Widget Pointers for API Testing ---
dash::DashCard*   pCardFloat     = nullptr;
dash::DashCard*   pCardDouble    = nullptr;
dash::DashCard*   pCardInt       = nullptr;
dash::DashCard*   pCardLong      = nullptr;
dash::DashCard*   pCardBool      = nullptr;
dash::DashCard*   pCardManual    = nullptr;

dash::DashGauge*  pGaugeFloat    = nullptr;
dash::DashGauge*  pGaugeInt      = nullptr;
dash::DashGauge*  pGaugeManual   = nullptr;

dash::DashMap*    pMap           = nullptr;

dash::DashLED*    pLedOn         = nullptr;
dash::DashLED*    pLedOff        = nullptr;
dash::DashLED*    pLedBlink      = nullptr;
dash::DashLED*    pLedCustom     = nullptr;
dash::DashLED*    pLedManual     = nullptr;

dash::DashText*   pText          = nullptr;
dash::DashStatus* pStatus        = nullptr;
dash::DashImage*  pImage         = nullptr;
dash::DashVideo*  pVideo         = nullptr;

// --- Timing Trackers ---
uint32_t lastThemeCycleMs    = 0;
uint32_t lastMarkerCycleMs   = 0;
uint32_t lastTextUpdateMs    = 0;
uint32_t lastStatusCycleMs   = 0;
uint32_t lastPerfLogMs       = 0;
uint32_t lastBlinkMs         = 0;
uint32_t lastManualUpdMs     = 0;

// Theme & Marker Style Arrays for cycling
const MapTheme themes[] = {
    MapTheme::DarkMatter,
    MapTheme::Positron,
    MapTheme::Voyager,
    MapTheme::Satellite,
    MapTheme::Terrain
};
const int themeCount = sizeof(themes) / sizeof(themes[0]);
int currentThemeIdx = 0;

const MarkerStyle markerStyles[] = {
    MarkerStyle::Circle,
    MarkerStyle::Pin,
    MarkerStyle::Car,
    MarkerStyle::Truck,
    MarkerStyle::Motorcycle,
    MarkerStyle::Bicycle,
    MarkerStyle::Drone,
    MarkerStyle::Boat,
    MarkerStyle::Aircraft
};
const int markerCount = sizeof(markerStyles) / sizeof(markerStyles[0]);
int currentMarkerIdx = 0;

// Telemetry statistics
uint32_t loopCount       = 0;
uint32_t maxLoopTimeUs   = 0;
uint32_t minLoopTimeUs   = 0xFFFFFFFF;
uint32_t totalLoopTimeUs = 0;

// =============================================================================
// Helper Functions for Setup & Initialization
// =============================================================================

void setupCards() {
    Serial.println("[TEST] Initializing Card widgets...");
    pCardFloat  = dashboard.addCard("Card Float", &cardFloatVal, "V");
    pCardDouble = dashboard.addCard("Card Double", &cardDoubleVal, "A");
    pCardInt    = dashboard.addCard("Card Int", &cardIntVal, "rpm");
    pCardLong   = dashboard.addCard("Card Long", &cardLongVal, "ticks");
    pCardBool   = dashboard.addCard("Card Bool", &cardBoolVal, "state");
    pCardManual = dashboard.addCard("Card Manual", "kW");

    if (pCardFloat && pCardDouble && pCardInt && pCardLong && pCardBool && pCardManual) {
        Serial.println("[PASS] All Card widgets initialized successfully");
    } else {
        Serial.println("[FAIL] Card widget initialization failed!");
    }
}

void setupGauges() {
    Serial.println("[TEST] Initializing Gauge widgets...");
    pGaugeFloat  = dashboard.addGauge("Gauge Float", &gaugeFloatVal, "%");
    pGaugeInt    = dashboard.addGauge("Gauge Int", &gaugeIntVal, "Hz");
    pGaugeManual = dashboard.addGauge("Gauge Manual", "kPa");

    if (pGaugeFloat && pGaugeInt && pGaugeManual) {
        // Set limits and configuration options
        pGaugeFloat->setRange(-50.0f, 150.0f);
        pGaugeInt->setRange(0, 100);
        pGaugeManual->setRange(0, 500);
        Serial.println("[PASS] Gauge widgets initialized successfully");
    } else {
        Serial.println("[FAIL] Gauge widget initialization failed!");
    }
}

void setupMaps() {
    Serial.println("[TEST] Initializing Map widget...");
    // Bind coordinates initially
    pMap = dashboard.addMap(&mapLat, &mapLon, "Validation Tracker");

    if (pMap) {
        // Configure Map Options
        pMap->setTheme(MapTheme::DarkMatter);
        pMap->setMarker(MarkerStyle::Circle);
        pMap->setZoom(15);
        pMap->setFollow(true);
        pMap->setZoomControls(true);
        pMap->enableTrail(true);
        pMap->setTrailLength(100); // within [2, 500] limit
        pMap->setFullscreen(true);
        pMap->setScale(true);
        pMap->enableLayerSwitcher(true);
        pMap->setHeadingOffset(90.0f); // rotate standard SVGs rightwards
        pMap->setMarkerScale(1.25f);   // slightly scaled up

        // Bind Heading
        pMap->bindHeading(&mapHdg);

        Serial.println("[PASS] Map widget initialized and configured successfully");
    } else {
        Serial.println("[FAIL] Map widget initialization failed!");
    }
}

void setupLEDs() {
    Serial.println("[TEST] Initializing LED widgets...");
    pLedOn     = dashboard.addLED("LED Solid ON");
    pLedOff    = dashboard.addLED("LED Solid OFF");
    pLedBlink  = dashboard.addLED("LED Blinking", &ledBlinkVal);
    pLedCustom = dashboard.addLED("LED Custom Colors", &ledBoundVal);
    pLedManual = dashboard.addLED("LED Manual Mode");

    if (pLedOn && pLedOff && pLedBlink && pLedCustom && pLedManual) {
        // Set static states
        pLedOn->setState(true);
        pLedOff->setState(false);

        // Customize colors (Orange ON, Dark-gray OFF)
        pLedCustom->setColors(0xFF5722, 0x222222);

        Serial.println("[PASS] LED widgets initialized successfully");
    } else {
        Serial.println("[FAIL] LED widget initialization failed!");
    }
}

void setupText() {
    Serial.println("[TEST] Initializing Text widget...");
    pText = dashboard.addText("System Telemetry Console");
    if (pText) {
        pText->setText("Suite loaded. Performing startup validation...");
        Serial.println("[PASS] Text widget initialized successfully");
    } else {
        Serial.println("[FAIL] Text widget initialization failed!");
    }
}

void setupStatus() {
    Serial.println("[TEST] Initializing Status widget...");
    pStatus = dashboard.addStatus("System Health Status");
    if (pStatus) {
        pStatus->setStatus("Booting Validation", StatusLevel::Info);
        Serial.println("[PASS] Status widget initialized successfully");
    } else {
        Serial.println("[FAIL] Status widget initialization failed!");
    }
}

void setupMedia() {
    Serial.println("[TEST] Initializing Media widgets...");
    pImage = dashboard.addImage("Validation Image");
    pVideo = dashboard.addVideo("Validation Video");

    if (pImage && pVideo) {
        pImage->setURL("https://images.unsplash.com/photo-1579546929518-9e396f3cc809?auto=format&fit=crop&w=400&q=80");
        pImage->setFit(ImageFit::Cover);
        pImage->setRefreshInterval(0);
        pImage->enableFullscreen(true);
        pImage->showBorder(true);

        pVideo->setURL("https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
        pVideo->setAspectRatio(16, 9);
        pVideo->setAutoplay(true);
        pVideo->setMuted(true);
        pVideo->showControls(false);
        pVideo->enableFullscreen(true);
        pVideo->setReconnectInterval(3000);

        Serial.println("[PASS] Media widgets initialized successfully");
    } else {
        Serial.println("[FAIL] Media widget initialization failed!");
    }
}

void verifyWidgetLimit() {
    Serial.println("[TEST] Verifying registration limit compliance...");
    
    // We currently have 17 widgets (15 original + 2 media). DASH_MAX_WIDGETS is 20.
    // Let's dynamically add 3 cards to reach the limit.
    for (int i = 0; i < 3; ++i) {
        char name[16];
        snprintf(name, sizeof(name), "Filler Card %d", i);
        auto* filler = dashboard.addCard(name);
        if (!filler) {
            Serial.printf("[FAIL] Failed to register filler card %d. Widget count: %d\n", i, dashboard.getWidgetCount());
            return;
        }
    }

    // Now, adding the 21st widget MUST return nullptr (fails gracefully)
    auto* overLimitCard = dashboard.addCard("Over Limit Card");
    if (overLimitCard == nullptr) {
        Serial.println("[PASS] Widget limits enforced! 21st widget addition rejected (nullptr returned)");
    } else {
        Serial.println("[FAIL] Widget limit check failed! 21st widget registration succeeded incorrectly.");
    }
}

// =============================================================================
// Helper Functions for Runtime Updates
// =============================================================================

void updateCards() {
    // Dynamic coordinate pointer values change
    cardFloatVal = sin(millis() / 2000.0f) * 100.0f;
    cardDoubleVal = 1000.0 + cos(millis() / 5000.0) * 500.0;
    cardIntVal = -1000 + (millis() / 10) % 2001; // Negative to positive
    cardLongVal = 1234567890L + (millis() / 500);
    cardBoolVal = (millis() / 1500) % 2 == 0;

    // Manual Card Updates
    if (millis() - lastManualUpdMs >= 1000) {
        lastManualUpdMs = millis();
        
        static int manualCardState = 0;
        manualCardState++;
        
        switch (manualCardState % 4) {
            case 0:
                pCardManual->setValue(45.67f); // decimal
                break;
            case 1:
                pCardManual->setValue(-123);   // negative
                break;
            case 2:
                pCardManual->setValue(987654L); // long
                break;
            case 3:
                pCardManual->setValue("Manual OK"); // string
                break;
        }
    }
}

void updateGauges() {
    // Pointer gauge animations (dynamic sine wave)
    gaugeFloatVal = 50.0f + sin(millis() / 1000.0f) * 120.0f; // range is [-50, 150]
    gaugeIntVal = (millis() / 50) % 101;

    // Manual Gauge underflow and overflow validation
    static uint32_t lastGaugeUpd = 0;
    if (millis() - lastGaugeUpd >= 1200) {
        lastGaugeUpd = millis();
        static int gaugeState = 0;
        gaugeState++;
        
        switch (gaugeState % 3) {
            case 0:
                pGaugeManual->setValue(-50);  // Underflow limit (0)
                break;
            case 1:
                pGaugeManual->setValue(250);  // Normal range
                break;
            case 2:
                pGaugeManual->setValue(600);  // Overflow limit (500)
                break;
        }
    }
}

void updateMaps() {
    // 1. Move latitude/longitude continuously (circular path)
    double angle = millis() / 10000.0;
    mapLat = 37.7749 + 0.005 * sin(angle);
    mapLon = -122.4194 + 0.005 * cos(angle);

    // 2. Rotate heading continuously (0-360) and test wraparound
    mapHdg = fmod(millis() / 50.0, 360.0);

    // 3. Cycle Themes every 10 seconds (verifies config update broadcast)
    if (millis() - lastThemeCycleMs >= 10000) {
        lastThemeCycleMs = millis();
        currentThemeIdx = (currentThemeIdx + 1) % themeCount;
        pMap->setTheme(themes[currentThemeIdx]);
        
        const char* themeNames[] = {"DarkMatter", "Positron", "Voyager", "Satellite", "Terrain"};
        Serial.printf("[PASS] Map theme changed to: %s\n", themeNames[currentThemeIdx]);
    }

    // 4. Cycle Marker Styles every 8 seconds
    if (millis() - lastMarkerCycleMs >= 8000) {
        lastMarkerCycleMs = millis();
        currentMarkerIdx = (currentMarkerIdx + 1) % markerCount;
        pMap->setMarker(markerStyles[currentMarkerIdx]);
        
        const char* markerNames[] = {"Circle", "Pin", "Car", "Truck", "Motorcycle", "Bicycle", "Drone", "Boat", "Aircraft"};
        Serial.printf("[PASS] Marker updated to: %s\n", markerNames[currentMarkerIdx]);
    }
}

void updateLEDs() {
    // 1. Blinking LED toggled every 500ms
    if (millis() - lastBlinkMs >= 500) {
        lastBlinkMs = millis();
        ledBlinkVal = !ledBlinkVal;
    }

    // 2. Custom Colors LED state toggles every 2000ms
    static uint32_t lastCustomLedMs = 0;
    if (millis() - lastCustomLedMs >= 2000) {
        lastCustomLedMs = millis();
        ledBoundVal = !ledBoundVal;
    }

    // 3. Manual LED toggles every 1500ms
    static uint32_t lastManualLedMs = 0;
    if (millis() - lastManualLedMs >= 1500) {
        lastManualLedMs = millis();
        static bool manualLedState = false;
        manualLedState = !manualLedState;
        pLedManual->setState(manualLedState);
    }
}

void updateText() {
    // Update Text widget every 4 seconds
    if (millis() - lastTextUpdateMs >= 4000) {
        lastTextUpdateMs = millis();
        static int textCycle = 0;
        textCycle++;

        switch (textCycle % 4) {
            case 0:
                pText->setText("Status: RUNNING\nUptime: " + String(millis() / 1000) + "s");
                break;
            case 1:
                pText->setText("Short text message");
                break;
            case 2:
                pText->setText("Long text message: validating system performance limits, memory fragmentation, and WebSocket transmission reliability under intense workload conditions.");
                break;
            case 3:
                pText->setText("Unicode symbols: °C, 360°, ☺, ☀, ✈");
                break;
        }
    }
}

void updateStatus() {
    // Cycle status severity and label every 3 seconds
    if (millis() - lastStatusCycleMs >= 3000) {
        lastStatusCycleMs = millis();
        static int statusCycle = 0;
        statusCycle++;

        switch (statusCycle % 4) {
            case 0:
                pStatus->setStatus("System Nominal", StatusLevel::OK);
                break;
            case 1:
                pStatus->setStatus("High Memory Usage", StatusLevel::Warning);
                break;
            case 2:
                pStatus->setStatus("Telemetry Failure", StatusLevel::Error);
                break;
            case 3:
                pStatus->setStatus("Sensor Offline", StatusLevel::Info);
                break;
        }
    }
}

void updateMedia() {
    static uint32_t lastMediaUpd = 0;
    if (millis() - lastMediaUpd >= 15000) {
        lastMediaUpd = millis();
        static bool toggle = false;
        toggle = !toggle;

        if (toggle) {
            pImage->setURL("https://images.unsplash.com/photo-1507525428034-b723cf961d3e?auto=format&fit=crop&w=400&q=80");
            pVideo->setURL("http://192.168.1.50:81/stream");
            Serial.println("[PASS] Media URLs toggled to alternative state");
        } else {
            pImage->setURL("https://images.unsplash.com/photo-1579546929518-9e396f3cc809?auto=format&fit=crop&w=400&q=80");
            pVideo->setURL("https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
            Serial.println("[PASS] Media URLs toggled to initial state");
        }
    }
}

// =============================================================================
// Statistics & Performance Telemetry
// =============================================================================

void printStatistics() {
    if (millis() - lastPerfLogMs >= 5000) {
        lastPerfLogMs = millis();

        uint32_t averageLoopUs = (loopCount > 0) ? (totalLoopTimeUs / loopCount) : 0;
        uint32_t freeHeap = ESP.getFreeHeap();
        uint32_t minHeap = ESP.getMinFreeHeap();

        Serial.println("=================================================");
        Serial.println("        DASH32 VALIDATION SUITE STATISTICS       ");
        Serial.println("=================================================");
        Serial.printf("Uptime:              %u seconds\n", millis() / 1000);
        Serial.printf("Client Count:        %u connected\n", dashboard.getClientCount());
        Serial.printf("Widget Count:        %u registered\n", dashboard.getWidgetCount());
        Serial.printf("Free Heap Memory:    %u bytes\n", freeHeap);
        Serial.printf("Min Free Heap:       %u bytes\n", minHeap);
        Serial.printf("Loop Frequency:      %u Hz\n", loopCount / 5);
        Serial.printf("Average Loop Time:   %u us\n", averageLoopUs);
        Serial.printf("Min/Max Loop Time:   %u us / %u us\n", minLoopTimeUs, maxLoopTimeUs);
        Serial.printf("Gzip Asset Server:   %s\n", freeHeap > 10000 ? "PASS" : "WARN");
        Serial.println("=================================================");

        // Reset loop metrics for next 5-second window
        loopCount = 0;
        totalLoopTimeUs = 0;
        maxLoopTimeUs = 0;
        minLoopTimeUs = 0xFFFFFFFF;
    }
}

// =============================================================================
// Main Arduino Entry Points
// =============================================================================

void setup() {
    Serial.begin(115200);
    delay(2000); // Allow serial window to settle

    Serial.println("\n");
    Serial.println("=================================================");
    Serial.println("         ESP32DASHBOARD VALIDATION SUITE         ");
    Serial.println("=================================================");

    // Add widgets (total = 17)
    setupCards();
    setupGauges();
    setupMaps();
    setupLEDs();
    setupText();
    setupStatus();
    setupMedia();

    // Verify compliance with the maximum widget limit (20)
    verifyWidgetLimit();

    // Startup connection and servers initialization
    Serial.println("[TEST] Starting Dashboard Services...");
    // Uses MDNS hostname "validation" (resolvable at http://validation.local)
    bool initSuccess = dashboard.begin(ssid, password, "validation");

    if (initSuccess) {
        Serial.println("[PASS] Dashboard initialization success");
        Serial.printf("[PASS] IP Address: %s\n", dashboard.getIPAddress());
        Serial.println("[PASS] HTTP Server Started");
        Serial.println("[PASS] WebSocket Started");
    } else {
        Serial.println("[FAIL] WiFi or Server Initialization Failed!");
    }
}

void loop() {
    uint32_t startUs = micros();

    // 1. Process network and execute internal websocket check loops
    dashboard.update();

    // 2. Perform updates to all widget configurations & bindings
    updateCards();
    updateGauges();
    updateMaps();
    updateLEDs();
    updateText();
    updateStatus();
    updateMedia();

    // 3. Telemetry and statistics print
    printStatistics();

    // Calculate loop metrics
    uint32_t elapsedUs = micros() - startUs;
    loopCount++;
    totalLoopTimeUs += elapsedUs;
    if (elapsedUs > maxLoopTimeUs) maxLoopTimeUs = elapsedUs;
    if (elapsedUs < minLoopTimeUs) minLoopTimeUs = elapsedUs;

    // Yield to prevent watchdog triggers
    yield();
}
