/**
 * @file ManualUpdates.ino
 * @brief Demonstrates manual-mode updates for widgets in ESP32Dashboard.
 *
 * This example covers:
 * - Creating cards, gauges, maps, LEDs, and text/status widgets in manual mode
 * - Explaining when and why to use manual mode (event-driven vs polling)
 * - Updating widgets using their pointers (`widget->setValue()`, etc.)
 * - Updating cards using dashboard convenience methods (`dashboard.setCardValue()`)
 * - Lowering CPU overhead by updating only when changes actually happen
 */

#include <ESP32Dashboard.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// Widget pointers
dash::DashCard*   myCard   = nullptr;
dash::DashGauge*  myGauge  = nullptr;
dash::DashLED*    myLed    = nullptr;
dash::DashMap*    myMap    = nullptr;
dash::DashText*   myText   = nullptr;
dash::DashStatus* myStatus = nullptr;

void setup() {
    Serial.begin(115200);

    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard initialization failed!");
        while (true) delay(1000);
    }

    Serial.print("Dashboard: http://");
    Serial.println(dashboard.getIPAddress());

    // =========================================================================
    // 1. Initializing Widgets in Manual Mode
    // =========================================================================
    // Notice that we pass NO pointers to variables in these initialization calls.
    // The dashboard does not track any variables automatically for these widgets.

    myCard   = dashboard.addCard("Static Load", "kg");
    myGauge  = dashboard.addGauge("Vessel Pressure", "PSI");
    myLed    = dashboard.addLED("Vent Valve");
    myMap    = dashboard.addMap("Vessel Coordinates");
    myText   = dashboard.addText("Operations Console");
    myStatus = dashboard.addStatus("Process Health");

    // =========================================================================
    // 2. Setting Initial Manual Values
    // =========================================================================
    if (myCard)   myCard->setValue(0.0f);
    if (myGauge)  myGauge->setRange(0, 100);
    if (myGauge)  myGauge->setValue(15.2f);
    if (myLed)    myLed->setColors(0x00FF88, 0x333333);
    if (myLed)    myLed->setState(false); // closed
    if (myMap)    myMap->setCoordinates(37.7749, -122.4194);
    if (myText)   myText->setText("Ready. Initial scan complete.");
    if (myStatus) myStatus->setStatus("System Idle", StatusLevel::Info);

    // Alternative: We can also add cards without saving a widget pointer
    // and update them by their title using the dashboard helper.
    dashboard.addCard("Convenience Card", "unit");
    dashboard.setCardValue("Convenience Card", "Ready!");
}

void loop() {
    unsigned long currentMillis = millis();

    // =========================================================================
    // 3. Simulating Event-Driven Updates
    // =========================================================================
    // In typical setups, you only want to update widgets when an event actually 
    // happens (e.g. an interrupt, a packet received, or a state transition)
    // rather than running computation on every loop() cycle.

    static unsigned long lastEventTime = 0;
    if (currentMillis - lastEventTime >= 5000) { // Every 5 seconds
        lastEventTime = currentMillis;

        static int step = 0;
        step++;

        Serial.printf("[Event] Running system step %d...\n", step);

        if (step % 3 == 1) {
            // Event A: High pressure detected! Open vent valve.
            if (myGauge)  myGauge->setValue(85.0f);
            if (myLed)    myLed->setState(true); // Open vent valve indicator
            if (myStatus) myStatus->setStatus("HIGH PRESSURE WARNING", StatusLevel::Warning);
            if (myText)   myText->setText("ALERT: Pressure spiked to 85 PSI. Opening Vent Valve...");
            if (myCard)   myCard->setValue(120.5f); // Weight drops as gas escapes

            // Update Map location to indicate venting site
            if (myMap)    myMap->setCoordinates(37.7752, -122.4189);

        } else if (step % 3 == 2) {
            // Event B: Pressure under control. Close vent valve.
            if (myGauge)  myGauge->setValue(35.5f);
            if (myLed)    myLed->setState(false); // Close vent valve
            if (myStatus) myStatus->setStatus("Nominal Operation", StatusLevel::OK);
            if (myText)   myText->setText("INFO: Pressure stabilized at 35.5 PSI. Vent Valve closed.");
            if (myCard)   myCard->setValue(115.0f);

        } else {
            // Event C: Maintenance cycle.
            if (myStatus) myStatus->setStatus("Maintenance Check", StatusLevel::Info);
            if (myText)   myText->setText("INFO: Performing self-diagnostic check. All loops normal.");
            
            // Update the card by Title string
            dashboard.setCardValue("Convenience Card", "Checking...");
        }
    }

    // =========================================================================
    // 4. Update the Dashboard
    // =========================================================================
    // Even when using manual mode, dashboard.update() must be called in loop().
    // It is non-blocking and handles serializing and sending the changes
    // we marked dirty when we called setValue(), setState(), setStatus(), etc.
    dashboard.update();

    delay(20);
}
