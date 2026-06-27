/**
 * @file DashWebServer.h
 * @brief HTTP server wrapper for serving the embedded dashboard page.
 *
 * Wraps ESP32 WebServer to serve the single-page dashboard application
 * from PROGMEM. Handles HTTP routing and gzip content encoding.
 */

#ifndef DASH_WEB_SERVER_H
#define DASH_WEB_SERVER_H

#include "../core/DashConfig.h"
#include <cstdint>

// Forward declaration to avoid pulling in WebServer.h in the header
class WebServer;

namespace dash {

class DashWebServer {
public:
    DashWebServer();
    ~DashWebServer();

    // Non-copyable
    DashWebServer(const DashWebServer&) = delete;
    DashWebServer& operator=(const DashWebServer&) = delete;

    /**
     * @brief Start the HTTP server.
     * @param port HTTP port (default: DASH_HTTP_PORT).
     */
    void begin(uint16_t port = DASH_HTTP_PORT);

    /**
     * @brief Process pending HTTP requests (non-blocking).
     * Must be called frequently from loop().
     */
    void handleClient();

    /**
     * @brief Check if the server has been started.
     */
    bool isRunning() const { return _running; }

private:
    WebServer* _server;  ///< Pointer to avoid including WebServer.h
    bool       _running;

    /** @brief Register all HTTP routes. */
    void setupRoutes();

    /** @brief Handler for GET / — serves the dashboard HTML page. */
    static void handleRoot();

    /** @brief Handler for GET /style.css — serves embedded CSS. */
    static void handleCSS();

    /** @brief Handler for GET /app.js — serves embedded JavaScript. */
    static void handleJS();

    /** @brief Handler for 404. */
    static void handleNotFound();

    /**
     * @brief Static pointer to the active server instance.
     * Required because WebServer callbacks are C-style function pointers.
     * Only one DashWebServer should exist at a time.
     */
    static WebServer* _activeServer;
};

} // namespace dash

#endif // DASH_WEB_SERVER_H
