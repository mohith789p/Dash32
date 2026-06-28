/**
 * @file ESP32Dashboard.h
 * @brief Public API — single include for the ESP32Dashboard library.
 *
 * This is the only header a user needs to include. It provides the
 * ESP32Dashboard class which orchestrates all internal subsystems:
 * WiFi, HTTP server, WebSocket server, widget management, and
 * JSON serialization.
 *
 * @example BasicDashboard.ino
 * @code
 *   #include <ESP32Dashboard.h>
 *
 *   ESP32Dashboard dashboard;
 *   float temp = 22.5;
 *   int humidity = 65;
 *
 *   void setup() {
 *       Serial.begin(115200);
 *       dashboard.begin("MySSID", "MyPassword");
 *       dashboard.addCard("Temperature", &temp, "°C");
 *       dashboard.addCard("Humidity", &humidity, "%");
 *   }
 *
 *   void loop() {
 *       temp = analogRead(34) * 0.1;
 *       dashboard.update();
 *   }
 * @endcode
 */

#ifndef ESP32_DASHBOARD_H
#define ESP32_DASHBOARD_H

#include "core/DashConfig.h"
#include "core/DashTypes.h"
#include "widgets/DashCard.h"
#include "widgets/DashGauge.h"
#include "widgets/DashMap.h"
#include "widgets/DashLED.h"
#include "widgets/DashText.h"
#include "widgets/DashStatus.h"
#include "widgets/DashImage.h"
#include "widgets/DashVideo.h"
#include "network/DashWebServer.h"
#include "network/DashWebSocket.h"
#include "data/DashJsonEngine.h"

// Re-export enums and classes for convenience
using dash::StatusLevel;
using dash::MapTheme;
using dash::MarkerStyle;
using dash::ImageFit;
using dash::DashImage;
using dash::DashVideo;

class ESP32Dashboard {
public:
    ESP32Dashboard();
    ~ESP32Dashboard();

    // Non-copyable
    ESP32Dashboard(const ESP32Dashboard&) = delete;
    ESP32Dashboard& operator=(const ESP32Dashboard&) = delete;

    // =================================================================
    // Lifecycle
    // =================================================================

    /**
     * @brief Initialize WiFi, HTTP server, and WebSocket server.
     *
     * Connects to the specified WiFi network and starts all servers.
     * Blocks until WiFi is connected (with timeout).
     *
     * @param ssid     WiFi network name.
     * @param password WiFi password.
     * @param httpPort HTTP server port (default: 80).
     * @param wsPort   WebSocket server port (default: 81).
     * @return true if WiFi connected and servers started successfully.
     */
    bool begin(const char* ssid, const char* password,
               uint16_t httpPort = DASH_HTTP_PORT,
               uint16_t wsPort = DASH_WS_PORT);

    bool begin(const char* ssid, const char* password, const char* hostname,
               uint16_t httpPort = DASH_HTTP_PORT,
               uint16_t wsPort = DASH_WS_PORT);

    /**
     * @brief Main loop function — call this in your loop().
     *
     * Handles HTTP requests, WebSocket events, checks for value changes,
     * and broadcasts delta updates to connected clients.
     *
     * This function is non-blocking and designed to be called at
     * high frequency.
     */
    void update();

    /**
     * @brief Set the dashboard title displayed on the web interface.
     * @param title The new dashboard title.
     */
    void setTitle(const char* title);

    // =================================================================
    // Card Widget
    // =================================================================

    /**
     * @brief Add a card widget bound to a float pointer.
     * @param title Display title.
     * @param value Pointer to the float variable.
     * @param unit  Unit string (e.g., "°C").
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashCard* addCard(const char* title, float* value,
                            const char* unit = "");

    /**
     * @brief Add a card widget bound to a double pointer.
     */
    dash::DashCard* addCard(const char* title, double* value,
                            const char* unit = "");

    /**
     * @brief Add a card widget bound to an int pointer.
     */
    dash::DashCard* addCard(const char* title, int* value,
                            const char* unit = "");

    /**
     * @brief Add a card widget bound to a long pointer.
     */
    dash::DashCard* addCard(const char* title, long* value,
                            const char* unit = "");

    /**
     * @brief Add a card widget bound to a bool pointer.
     */
    dash::DashCard* addCard(const char* title, bool* value,
                            const char* unit = "");

    /**
     * @brief Add a card widget in manual mode (no pointer binding).
     * @param title Display title.
     * @param unit  Unit string.
     * @return Pointer to the created widget.
     */
    dash::DashCard* addCard(const char* title, const char* unit = "");

    /**
     * @brief Set a card's value by title (manual mode convenience).
     * @param title The card title to search for.
     * @param value The new value.
     * @return true if the card was found and updated.
     */
    bool setCardValue(const char* title, float value);
    bool setCardValue(const char* title, int value);
    bool setCardValue(const char* title, const char* value);

