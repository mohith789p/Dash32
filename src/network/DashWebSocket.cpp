/**
 * @file DashWebSocket.cpp
 * @brief WebSocket server implementation.
 */

#include "DashWebSocket.h"
#include "../core/DashDebug.h"

#include <WebSocketsServer.h>

namespace dash {

// Static member
DashWebSocket* DashWebSocket::_instance = nullptr;

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

DashWebSocket::DashWebSocket()
    : _wsServer(nullptr)
    , _clientCount(0)
    , _running(false)
    , _onConnect(nullptr)
{
}

DashWebSocket::~DashWebSocket() {
    if (_wsServer) {
        delete _wsServer;
        _wsServer = nullptr;
    }
    if (_instance == this) {
        _instance = nullptr;
    }
    _running = false;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void DashWebSocket::begin(uint16_t port) {
    if (_running) {
        DASH_LOG_WARN("WebSocket server already running");
        return;
    }

    _wsServer = new WebSocketsServer(port);
    if (!_wsServer) {
        DASH_LOG_ERROR("Failed to allocate WebSocketsServer");
        return;
    }

    _instance = this;
    _wsServer->begin();
    _wsServer->onEvent(webSocketEvent);

    _running = true;
    DASH_LOG("WebSocket server started on port %u", port);
}

void DashWebSocket::loop() {
    if (_wsServer && _running) {
        _wsServer->loop();
    }
}

// ---------------------------------------------------------------------------
// Data transmission
// ---------------------------------------------------------------------------

void DashWebSocket::broadcast(const char* data, size_t len) {
    if (!_wsServer || !_running || _clientCount == 0) return;
    if (!data || len == 0) return;

    _wsServer->broadcastTXT(reinterpret_cast<const uint8_t*>(data), len);
}

void DashWebSocket::sendToClient(uint8_t clientId, const char* data,
                                 size_t len) {
    if (!_wsServer || !_running) return;
    if (!data || len == 0) return;

    _wsServer->sendTXT(clientId, reinterpret_cast<const uint8_t*>(data), len);
}

// ---------------------------------------------------------------------------
// Event handler
// ---------------------------------------------------------------------------

void DashWebSocket::webSocketEvent(uint8_t num, uint8_t type,
                                   uint8_t* payload, size_t length) {
    (void)payload;
    (void)length;

    if (!_instance) return;

    switch (type) {
        case WStype_CONNECTED:
            _instance->_clientCount++;
            DASH_LOG("WS client #%u connected (total: %u)",
                     num, _instance->_clientCount);

            // Notify dashboard to send full config to this client
            if (_instance->_onConnect) {
                _instance->_onConnect(num);
            }
            break;

        case WStype_DISCONNECTED:
            if (_instance->_clientCount > 0) {
                _instance->_clientCount--;
            }
            DASH_LOG("WS client #%u disconnected (total: %u)",
                     num, _instance->_clientCount);
            break;

        case WStype_TEXT:
            // Future: handle client→server messages (e.g., button clicks)
            // For now, we don't expect client messages.
            DASH_LOG("WS received text from #%u (ignored, %u bytes)",
                     num, (unsigned)length);
            break;

        case WStype_ERROR:
            DASH_LOG_ERROR("WS error on client #%u", num);
            break;

        default:
            break;
    }
}

} // namespace dash
