/**
 * @file DashLeaflet.h
 * @brief Local Leaflet.js and CSS fallback stored in PROGMEM.
 *
 * Contains a minimal Leaflet redirect stub. For full offline support,
 * replace the contents of DASH_LEAFLET_JS and DASH_LEAFLET_CSS with
 * the minified Leaflet 1.9.4 source files.
 *
 * The HTML page attempts to load Leaflet from the CDN first. Only if
 * that fails (e.g., no internet) does the browser request these local
 * endpoints. This keeps typical RAM usage low while providing a
 * graceful degradation path.
 *
 * To embed the full Leaflet library:
 *   1. Download leaflet.min.js and leaflet.min.css from leafletjs.com
 *   2. Replace the stub strings below with the full file contents
 *   3. Flash size will increase by ~160KB
 */

#ifndef DASH_LEAFLET_H
#define DASH_LEAFLET_H

#include <pgmspace.h>

// ---------------------------------------------------------------------------
// Leaflet JS (stub — replace with full leaflet.min.js for offline support)
// ---------------------------------------------------------------------------

static const char DASH_LEAFLET_JS[] PROGMEM = R"rawliteral(
/* Leaflet.js local fallback stub.
 * Replace this file content with the minified Leaflet 1.9.4 JS
 * for full offline support. See: https://unpkg.com/leaflet@1.9.4/dist/leaflet.js
 */
console.warn('[Dash32] Leaflet CDN unavailable. Local stub loaded. For offline support, embed the full Leaflet library in DashLeaflet.h');
)rawliteral";

static const size_t DASH_LEAFLET_JS_SIZE = sizeof(DASH_LEAFLET_JS) - 1;

// ---------------------------------------------------------------------------
// Leaflet CSS (stub — replace with full leaflet.css for offline support)
// ---------------------------------------------------------------------------

static const char DASH_LEAFLET_CSS[] PROGMEM = R"rawliteral(
/* Leaflet.css local fallback stub.
 * Replace this file content with the minified Leaflet 1.9.4 CSS
 * for full offline support. See: https://unpkg.com/leaflet@1.9.4/dist/leaflet.css
 */
)rawliteral";

static const size_t DASH_LEAFLET_CSS_SIZE = sizeof(DASH_LEAFLET_CSS) - 1;

#endif // DASH_LEAFLET_H
