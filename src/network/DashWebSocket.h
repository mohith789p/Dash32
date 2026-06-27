/**
 * @file DashWebSocket.h
 * @brief WebSocket server manager for real-time dashboard updates.
 *
 * Wraps the WebSocketsServer library (Links2004) to handle:
 * - Client connect/disconnect events
 * - Broadcasting JSON updates to all connected clients
 * - Client count tracking
 * - Connection event callbacks
 */

#ifndef DASH_WEB_SOCKET_H
#define DASH_WEB_SOCKET_H

#include "../core/DashConfig.h"
#include <cstdint>
#include <cstddef>

// Forward declaration
class WebSocketsServer;

namespace dash {

/**
 * @brief Callback type for new client connections.
 *
 * When a new client connects, the dashboard sends the full config.
 * This callback is invoked by DashWebSocket to request that data.
 */
using OnClientConnectCallback = void (*)(uint8_t clientId);

class DashWebSocket {
public:
    DashWebSocket();
    ~DashWebSocket();

    // Non-copyable
    DashWebSocket(const DashWebSocket&) = delete;
    DashWebSocket& operator=(const DashWebSocket&) = delete;

    /**
     * @brief Start the WebSocket server.
     * @param port WebSocket port (default: DASH_WS_PORT).
     */
    void begin(uint16_t port = DASH_WS_PORT);

    /**
     * @brief Process WebSocket events (non-blocking).
     * Must be called frequently from loop().
     */
    void loop();

    /**
     * @brief Broadcast data to all connected clients.
     * @param data  Null-terminated string to send.
     * @param len   Length of data (excluding null terminator).
     */
    void broadcast(const char* data, size_t len);

    /**
     * @brief Send data to a specific client.
     * @param clientId Client number.
     * @param data     Null-terminated string.
     * @param len      Data length.
     */
    void sendToClient(uint8_t clientId, const char* data, size_t len);

    /** @brief Get the number of currently connected clients. */
    uint8_t getClientCount() const { return _clientCount; }

    /** @brief Check if any clients are connected. */
    bool hasClients() const { return _clientCount > 0; }

    /**
     * @brief Set callback for new client connections.
     * @param cb Callback function pointer.
     */
    void onClientConnect(OnClientConnectCallback cb) { _onConnect = cb; }

    /** @brief Check if the server is running. */
    bool isRunning() const { return _running; }

private:
    WebSocketsServer*      _wsServer;
    uint8_t                _clientCount;
    bool                   _running;
    OnClientConnectCallback _onConnect;

    /**
     * @brief WebSocket event handler (static because the library uses C callbacks).
     */
    static void webSocketEvent(uint8_t num, uint8_t type,
                               uint8_t* payload, size_t length);

    /** @brief Static pointer for callback routing. */
    static DashWebSocket* _instance;
};

} // namespace dash

#endif // DASH_WEB_SOCKET_H
