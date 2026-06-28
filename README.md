# ESP32Dashboard (Dash32)

**Zero-frontend, real-time web monitoring dashboard for ESP32 devices.**

Build professional IoT dashboards with pure C++ — no HTML, CSS, or JavaScript required. The dashboard page is stored in the ESP32's flash memory and served instantly to any web browser over local networks.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)]()
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal.svg)]()

---

## Table of Contents
1. [Project Overview](#project-overview)
2. [Supported Boards](#supported-boards)
3. [Installation](#installation)
4. [Networking Requirements](#networking-requirements)
5. [Quick Start & Minimal Example](#quick-start--minimal-example)
6. [Library Architecture](#library-architecture)
7. [Folder Structure](#folder-structure)
8. [Available Widgets](#available-widgets)
9. [Public APIs & Reference](#public-apis--reference)
10. [Widget Customization Options](#widget-customization-options)
11. [Configuration Overrides](#configuration-overrides)
12. [Example Sketches](#example-sketches)
13. [Performance Considerations](#performance-considerations)
14. [Memory Considerations](#memory-considerations)
15. [Browser Compatibility](#browser-compatibility)
16. [Troubleshooting](#troubleshooting)
17. [Frequently Asked Questions (FAQ)](#frequently-asked-questions-faq)
18. [Best Practices](#best-practices)
19. [Contribution Guidelines](#contribution-guidelines)
20. [License](#license)

---

## Project Overview
ESP32Dashboard allows hardware developers to visualize sensor data, track GPS positions, and control pins using an attractive, responsive UI. Using a dual value mode, widgets can either be bound directly to C++ pointers (for automatic polling updates) or manipulated manually in an event-driven style. 

Data is streamed efficiently to the client browser via WebSockets. To minimize bandwidth and loop cycles, only modified widget values (deltas) are sent, keeping packets lightweight.

---

## Supported Boards
The library works on all microcontrollers powered by the ESP32 chip family using the Arduino core:
- **ESP32** (Generic / NodeMCU / WROOM)
- **ESP32-S2**
- **ESP32-S3**
- **ESP32-C3**
- **ESP32-C6**

---

## Installation

### Arduino Library Manager (Recommended)
1. Open the Arduino IDE.
2. Go to **Sketch** → **Include Library** → **Manage Libraries**.
3. Search for `ESP32Dashboard`.
4. Click **Install**.

### Manual Installation
1. Download this repository as a ZIP archive.
2. Unzip and rename the folder to `ESP32Dashboard`.
3. Move the folder to your `Arduino/libraries/` folder.
4. Restart your Arduino IDE.

### Dependencies
The library relies on:
- **WebSockets** by Markus Sattler (installable via the Library Manager)
- Built-in libraries: `WiFi`, `WebServer`, `DNSServer`, `mDNS` (included in the ESP32 Arduino Core)

---

## Networking Requirements
- **Local Area Network (LAN):** The ESP32 and the client browser must be on the same subnet.
- **Port Allocations:** 
  - Default HTTP Server: **80**
  - Default WebSocket Server: **81**
  *(Ports can be overridden in `begin()`)*
- **mDNS Support:** Requires mDNS (Multicast DNS) client support on the browsing device (enabled by default on macOS, iOS, Windows, and most Linux setups). Allows resolution of hostnames like `http://dash32.local/`.

---

## Quick Start & Minimal Example

```cpp
#include <ESP32Dashboard.h>

// WiFi Credentials
const char* ssid = "YourSSID";
const char* password = "YourPassword";

// Dashboard instance
ESP32Dashboard dashboard;

// Variables bound to the widgets
float temperature = 22.5f;
int   humidity    = 65;

void setup() {
    Serial.begin(115200);

    // Initialize WiFi, HTTP, and WebSocket servers
    if (!dashboard.begin(ssid, password)) {
        Serial.println("Dashboard failed to start!");
        while (true) delay(1000);
    }

    // Add widgets bound to variables
    dashboard.addCard("Temperature", &temperature, "°C");
    dashboard.addCard("Humidity", &humidity, "%");

    Serial.print("Dashboard URL: http://");
    Serial.println(dashboard.getIPAddress());
}

void loop() {
    // 1. Update variables (simulate sensors)
    temperature = 20.0f + (float)(millis() % 10000) / 1000.0f;
    humidity = 50 + (millis() / 500) % 40;

    // 2. Poll changes and broadcast to clients (non-blocking)
    dashboard.update();

    delay(50); // Yield to prevent watchdog triggers
}
```

Open `http://<ESP32_IP>` in your browser to view the dashboard live.

---

## Library Architecture
The library uses a highly optimized composite structure:
- **`ESP32Dashboard`** acts as the central coordinator, orchestrating the HTTP web server, WebSocket server, JSON serialization engine, and widget management.
- **`ValueHolder`** provides type-safe, type-erased pointer/value management.
- **`DashWidget`** is the abstract polymorphic base class defining the standard interface for widgets.
- **WebSocket Protocol** broadcasts configuration details upon connection and handles periodic delta synchronization (transmitting changed values only).
- **Embedded UI assets** (HTML, CSS, JS) are stored directly in Flash memory (`PROGMEM`) as compressed strings.

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32Dashboard                       │
│                                                         │
│  ┌─────────────────┐ ┌─────────────────┐ ┌───────────┐  │
│  │ DashWebServer   │ │ DashWebSocket   │ │ JSON      │  │
│  │ (Serves UI page)│ │ (Sends updates) │ │ Serializer│  │
│  └─────────────────┘ └─────────────────┘ └───────────┘  │
│                           │                             │
│                           ▼                             │
│                     [Widget Array]                      │
│      ┌────────────┬───────────┬───────────┬─────────┐   │
│      │  DashCard  │ DashGauge │  DashMap  │ DashLED │   │
│      └────────────┴───────────┴───────────┴─────────┘   │
└─────────────────────────────────────────────────────────┘
```

---

## Folder Structure
```
.
├── library.properties      # Arduino Library metadata
├── keywords.txt            # IDE syntax highlighting keywords
├── LICENSE                 # License terms
├── README.md               # Documentation
├── examples/               # Standard library sketches
│   ├── BasicDashboard/
│   ├── Cards/
│   ├── Gauges/
│   ├── LEDs/
│   ├── TextWidget/
│   ├── StatusWidget/
│   ├── Maps/
│   ├── ManualUpdates/
│   ├── PointerBinding/
│   ├── MultipleWidgets/
│   └── FullDashboard/
└── src/                    # Source files
    ├── ESP32Dashboard.h/.cpp
    ├── core/
    │   ├── DashConfig.h    # Compile-time constants
    │   ├── DashTypes.h     # Enums & helper structures
    │   ├── DashDebug.h     # Debug logging macros
    │   └── ValueHolder.h/.cpp
    ├── data/
    │   └── DashJsonEngine.h/.cpp
    ├── network/
    │   ├── DashWebServer.h/.cpp
    │   └── DashWebSocket.h/.cpp
    ├── ui/                 # Embedded HTML/CSS/JS code
    │   ├── DashPageHTML.h
    │   ├── DashPageCSS.h
    │   ├── DashPageJS.h
    │   └── DashLeaflet.h
    └── widgets/            # Subclassed widget controls
        ├── DashWidget.h/.cpp
        ├── DashCard.h/.cpp
        ├── DashGauge.h/.cpp
        ├── DashLED.h/.cpp
        ├── DashMap.h/.cpp
        ├── DashText.h/.cpp
        └── DashStatus.h/.cpp
```

---

## Available Widgets

| Widget | Description | Value Types | Default Binding | Mode Support |
|---|---|---|:---:|:---:|
| **Card** | Displays a single label, numerical/string value, and unit badge. | `float`, `double`, `int`, `long`, `bool`, `const char*` | Pointer | Pointer & Manual |
| **Gauge** | Displays a progress arc based on min/max parameters. | `float`, `double`, `int`, `long` | Pointer | Pointer & Manual |
| **Map** | Leaflet-based geographic display showing path, markers, and zoom. | Latitude (`double`), Longitude (`double`), Heading (`float`) | Pointer | Pointer & Manual |
| **LED** | An ON/OFF status circle with customizable color options. | `bool` | Pointer | Pointer & Manual |
| **Text** | Displays multiline blocks of text. Supports JSON escaping. | `const char*` | Manual | Manual Only |
| **Status** | A status pill styled green, orange, red, or blue. | `StatusLevel` (enum), `const char*` | Manual | Manual Only |

---

## Public APIs & Reference

### ESP32Dashboard Class

#### Lifecycle
* **`bool begin(const char* ssid, const char* password, uint16_t httpPort = 80, uint16_t wsPort = 81)`**
  *Connects to WiFi network and launches the HTTP and WebSocket servers.*
* **`bool begin(const char* ssid, const char* password, const char* hostname, uint16_t httpPort = 80, uint16_t wsPort = 81)`**
  *Connects to WiFi, sets up mDNS with the provided hostname, and launches servers.*
* **`void update()`**
  *Core execution loop. Must be called frequently inside `loop()`. Polling and client communications happen here.*

#### Widget Creation (returns pointer to widget, or `nullptr` if full)
* **`DashCard* addCard(const char* title, float* value, const char* unit = "")`**
* **`DashCard* addCard(const char* title, double* value, const char* unit = "")`**
* **`DashCard* addCard(const char* title, int* value, const char* unit = "")`**
* **`DashCard* addCard(const char* title, long* value, const char* unit = "")`**
* **`DashCard* addCard(const char* title, bool* value, const char* unit = "")`**
* **`DashCard* addCard(const char* title, const char* unit = "")`** *(Manual mode)*
* **`DashGauge* addGauge(const char* title, float* value, const char* unit = "")`**
* **`DashGauge* addGauge(const char* title, int* value, const char* unit = "")`**
* **`DashGauge* addGauge(const char* title, const char* unit = "")`** *(Manual mode)*
* **`DashMap* addMap(double* lat, double* lon, const char* title = "Location")`**
* **`DashMap* addMap(const char* title = "Location")`** *(Manual mode)*
* **`DashLED* addLED(const char* title, bool* state = nullptr)`**
* **`DashText* addText(const char* title)`**
* **`DashStatus* addStatus(const char* title)`**

#### Manual Modifiers
* **`bool setCardValue(const char* title, float value)`**
* **`bool setCardValue(const char* title, int value)`**
* **`bool setCardValue(const char* title, const char* value)`**
  *Finds a card by title and sets its value directly, marking it dirty.*

#### Queries
* **`uint8_t getWidgetCount() const`**
  *Returns the number of registered widgets.*
* **`uint8_t getClientCount() const`**
  *Returns the number of connected browser WebSocket clients.*
* **`const char* getIPAddress() const`**
  *Returns the local IP address of the board.*

---

## Widget Customization Options

### DashCard
```cpp
// Bind new variables at runtime
void bindValue(float* ptr);
void bindValue(double* ptr);
void bindValue(int* ptr);
void bindValue(long* ptr);
void bindValue(bool* ptr);

// Set manual values explicitly
void setValue(float val);
void setValue(double val);
void setValue(int val);
void setValue(long val);
void setValue(bool val);
void setValue(const char* val);
```

### DashGauge
```cpp
// Configure limits
void setRange(float min, float max);
float getMin() const;
float getMax() const;

// Bindings and manual setting
void bindValue(float* ptr);
void bindValue(double* ptr);
void bindValue(int* ptr);
void bindValue(long* ptr);
void setValue(float val);
void setValue(double val);
void setValue(int val);
void setValue(long val);
```

### DashLED
```cpp
// Set custom RGB hex colors for ON and OFF states
void setColors(uint32_t colorOn, uint32_t colorOff);

// Manual controller
void setState(bool on);

// Bindings
void bindState(bool* ptr);
```

### DashMap
```cpp
// Update coordinate bindings
void bindCoordinates(double* lat, double* lon);
void setCoordinates(double lat, double lon); // Manual coordinate mode

// Map styling
void setTheme(MapTheme theme); 
// Themes: MapTheme::DarkMatter, MapTheme::Positron, MapTheme::Voyager, MapTheme::Satellite, MapTheme::Terrain

// Marker styling
void setMarker(MarkerStyle style);
// Styles: MarkerStyle::Circle, MarkerStyle::Pin, MarkerStyle::Car, MarkerStyle::Truck, MarkerStyle::Motorcycle, MarkerStyle::Bicycle, MarkerStyle::Drone, MarkerStyle::Boat, MarkerStyle::Aircraft

void setHeadingOffset(float degrees); // Rotate offset for custom icon orientations
void setMarkerScale(float scale);     // Clamped [0.25 to 4.0]

// Heading rotation (CSS transforms)
void bindHeading(float* heading);
void setHeading(float heading);

// Breadcrumb history trails
void enableTrail(bool enabled);
void setTrailLength(uint16_t length); // Clamped [2 to 500]

// Map UI Controls
void setZoom(uint8_t zoom);           // Clamped [1 to 19]
void setZoomControls(bool enabled);   // Show/hide +/- buttons
void setFollow(bool enabled);         // Keep marker centered
void setFullscreen(bool enabled);     // Enable fullscreen button
void setScale(bool enabled);          // Show distance scale
void enableLayerSwitcher(bool enabled); // Enable map theme layers selector in browser
```

### Heading / Marker Rotation

The map widget supports **real-time marker rotation** to visually indicate the heading (front-facing direction) of a moving object such as a drone, car, aircraft, or boat. Rotation is performed entirely in the browser via CSS transforms — no marker recreation occurs when only the heading changes.

#### Heading Values

The heading represents **degrees clockwise from true north**:

| Value | Direction |
|------:|-----------|
| `0°`  | North     |
| `90°` | East      |
| `180°`| South     |
| `270°`| West      |

Any input value is automatically normalized to the `[0, 360)` range (negative values and values ≥ 360 are handled correctly).

#### Shortest-Path Rotation

The browser always rotates the marker along the **shortest angular path**. A heading change from `359° → 1°` rotates **+2°** (not −358°), and `1° → 359°` rotates **−2°**. This prevents jarring full-circle spins when crossing north.

#### API Reference

* **`void bindHeading(float* heading)`**
  Binds a pointer to a heading variable for automatic polling. The library reads the value each update cycle, normalizes it, and sends delta packets when the heading changes by more than 1°.

* **`void setHeading(float heading)`**
  Manually sets the heading value (switching to manual mode). Pointer updates are suspended until `bindHeading()` is called again.

* **`void setHeadingOffset(float degrees)`**
  Applies a rotation offset to compensate for SVG assets whose forward direction is not aligned with north. The final displayed rotation is:
  ```
  displayRotation = normalize(heading + headingOffset)
  ```
  For example, if your icon faces **east** by default, set `setHeadingOffset(-90.0f)`.

* **`void enableRotation(bool enabled)`**
  Enables or disables the visual rotation of the marker (default: `true`). When disabled, heading values continue to update internally and are transmitted in delta packets, but the marker remains visually fixed. Useful when a vehicle icon is desired without directional rotation (e.g. a parked car).

* **`void enableAutoBearing(bool enabled)`**
  Enables automatic heading computation from consecutive GPS coordinates (default: `false`). When enabled and no heading pointer is bound, the widget computes the compass bearing between the previous and current coordinates and uses it as the heading. Movements smaller than a minimum epsilon are ignored to prevent jitter. If a heading pointer is bound or `setHeading()` is called, the explicit heading takes priority.

#### Supported Marker Types

| Marker Style | Supports Rotation |
|---|:---:|
| `Car`, `Truck`, `Motorcycle`, `Bicycle` | ✅ |
| `Drone`, `Aircraft`, `Boat` | ✅ |
| `Circle`, `Pin` | ❌ (heading ignored) |

#### Rotating Drone Marker Example

```cpp
#include <ESP32Dashboard.h>

ESP32Dashboard dashboard;

double lat = 37.7749, lon = -122.4194;
float heading = 0.0f;

void setup() {
    Serial.begin(115200);
    dashboard.begin("YourSSID", "YourPassword");

    auto* map = dashboard.addMap(&lat, &lon, "Drone");
    map->setMarker(MarkerStyle::Drone);
    map->bindHeading(&heading);
    map->setMarkerScale(1.5f);
    map->setFollow(true);
}

void loop() {
    heading += 2.0f;
    if (heading >= 360.0f) heading = 0.0f;

    dashboard.update();
    delay(50);
}
```

#### Auto-Bearing GPS Tracker Example

```cpp
#include <ESP32Dashboard.h>

ESP32Dashboard dashboard;

double lat = 37.7749, lon = -122.4194;

void setup() {
    Serial.begin(115200);
    dashboard.begin("YourSSID", "YourPassword");

    auto* map = dashboard.addMap(&lat, &lon, "Vehicle");
    map->setMarker(MarkerStyle::Car);
    map->enableAutoBearing(true);   // heading from GPS movement
    map->enableTrail(true);
    map->setFollow(true);
}

void loop() {
    // Read GPS sensor...
    // lat = gps.lat;
    // lon = gps.lon;

    dashboard.update();
    delay(100);
}
```

The marker rotates smoothly in the browser at ~180ms CSS transition with cubic-bezier easing — fast enough for real-time tracking without visual jitter.

### DashText
```cpp
void setText(const char* text);   // Up to DASH_TEXT_MAX_LEN characters
const char* getText() const;
```

### DashStatus
```cpp
void setStatus(const char* label, StatusLevel level = StatusLevel::Info);
// Levels: StatusLevel::OK (green), StatusLevel::Warning (amber), StatusLevel::Error (red), StatusLevel::Info (blue)

const char* getLabel() const;
StatusLevel getLevel() const;
```

---

## Configuration Overrides
You can adjust compile-time parameters by `#define`ing constants **before** including `ESP32Dashboard.h`:

```cpp
#define DASH_MAX_WIDGETS 30             // Max widgets (Default: 20)
#define DASH_TITLE_MAX_LEN 48           // Widget title length (Default: 32)
#define DASH_UNIT_MAX_LEN 12            // Unit string length (Default: 16)
#define DASH_TEXT_MAX_LEN 256           // Text widget buffer size (Default: 128)
#define DASH_STATUS_LABEL_MAX_LEN 32    // Status text buffer size (Default: 48)
#define DASH_JSON_BUFFER_SIZE 4096      // Serialization buffer size (Default: 2048)
#define DASH_MAX_WS_CLIENTS 8           // Max WebSocket connections (Default: 4)
#define DASH_UPDATE_INTERVAL_MS 50      // Websocket poll interval (Default: 100)
#define DASH_FULL_SYNC_INTERVAL_MS 8000 // Full state push rate (Default: 5000)
#define DASH_DEBUG 0                    // Disable Serial debug logs (Default: 1)

#include <ESP32Dashboard.h>
```

---

## Example Sketches
Find the following examples in the **File** → **Examples** → **ESP32Dashboard** menu inside the Arduino IDE:

1. **`BasicDashboard`**: Basic setup with 2 cards, 1 gauge, and 1 LED using simulated variables.
2. **`Cards`**: Exercises pointer bindings and manual updates across multiple card datatypes.
3. **`Gauges`**: Demonstrates range modifications, variable binding, and underflow/overflow safety.
4. **`LEDs`**: Sets up status LEDs, showcases color modification, blinking, and manual toggling.
5. **`TextWidget`**: Demonstrates formatting, Unicode characters, and JSON string escaping on multiline fields.
6. **`StatusWidget`**: Customizes system status badges using the `StatusLevel` severity indicators.
7. **`Maps`**: The full GPS mapping suite: trails, rotation, markers, follow modes, and layers.
8. **`ManualUpdates`**: Shows how to construct an event-driven dashboard without binding variables.
9. **`PointerBinding`**: Highlights pointer binding to eliminate code overhead in standard routines.
10. **`MultipleWidgets`**: Demonstrates handling up to 25 widgets and overriding default registration maximums.
11. **`FullDashboard`**: The flagship library example showing all widgets cooperating on simulated data.

---

## Performance Considerations
- **Non-Blocking Loop:** `dashboard.update()` takes only a fraction of a millisecond to execute when no variables have changed. Do not use blocking delay() calls in loop(), as they will stall the WebSocket server.
- **Epsilon Comparators:** The map widget uses epsilon thresholds (e.g. `kCoordEpsilon = 0.0001`) to avoid streaming updates when coordinates drift by fractions of a millimeter.
- **Delta-Compression:** Delta packets containing only the ID and new value are sent over WebSockets. The HTML, CSS, and JS files are only transferred once on browser connect via HTTP.

---

## Memory Considerations
- **Zero Dynamic Allocation (Widgets):** Widget elements are placed in a pre-allocated array of size `DASH_MAX_WIDGETS` (allocated once during startup). No heap fragmentation occurs during runtime.
- **Gzip Page Assets:** Page assets are pre-compressed and served as raw byte arrays, saving valuable flash memory.
- **JSON Buffer:** Increase `DASH_JSON_BUFFER_SIZE` if you use more than 20 widgets or large text blocks to avoid JSON truncation during synchronization.

---

## Browser Compatibility
Supports all modern web browsers complying with HTML5, ES6, WebSockets, and CSS Grid:
- Google Chrome / Chromium
- Mozilla Firefox
- Apple Safari
- Microsoft Edge
- Mobile browsers (Safari iOS, Chrome Android)

---

## Troubleshooting
- **Compiler errors on 'min' or 'max':** Ensure that you do not define custom macro conflicts. Use `std::min` or `std::max` in your code.
- **IP Address displays as '0.0.0.0':** The ESP32 failed to associate with your Wi-Fi access point. Check credentials and power.
- **Web page loads but widgets do not show up:** WebSocket port (default 81) might be blocked on your network or security router. Make sure ports 80 and 81 are accessible.
- **JSON serialization error / warning logs:** Check if your `DASH_JSON_BUFFER_SIZE` is too small. If using many widgets, override this variable to `4096`.

---

## Frequently Asked Questions (FAQ)
* **Q: Can I run this in Access Point (AP) Mode?**
  *A: Yes, you can initialize WiFi in AP mode (`WiFi.softAP(...)`) and call `dashboard.begin(nullptr, nullptr)` or use network interfaces manually. The HTTP/WebSocket listeners will bind to whatever local IP the ESP32 receives.*
* **Q: Can I customize the look of the dashboard?**
  *A: The UI features a glassmorphic dark layout out-of-the-box. Custom CSS styling is compiled inside the library's `PROGMEM` headers.*
* **Q: How many clients can open the page concurrently?**
  *A: Default is 4 clients, customizable up to 8 using `DASH_MAX_WS_CLIENTS` in config overrides.*

---

## Best Practices
1. **Prefer Pointer Binding:** Use pointer binding for sensor variables that cycle continuously. It minimizes boilerplate code.
2. **Use Manual Mode for Events:** For logging, status pills, or rare operations, use manual mode (`setValue()`, `setStatus()`) to prevent repetitive delta checks.
3. **Run Update Frequently:** Call `dashboard.update()` at the root of `loop()`. Keep loop iterations below 10ms.
4. **Always verify returned pointers:** Guard against configuration limits by verifying widget additions are not null (e.g. `if (myGauge != nullptr)`).

---

## Contribution Guidelines
1. Fork this repository.
2. Create your feature branch (`git checkout -b feature/AmazingFeature`).
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`).
4. Push to the branch (`git push origin feature/AmazingFeature`).
5. Open a Pull Request.

---

## License
Distributed under the MIT License. See [LICENSE](LICENSE) for details.
