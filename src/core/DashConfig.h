/**
 * @file DashConfig.h
 * @brief Compile-time configuration constants for ESP32Dashboard.
 *
 * Users may override any constant by defining it before including ESP32Dashboard.h.
 * Example:
 *   #define DASH_MAX_WIDGETS 30
 *   #include <ESP32Dashboard.h>
 */

#ifndef DASH_CONFIG_H
#define DASH_CONFIG_H

// ---------------------------------------------------------------------------
// Widget limits
// ---------------------------------------------------------------------------

/** Maximum number of widgets the dashboard can hold. */
#ifndef DASH_MAX_WIDGETS
#define DASH_MAX_WIDGETS 20
#endif

// ---------------------------------------------------------------------------
// String buffer sizes
// ---------------------------------------------------------------------------

/** Maximum length of a widget title (including null terminator). */
#ifndef DASH_TITLE_MAX_LEN
#define DASH_TITLE_MAX_LEN 32
#endif

/** Maximum length of a widget unit string (including null terminator). */
#ifndef DASH_UNIT_MAX_LEN
#define DASH_UNIT_MAX_LEN 16
#endif

/** Maximum length of a DashText content buffer (including null terminator). */
#ifndef DASH_TEXT_MAX_LEN
#define DASH_TEXT_MAX_LEN 128
#endif

/** Maximum length of a DashStatus label (including null terminator). */
#ifndef DASH_STATUS_LABEL_MAX_LEN
#define DASH_STATUS_LABEL_MAX_LEN 48
#endif

/** Maximum length of a manual string value (including null terminator). */
#ifndef DASH_STRING_VALUE_MAX_LEN
#define DASH_STRING_VALUE_MAX_LEN 64
#endif

// ---------------------------------------------------------------------------
// JSON buffers
// ---------------------------------------------------------------------------

/** Size of the stack-allocated JSON serialization buffer. */
#ifndef DASH_JSON_BUFFER_SIZE
#define DASH_JSON_BUFFER_SIZE 2048
#endif

// ---------------------------------------------------------------------------
// Network
// ---------------------------------------------------------------------------

/** Default HTTP server port. */
#ifndef DASH_HTTP_PORT
#define DASH_HTTP_PORT 80
#endif

/** Default WebSocket server port. */
#ifndef DASH_WS_PORT
#define DASH_WS_PORT 81
#endif

/** Maximum concurrent WebSocket clients. */
#ifndef DASH_MAX_WS_CLIENTS
#define DASH_MAX_WS_CLIENTS 4
#endif

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------

/** Minimum interval (ms) between WebSocket broadcasts. */
#ifndef DASH_UPDATE_INTERVAL_MS
#define DASH_UPDATE_INTERVAL_MS 100
#endif

/** Interval (ms) for full state resync to all clients. */
#ifndef DASH_FULL_SYNC_INTERVAL_MS
#define DASH_FULL_SYNC_INTERVAL_MS 5000
#endif

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

/** Set to 1 to enable Serial debug output. */
#ifndef DASH_DEBUG
#define DASH_DEBUG 1
#endif

#endif // DASH_CONFIG_H
