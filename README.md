# ESP32Dashboard

**Zero-frontend real-time monitoring dashboard for ESP32.**

Build professional IoT dashboards with pure C++ — no HTML, CSS, or JavaScript required.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-green.svg)]()
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal.svg)]()

---

## Features

- 🔌 **Zero Frontend Code** — Write only C++, get a professional dashboard
- 📡 **Real-Time Updates** — WebSocket-based streaming with delta compression
- 🎨 **Dark Theme UI** — Modern glassmorphism design, responsive layout
- 🧩 **Widget System** — Cards, Gauges, Maps, LEDs, Text, Status indicators
- ⚡ **Memory Efficient** — Fixed buffers, minimal heap allocation, PROGMEM assets
- 🔗 **Dual Value Mode** — Bind to pointers (automatic) or set values manually
- 🗺️ **Leaflet Maps** — Live GPS tracking with dark map tiles
- 📊 **Extensible** — Add custom widgets via the base class

## Quick Start

```cpp
#include <ESP32Dashboard.h>

ESP32Dashboard dashboard;

float temperature = 22.5;
int humidity = 65;
float speed = 0.0;

void setup() {
    Serial.begin(115200);
    dashboard.begin("YourSSID", "YourPassword");

    dashboard.addCard("Temperature", &temperature, "°C");
    dashboard.addCard("Humidity", &humidity, "%");

    auto* gauge = dashboard.addGauge("Speed", &speed, "km/h");
    gauge->setRange(0, 200);
}

void loop() {
    temperature = readTemperature();
    humidity = readHumidity();
    speed = readSpeed();

    dashboard.update();  // Non-blocking
}
```

Open `http://<ESP32_IP>` in your browser. That's it.

## Installation

### Arduino Library Manager (Recommended)
1. Open Arduino IDE → Sketch → Include Library → Manage Libraries
2. Search for "ESP32Dashboard"
3. Click Install

### Manual Installation
1. Download or clone this repository
2. Copy the folder to your Arduino libraries directory
3. Restart the Arduino IDE

### Dependencies
- [WebSockets](https://github.com/Links2004/arduinoWebSockets) by Markus Sattler

## Widget Types

| Widget | Description | Pointer Mode | Manual Mode |
|--------|-------------|:------------:|:-----------:|
| **Card** | Displays a labeled value with unit | ✅ | ✅ |
| **Gauge** | Circular arc gauge with min/max range | ✅ | ✅ |
| **Map** | Leaflet map with GPS marker | ✅ | ✅ |
| **LED** | On/off indicator with custom colors | ✅ | ✅ |
| **Text** | Multi-line text display | — | ✅ |
| **Status** | Severity badge (OK/Warning/Error/Info) | — | ✅ |

## API Reference

### ESP32Dashboard

```cpp
// Lifecycle
bool begin(const char* ssid, const char* password,
           uint16_t httpPort = 80, uint16_t wsPort = 81);
void update();

// Cards
DashCard* addCard(const char* title, float* value, const char* unit = "");
DashCard* addCard(const char* title, int* value, const char* unit = "");
DashCard* addCard(const char* title, const char* unit = "");
bool setCardValue(const char* title, float value);

// Gauges
DashGauge* addGauge(const char* title, float* value, const char* unit = "");

// Map
DashMap* addMap(double* lat, double* lon, const char* title = "Location");

// LED
DashLED* addLED(const char* title, bool* state = nullptr);

// Text & Status
DashText* addText(const char* title);
DashStatus* addStatus(const char* title);
```

### DashGauge

```cpp
gauge->setRange(0, 200);  // Set min/max for the arc
```

### DashLED

```cpp
led->setColors(0x00FF88, 0x444444);  // Green on, gray off
led->setState(true);                  // Manual mode
```

### DashStatus

```cpp
status->setStatus("Connected", StatusLevel::OK);
status->setStatus("Reconnecting...", StatusLevel::Warning);
status->setStatus("Failed", StatusLevel::Error);
```

## Configuration

Override defaults before including the library:

```cpp
#define DASH_MAX_WIDGETS 30           // Default: 20
#define DASH_UPDATE_INTERVAL_MS 50    // Default: 100ms
#define DASH_JSON_BUFFER_SIZE 4096    // Default: 2048
#define DASH_DEBUG 0                  // Disable debug output
#include <ESP32Dashboard.h>
```

## Architecture

```
src/
├── ESP32Dashboard.h/.cpp       # Public API & orchestrator
├── core/
│   ├── DashConfig.h            # Compile-time constants
│   ├── DashTypes.h             # Enums & utilities
│   ├── DashDebug.h             # Debug macros
│   └── ValueHolder.h/.cpp      # Type-erased value container
├── widgets/
│   ├── DashWidget.h/.cpp       # Abstract base class
│   ├── DashCard.h/.cpp
│   ├── DashGauge.h/.cpp
│   ├── DashMap.h/.cpp
│   ├── DashLED.h/.cpp
│   ├── DashText.h/.cpp
│   └── DashStatus.h/.cpp
├── network/
│   ├── DashWebServer.h/.cpp    # HTTP server
│   └── DashWebSocket.h/.cpp    # WebSocket manager
├── data/
│   └── DashJsonEngine.h/.cpp   # JSON serializer
└── ui/
    ├── DashPageHTML.h           # Embedded HTML (PROGMEM)
    ├── DashPageCSS.h            # Embedded CSS (PROGMEM)
    └── DashPageJS.h             # Embedded JS (PROGMEM)
```

## Memory Usage

| Resource | Typical Usage |
|----------|--------------|
| Flash | ~70 KB (including frontend assets) |
| RAM (static) | ~3 KB |
| RAM per widget | ~80 bytes |
| JSON buffer | 2 KB (configurable) |

## License

MIT License — see [LICENSE](LICENSE) for details.
