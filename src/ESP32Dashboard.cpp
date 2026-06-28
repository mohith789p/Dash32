/**
 * @file ESP32Dashboard.cpp
 * @brief Main orchestrator implementation.
 *
 * Ties together WiFi, HTTP, WebSocket, widgets, and JSON serialization
 * into a single cohesive update loop.
 */

#include "ESP32Dashboard.h"
#include "core/DashDebug.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <cstring>
#include <cctype>

// Helper to sanitize hostname
static void sanitizeHostname(const char* input, char* output, size_t maxLen) {
    if (!input || maxLen == 0) {
        if (maxLen > 0) output[0] = '\0';
        return;
    }

    // Trim leading
    const char* start = input;
    while (*start && std::isspace((unsigned char)*start)) {
        start++;
    }

    // Trim trailing
    const char* end = start + strlen(start);
    while (end > start && std::isspace((unsigned char)*(end - 1))) {
        end--;
    }

    // Copy, replacing internal spaces with '-' and limiting length
    size_t outIdx = 0;
    // RFC 1035 limits label to 63 chars
    size_t limit = (maxLen - 1 < 63) ? (maxLen - 1) : 63;

    for (const char* p = start; p < end && outIdx < limit; p++) {
        char c = *p;
        if (std::isspace((unsigned char)c)) {
            output[outIdx++] = '-';
        } else {
            output[outIdx++] = c;
        }
    }
    output[outIdx] = '\0';
}

// ---------------------------------------------------------------------------
// Static member
// ---------------------------------------------------------------------------

ESP32Dashboard* ESP32Dashboard::_instance = nullptr;

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

ESP32Dashboard::ESP32Dashboard()
    : _widgetCount(0)
    , _lastDeltaMs(0)
    , _lastFullSyncMs(0)
    , _started(false)
    , _jsonBuf(nullptr)
    , _jsonBufSize(DASH_JSON_BUFFER_SIZE)
    , _httpPort(DASH_HTTP_PORT)
    , _wsPort(DASH_WS_PORT)
{
    memset(_widgets, 0, sizeof(_widgets));
    memset(_ipStr, 0, sizeof(_ipStr));
    strncpy(_title, "Dash32 Monitor", sizeof(_title));
    _title[sizeof(_title) - 1] = '\0';
    strncpy(_hostname, "dash32", sizeof(_hostname));
    _hostname[sizeof(_hostname) - 1] = '\0';
    _instance = this;
}