    // =================================================================
    // Gauge Widget
    // =================================================================

    /**
     * @brief Add a gauge widget bound to a float pointer.
     * @param title Display title.
     * @param value Pointer to the float variable.
     * @param unit  Unit string.
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashGauge* addGauge(const char* title, float* value,
                              const char* unit = "");

    /** @brief Add a gauge bound to an int pointer. */
    dash::DashGauge* addGauge(const char* title, int* value,
                              const char* unit = "");

    /** @brief Add a gauge in manual mode. */
    dash::DashGauge* addGauge(const char* title, const char* unit = "");

    // =================================================================
    // Map Widget
    // =================================================================

    /**
     * @brief Add a map widget bound to lat/lon pointers.
     * @param lat Pointer to latitude (double).
     * @param lon Pointer to longitude (double).
     * @param title Display title (default: "Location").
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashMap* addMap(double* lat, double* lon,
                          const char* title = "Location");

    /** @brief Add a map in manual mode. */
    dash::DashMap* addMap(const char* title = "Location");

    // =================================================================
    // LED Widget
    // =================================================================

    /**
     * @brief Add an LED indicator bound to a bool pointer.
     * @param title Display label.
     * @param state Pointer to a bool variable.
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashLED* addLED(const char* title, bool* state = nullptr);

    // =================================================================
    // Text Widget
    // =================================================================

    /**
     * @brief Add a text display widget.
     * @param title Display title.
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashText* addText(const char* title);

    // =================================================================
    // Status Widget
    // =================================================================

    /**
     * @brief Add a status indicator widget.
     * @param title Display title.
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashStatus* addStatus(const char* title);

    // =================================================================
    // Image Widget
    // =================================================================

    /**
     * @brief Add an image widget.
     * @param title Display title.
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashImage* addImage(const char* title);

    // =================================================================
    // Video Widget
    // =================================================================

    /**
     * @brief Add a video streaming widget.
     * @param title Display title.
     * @return Pointer to the created widget, or nullptr if full.
     */
    dash::DashVideo* addVideo(const char* title);

    // =================================================================
    // Queries
    // =================================================================

    /** @brief Get the number of registered widgets. */
    uint8_t getWidgetCount() const { return _widgetCount; }

    /** @brief Get the number of connected WebSocket clients. */
    uint8_t getClientCount() const;

    /** @brief Get the dashboard IP address as a string. */
    const char* getIPAddress() const;

private:
    // Subsystems (composition)
    dash::DashWebServer  _httpServer;
    dash::DashWebSocket  _wsServer;
    dash::DashJsonEngine _jsonEngine;

    // Widget storage (fixed array, no heap allocation for the array itself)
    dash::DashWidget* _widgets[DASH_MAX_WIDGETS];
    uint8_t           _widgetCount;

    // Timing
    uint32_t _lastDeltaMs;
    uint32_t _lastFullSyncMs;

    // State
    bool     _started;
    char     _ipStr[16]; ///< Cached IP address string
    char     _title[64]; ///< Configurable dashboard title
    char     _hostname[64]; ///< Sanitized hostname
    uint16_t _httpPort;
    uint16_t _wsPort;

    // JSON buffer (allocated once on heap at begin())
    char*    _jsonBuf;
    size_t   _jsonBufSize;

    // Internal helpers

    /**
     * @brief Register a widget in the array.
     * @param widget The widget to register.
     * @return true if registered, false if array is full.
     */
    bool registerWidget(dash::DashWidget* widget);

    /**
     * @brief Find a card widget by title.
     * @param title The title to search for.
     * @return Pointer to the card, or nullptr if not found.
     */
    dash::DashCard* findCard(const char* title) const;

    /**
     * @brief Check all widgets for changes.
     * @return Number of widgets that changed.
     */
    uint8_t checkAllChanges();

    /** @brief Send full config to a specific client (on connect). */
    void sendConfigToClient(uint8_t clientId);

    /** @brief Broadcast delta update to all clients. */
    void broadcastDelta();

    /** @brief Broadcast full update to all clients. */
    void broadcastFullUpdate();

    /** @brief Broadcast full configuration to all clients. */
    void broadcastConfig();

    /** @brief Clear dirty flags on all widgets. */
    void clearAllDirty();

    /** @brief Get the current system metadata and stats. */
    dash::DashboardSysInfo getSysInfo() const;

    /**
     * @brief Static callback for WebSocket connect events.
     * Routes to the active instance's sendConfigToClient().
     */
    static void onWsConnect(uint8_t clientId);

    /** @brief Static instance pointer for callback routing. */
    static ESP32Dashboard* _instance;
};

#endif // ESP32_DASHBOARD_H
