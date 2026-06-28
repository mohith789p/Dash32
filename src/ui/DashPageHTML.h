/**
 * @file DashPageHTML.h
 * @brief Embedded HTML page stored in PROGMEM (flash).
 *
 * The dashboard is a single-page application. This HTML loads
 * the CSS and JS from separate endpoints to keep each file manageable.
 */

#ifndef DASH_PAGE_HTML_H
#define DASH_PAGE_HTML_H

#include <pgmspace.h>

static const char DASH_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="ESP32 Real-Time Dashboard">
    <title>ESP32 Dashboard</title>
    <link rel="stylesheet" href="/style.css">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css"
          onerror="var l=document.createElement('link');l.rel='stylesheet';l.href='/leaflet.css';document.head.appendChild(l);" />
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"
            onerror="var s=document.createElement('script');s.src='/leaflet.js';document.head.appendChild(s);"></script>
</head>
<body>
    <header id="dash-header">
        <div class="header-left">
            <div class="logo-icon">
                <svg viewBox="0 0 32 32" width="28" height="28">
                    <circle cx="16" cy="16" r="14" fill="none" stroke="url(#logoGrad)" stroke-width="2.5"/>
                    <path d="M10 16 L14 20 L22 12" fill="none" stroke="url(#logoGrad)" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"/>
                    <defs>
                        <linearGradient id="logoGrad" x1="0" y1="0" x2="32" y2="32">
                            <stop offset="0%" stop-color="#6366f1"/>
                            <stop offset="100%" stop-color="#06b6d4"/>
                        </linearGradient>
                    </defs>
                </svg>
            </div>
            <h1>ESP32 Dashboard</h1>
        </div>
        <div class="header-right">
            <span id="connection-status" class="status-badge disconnected">
                <span class="status-dot"></span>
                <span class="status-text">Connecting...</span>
            </span>
            <span id="client-time" class="header-time"></span>
        </div>
    </header>

    <main id="widget-grid" class="widget-grid">
        <!-- Widgets are dynamically injected here by app.js -->
    </main>

    <footer id="dash-footer">
        <span>Powered by <strong>ESP32Dashboard</strong></span>
        <span id="update-rate" class="footer-stat">0 updates/s</span>
    </footer>

    <script src="/app.js"></script>
</body>
</html>
)rawliteral";

static const size_t DASH_HTML_SIZE = sizeof(DASH_HTML) - 1;

#endif // DASH_PAGE_HTML_H