ESP32Dashboard::~ESP32Dashboard() {
    // Clean up dynamically allocated widgets
    for (uint8_t i = 0; i < _widgetCount; ++i) {
        delete _widgets[i];
        _widgets[i] = nullptr;
    }
    // Clean up JSON buffer
    if (_jsonBuf) {
        delete[] _jsonBuf;
        _jsonBuf = nullptr;
    }
    if (_instance == this) {
        _instance = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool ESP32Dashboard::begin(const char* ssid, const char* password,
                           uint16_t httpPort, uint16_t wsPort) {
    return begin(ssid, password, "dash32", httpPort, wsPort);
}

bool ESP32Dashboard::begin(const char* ssid, const char* password, const char* hostname,
                           uint16_t httpPort, uint16_t wsPort) {
    if (_started) {
        DASH_LOG_WARN("Dashboard already started");
        return true;
    }

    // Input validation
    if (!ssid || strlen(ssid) == 0) {
        DASH_LOG_ERROR("begin(): SSID is null or empty");
        return false;
    }

    // Allocate JSON buffer (single heap allocation)
    _jsonBuf = new (std::nothrow) char[_jsonBufSize];
    if (!_jsonBuf) {
        DASH_LOG_ERROR("Failed to allocate JSON buffer (%u bytes)",
                       (unsigned)_jsonBufSize);
        return false;
    }

    // Connect WiFi
    DASH_LOG("Connecting to WiFi: %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    uint32_t startMs = millis();
    const uint32_t timeoutMs = 15000; // 15 second timeout

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startMs > timeoutMs) {
            DASH_LOG_ERROR("WiFi connection timeout after %u ms", timeoutMs);
            return false;
        }
        delay(100);
        yield(); // Prevent watchdog timeout
    }

    // Cache IP address
    IPAddress ip = WiFi.localIP();
    snprintf(_ipStr, sizeof(_ipStr), "%d.%d.%d.%d",
             ip[0], ip[1], ip[2], ip[3]);

    DASH_LOG("WiFi connected! IP: %s", _ipStr);

    // Initialize mDNS
    char sanitized[64];
    sanitizeHostname(hostname, sanitized, sizeof(sanitized));
    if (strlen(sanitized) == 0) {
        strncpy(sanitized, "dash32", sizeof(sanitized));
        sanitized[sizeof(sanitized) - 1] = '\0';
    }
    strncpy(_hostname, sanitized, sizeof(_hostname));
    _hostname[sizeof(_hostname) - 1] = '\0';
    _httpPort = httpPort;
    _wsPort = wsPort;

    if (!MDNS.begin(sanitized)) {
        DASH_LOG_ERROR("Failed to start mDNS!");
    } else {
        MDNS.addService("http", "tcp", httpPort);
        DASH_LOG("mDNS responder started with hostname: %s", sanitized);
    }

    // Start HTTP server
    _httpServer.begin(httpPort);

    // Start WebSocket server with connect callback
    _wsServer.onClientConnect(onWsConnect);
    _wsServer.begin(wsPort);

    _started = true;
    _lastDeltaMs = millis();
    _lastFullSyncMs = millis();

    DASH_LOG("Dashboard ready at http://%s:%u", _ipStr, httpPort);
    DASH_LOG("Widgets registered: %u / %u", _widgetCount, DASH_MAX_WIDGETS);

    return true;
}

void ESP32Dashboard::update() {
    if (!_started) return;

    // Process network events (non-blocking)
    _httpServer.handleClient();
    _wsServer.loop();

    // Skip updates if no clients are connected
    if (!_wsServer.hasClients()) return;

    // Check for config changes
    bool configChanged = false;
    for (uint8_t i = 0; i < _widgetCount; ++i) {
        if (_widgets[i] && _widgets[i]->isConfigDirty()) {
            configChanged = true;
            break;
        }
    }
    if (configChanged) {
        broadcastConfig();
        for (uint8_t i = 0; i < _widgetCount; ++i) {
            if (_widgets[i]) {
                _widgets[i]->clearConfigDirty();
            }
        }
    }

    uint32_t now = millis();

    // Periodic full sync
    if (now - _lastFullSyncMs >= DASH_FULL_SYNC_INTERVAL_MS) {
        _lastFullSyncMs = now;
        broadcastFullUpdate();
        return; // Skip delta check this iteration
    }

    // Rate-limited delta updates
    if (now - _lastDeltaMs >= DASH_UPDATE_INTERVAL_MS) {
        _lastDeltaMs = now;

        static uint32_t lastSysUpdateMs = 0;
        bool sysTimeElapsed = (now - lastSysUpdateMs >= 1000);
        uint8_t changedCount = checkAllChanges();

        if (changedCount > 0 || sysTimeElapsed) {
            if (sysTimeElapsed) {
                lastSysUpdateMs = now;
            }
            broadcastDelta();
            clearAllDirty();
        }
    }
}

// ---------------------------------------------------------------------------
// Widget registration
// ---------------------------------------------------------------------------

bool ESP32Dashboard::registerWidget(dash::DashWidget* widget) {
    if (!widget) return false;

    if (_widgetCount >= DASH_MAX_WIDGETS) {
        DASH_LOG_ERROR("Widget limit reached (%u). Cannot add '%s'",
                       DASH_MAX_WIDGETS, widget->getTitle());
        delete widget;
        return false;
    }

    widget->setId(_widgetCount);
    _widgets[_widgetCount] = widget;
    _widgetCount++;

    DASH_LOG("Widget added: '%s' (id=%u, type=%s)",
             widget->getTitle(), widget->getId(),
             widget->getType() == dash::WidgetType::Card ? "card" :
             widget->getType() == dash::WidgetType::Gauge ? "gauge" :
             widget->getType() == dash::WidgetType::Map ? "map" :
             widget->getType() == dash::WidgetType::LED ? "led" :
             widget->getType() == dash::WidgetType::Text ? "text" :
             widget->getType() == dash::WidgetType::Status ? "status" :
             "unknown");

    return true;
}

// ---------------------------------------------------------------------------
// Card widget factory methods
// ---------------------------------------------------------------------------

dash::DashCard* ESP32Dashboard::addCard(const char* title, float* value,
                                        const char* unit) {
    auto* card = new (std::nothrow) dash::DashCard(title, unit);
    if (!card) { DASH_LOG_ERROR("addCard: allocation failed"); return nullptr; }
    card->bindValue(value);
    if (!registerWidget(card)) return nullptr;
    return card;
}

dash::DashCard* ESP32Dashboard::addCard(const char* title, double* value,
                                        const char* unit) {
    auto* card = new (std::nothrow) dash::DashCard(title, unit);
    if (!card) { DASH_LOG_ERROR("addCard: allocation failed"); return nullptr; }
    card->bindValue(value);
    if (!registerWidget(card)) return nullptr;
    return card;
}

dash::DashCard* ESP32Dashboard::addCard(const char* title, int* value,
                                        const char* unit) {
    auto* card = new (std::nothrow) dash::DashCard(title, unit);
    if (!card) { DASH_LOG_ERROR("addCard: allocation failed"); return nullptr; }
    card->bindValue(value);
    if (!registerWidget(card)) return nullptr;
    return card;
}

dash::DashCard* ESP32Dashboard::addCard(const char* title, long* value,
                                        const char* unit) {
    auto* card = new (std::nothrow) dash::DashCard(title, unit);
    if (!card) { DASH_LOG_ERROR("addCard: allocation failed"); return nullptr; }
    card->bindValue(value);
    if (!registerWidget(card)) return nullptr;
    return card;
}

dash::DashCard* ESP32Dashboard::addCard(const char* title, bool* value,
                                        const char* unit) {
    auto* card = new (std::nothrow) dash::DashCard(title, unit);
    if (!card) { DASH_LOG_ERROR("addCard: allocation failed"); return nullptr; }
    card->bindValue(value);
    if (!registerWidget(card)) return nullptr;
    return card;
}

dash::DashCard* ESP32Dashboard::addCard(const char* title, const char* unit) {
    auto* card = new (std::nothrow) dash::DashCard(title, unit);
    if (!card) { DASH_LOG_ERROR("addCard: allocation failed"); return nullptr; }
    if (!registerWidget(card)) return nullptr;
    return card;
}

// ---------------------------------------------------------------------------
// setCardValue convenience methods
// ---------------------------------------------------------------------------

bool ESP32Dashboard::setCardValue(const char* title, float value) {
    auto* card = findCard(title);
    if (!card) return false;
    card->setValue(value);
    return true;
}

bool ESP32Dashboard::setCardValue(const char* title, int value) {
    auto* card = findCard(title);
    if (!card) return false;
    card->setValue(value);
    return true;
}

bool ESP32Dashboard::setCardValue(const char* title, const char* value) {
    auto* card = findCard(title);
    if (!card) return false;
    card->setValue(value);
    return true;
}

dash::DashCard* ESP32Dashboard::findCard(const char* title) const {
    if (!title) return nullptr;
    for (uint8_t i = 0; i < _widgetCount; ++i) {
        if (_widgets[i] &&
            _widgets[i]->getType() == dash::WidgetType::Card &&
            strncmp(_widgets[i]->getTitle(), title, DASH_TITLE_MAX_LEN) == 0) {
            return static_cast<dash::DashCard*>(_widgets[i]);
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Gauge widget factory methods
// ---------------------------------------------------------------------------

dash::DashGauge* ESP32Dashboard::addGauge(const char* title, float* value,
                                          const char* unit) {
    auto* gauge = new (std::nothrow) dash::DashGauge(title, unit);
    if (!gauge) { DASH_LOG_ERROR("addGauge: allocation failed"); return nullptr; }
    gauge->bindValue(value);
    if (!registerWidget(gauge)) return nullptr;
    return gauge;
}

dash::DashGauge* ESP32Dashboard::addGauge(const char* title, int* value,
                                          const char* unit) {
    auto* gauge = new (std::nothrow) dash::DashGauge(title, unit);
    if (!gauge) { DASH_LOG_ERROR("addGauge: allocation failed"); return nullptr; }
    gauge->bindValue(value);
    if (!registerWidget(gauge)) return nullptr;
    return gauge;
}

dash::DashGauge* ESP32Dashboard::addGauge(const char* title,
                                          const char* unit) {
    auto* gauge = new (std::nothrow) dash::DashGauge(title, unit);
    if (!gauge) { DASH_LOG_ERROR("addGauge: allocation failed"); return nullptr; }
    if (!registerWidget(gauge)) return nullptr;
    return gauge;
}

// ---------------------------------------------------------------------------
// Map widget factory
// ---------------------------------------------------------------------------

dash::DashMap* ESP32Dashboard::addMap(double* lat, double* lon,
                                      const char* title) {
    auto* map = new (std::nothrow) dash::DashMap(title, lat, lon);
    if (!map) { DASH_LOG_ERROR("addMap: allocation failed"); return nullptr; }
    if (!registerWidget(map)) return nullptr;
    return map;
}

dash::DashMap* ESP32Dashboard::addMap(const char* title) {
    auto* map = new (std::nothrow) dash::DashMap(title, nullptr, nullptr);
    if (!map) { DASH_LOG_ERROR("addMap: allocation failed"); return nullptr; }
    if (!registerWidget(map)) return nullptr;
    return map;
}

// ---------------------------------------------------------------------------
// LED widget factory
// ---------------------------------------------------------------------------

dash::DashLED* ESP32Dashboard::addLED(const char* title, bool* state) {
    auto* led = new (std::nothrow) dash::DashLED(title, state);
    if (!led) { DASH_LOG_ERROR("addLED: allocation failed"); return nullptr; }
    if (!registerWidget(led)) return nullptr;
    return led;
}

// ---------------------------------------------------------------------------
// Text widget factory
// ---------------------------------------------------------------------------

dash::DashText* ESP32Dashboard::addText(const char* title) {
    auto* text = new (std::nothrow) dash::DashText(title);
    if (!text) { DASH_LOG_ERROR("addText: allocation failed"); return nullptr; }
    if (!registerWidget(text)) return nullptr;
    return text;
}

// ---------------------------------------------------------------------------
// Status widget factory
// ---------------------------------------------------------------------------

dash::DashStatus* ESP32Dashboard::addStatus(const char* title) {
    auto* status = new (std::nothrow) dash::DashStatus(title);
    if (!status) { DASH_LOG_ERROR("addStatus: allocation failed"); return nullptr; }
    if (!registerWidget(status)) return nullptr;
    return status;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

void ESP32Dashboard::setTitle(const char* title) {
    if (title) {
        strncpy(_title, title, sizeof(_title));
        _title[sizeof(_title) - 1] = '\0';
        if (_started) {
            broadcastConfig();
        }
    }
}

dash::DashboardSysInfo ESP32Dashboard::getSysInfo() const {
    dash::DashboardSysInfo sys;
    sys.title = _title;
    sys.ip = _ipStr;
    sys.hostname = _hostname;
    sys.httpPort = _httpPort;
    sys.wsPort = _wsPort;
    sys.clients = _wsServer.getClientCount();
    sys.rssi = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
    sys.ssid = WiFi.status() == WL_CONNECTED ? WiFi.SSID().c_str() : "";
    sys.model = ESP.getChipModel();
    sys.heap = ESP.getFreeHeap();
    sys.uptime = millis() / 1000;
    return sys;
}

uint8_t ESP32Dashboard::getClientCount() const {
    return _wsServer.getClientCount();
}

const char* ESP32Dashboard::getIPAddress() const {
    return _ipStr;
}

// ---------------------------------------------------------------------------
// Internal: change detection and broadcasting
// ---------------------------------------------------------------------------

uint8_t ESP32Dashboard::checkAllChanges() {
    uint8_t count = 0;
    for (uint8_t i = 0; i < _widgetCount; ++i) {
        if (_widgets[i] && _widgets[i]->checkForChange()) {
            count++;
        }
    }
    return count;
}

void ESP32Dashboard::clearAllDirty() {
    for (uint8_t i = 0; i < _widgetCount; ++i) {
        if (_widgets[i]) {
            _widgets[i]->clearDirty();
        }
    }
}

void ESP32Dashboard::sendConfigToClient(uint8_t clientId) {
    if (!_jsonBuf) return;

    int len = _jsonEngine.serializeConfig(
        getSysInfo(), _widgets, _widgetCount, _jsonBuf, _jsonBufSize);

    if (len > 0) {
        _wsServer.sendToClient(clientId, _jsonBuf, len);
        DASH_LOG("Sent config to client #%u (%d bytes)", clientId, len);
    } else {
        DASH_LOG_ERROR("Failed to serialize config for client #%u", clientId);
    }
}

void ESP32Dashboard::broadcastDelta() {
    if (!_jsonBuf) return;

    int len = _jsonEngine.serializeDelta(
        getSysInfo(), _widgets, _widgetCount, _jsonBuf, _jsonBufSize);

    if (len > 0) {
        _wsServer.broadcast(_jsonBuf, len);
    }
}

void ESP32Dashboard::broadcastFullUpdate() {
    if (!_jsonBuf) return;

    int len = _jsonEngine.serializeFullUpdate(
        getSysInfo(), _widgets, _widgetCount, _jsonBuf, _jsonBufSize);

    if (len > 0) {
        _wsServer.broadcast(_jsonBuf, len);
    }
}

void ESP32Dashboard::broadcastConfig() {
    if (!_jsonBuf) return;

    int len = _jsonEngine.serializeConfig(
        getSysInfo(), _widgets, _widgetCount, _jsonBuf, _jsonBufSize);

    if (len > 0) {
        _wsServer.broadcast(_jsonBuf, len);
    }
}

// ---------------------------------------------------------------------------
// Static callback
// ---------------------------------------------------------------------------

void ESP32Dashboard::onWsConnect(uint8_t clientId) {
    if (_instance) {
        _instance->sendConfigToClient(clientId);
    }
}
