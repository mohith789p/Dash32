/**
 * @file DashWebServer.cpp
 * @brief HTTP server implementation serving embedded dashboard assets.
 */

#include "DashWebServer.h"
#include "../core/DashDebug.h"
#include "../ui/DashPageHTML.h"
#include "../ui/DashPageCSS.h"
#include "../ui/DashPageJS.h"

#include <WebServer.h>

namespace dash {

// Static member
WebServer* DashWebServer::_activeServer = nullptr;

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

DashWebServer::DashWebServer()
    : _server(nullptr)
    , _running(false)
{
}

DashWebServer::~DashWebServer() {
    if (_server) {
        _server->stop();
        delete _server;
        _server = nullptr;
    }
    if (_activeServer == _server) {
        _activeServer = nullptr;
    }
    _running = false;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void DashWebServer::begin(uint16_t port) {
    if (_running) {
        DASH_LOG_WARN("HTTP server already running");
        return;
    }

    _server = new WebServer(port);
    if (!_server) {
        DASH_LOG_ERROR("Failed to allocate WebServer");
        return;
    }

    _activeServer = _server;
    setupRoutes();
    _server->begin();
    _running = true;

    DASH_LOG("HTTP server started on port %u", port);
}

void DashWebServer::handleClient() {
    if (_server && _running) {
        _server->handleClient();
    }
}

// ---------------------------------------------------------------------------
// Route setup
// ---------------------------------------------------------------------------

void DashWebServer::setupRoutes() {
    _server->on("/", HTTP_GET, handleRoot);
    _server->on("/style.css", HTTP_GET, handleCSS);
    _server->on("/app.js", HTTP_GET, handleJS);
    _server->onNotFound(handleNotFound);
}

// ---------------------------------------------------------------------------
// Route handlers
// ---------------------------------------------------------------------------

void DashWebServer::handleRoot() {
    if (!_activeServer) return;
    _activeServer->send_P(200, "text/html", DASH_HTML, DASH_HTML_SIZE);
}

void DashWebServer::handleCSS() {
    if (!_activeServer) return;
    _activeServer->send_P(200, "text/css", DASH_CSS, DASH_CSS_SIZE);
}

void DashWebServer::handleJS() {
    if (!_activeServer) return;
    _activeServer->send_P(200, "application/javascript", DASH_JS, DASH_JS_SIZE);
}

void DashWebServer::handleNotFound() {
    if (!_activeServer) return;
    _activeServer->send(404, "text/plain", "Not Found");
}

} // namespace dash
