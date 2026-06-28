/**
 * @file PointerBinding.ino
 * @brief Demonstrates pointer binding mode in ESP32Dashboard.
 *
 * This example covers:
 * - How pointer binding simplifies code (variable updates automatically synchronize with UI)
 * - Binding float, int, double, long, and bool variables to widgets
 * - Understanding the internal dirty-flag check mechanism
 * - Writing clean loops that only focus on sensor code, leaving UI updates to the library
 */

#include <ESP32Dashboard.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Dashboard instance
ESP32Dashboard dashboard;

// -----------------------------------------------------------------------------
// 1. Declare variables that represent your hardware or sensor state
// -----------------------------------------------------------------------------
// The library will store pointers to these memory locations and automatically
// inspect them during dashboard.update() to detect changes.
float       sensorTemp    = 0.0f;
int         ambientLight  = 0;
double      currentDraw   = 0.00;
long        uptimeSecs    = 0;
bool        buttonState   = false;

void setup() {
    Serial.begin(115200);

    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard failed to start!");
        while (true) delay(1000);
    }

    Serial.print("Dashboard: http://");
    Serial.println(dashboard.getIPAddress());

    // -----------------------------------------------------------------------------
    // 2. Bind the variables to widgets using addCard, addGauge, addLED, addMap, etc.
    // -----------------------------------------------------------------------------
    // Passing the address (&) of the variable registers it for automatic checking.
    
    dashboard.addCard("Temperature", &sensorTemp, "°C");
    
    // Gauges also support pointer binding
    auto* lightGauge = dashboard.addGauge("Ambient Light", &ambientLight, "lux");
    if (lightGauge) {
        lightGauge->setRange(0, 4095); // ESP32 ADC range
    }

    dashboard.addCard("Current Draw", &currentDraw, "A");
    dashboard.addCard("System Uptime", &uptimeSecs, "s");
    
    // LEDs can be bound to bool variables
    dashboard.addLED("Input Button State", &buttonState);
}

void loop() {
    // -----------------------------------------------------------------------------
    // 3. Simply update your variables!
    // -----------------------------------------------------------------------------
    // You do NOT need to write any web page code, JSON serializers, or WebSocket
    // packet sender code here. Just write your standard Arduino sensor reading logic:

    unsigned long currentMillis = millis();

    // Read temperature (simulated)
    sensorTemp = 20.0f + 5.0f * sin(currentMillis / 10000.0f);

    // Read analog light level (simulated ESP32 ADC)
    ambientLight = analogRead(34); // If a real sensor was connected to pin 34

    // Read current draw (simulated double)
    currentDraw = 0.50 + 0.10 * cos(currentMillis / 5000.0f);

    // Increment uptime counter
    uptimeSecs = currentMillis / 1000;

    // Read digital pin state (simulated button state toggle)
    buttonState = (currentMillis / 1000) % 2 == 0;

    // -----------------------------------------------------------------------------
    // 4. Call dashboard.update() to sync changes
    // -----------------------------------------------------------------------------
    // Every time dashboard.update() is called, the library performs a check:
    //   if (current_value_at_pointer != last_value_sent) {
    //       mark_widget_dirty();
    //       last_value_sent = current_value_at_pointer;
    //   }
    //
    // If any widgets are marked dirty, a compact JSON payload is built containing
    // only the changed values, and broadcasted over the WebSocket server.
    dashboard.update();

    delay(100); // 100ms cycle matches DASH_UPDATE_INTERVAL_MS default perfectly
}
