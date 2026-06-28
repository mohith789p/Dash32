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
            <h1 id="dash-title">Dash32 Monitor</h1>
        </div>
        <div class="header-right">
            <button id="sys-info-btn" class="sys-info-btn" title="Toggle System Info">
                <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
                    <circle cx="12" cy="12" r="10"></circle>
                    <line x1="12" y1="16" x2="12" y2="12"></line>
                    <line x1="12" y1="8" x2="12.01" y2="8"></line>
                </svg>
                <span>System Info</span>
            </button>
            <span id="connection-status" class="status-badge disconnected">
                <span class="status-dot"></span>
                <span class="status-text">Connecting...</span>
            </span>
            <span id="client-time" class="header-time"></span>
        </div>
    </header>

    <div id="sys-info-panel" class="sys-info-panel collapsed">
        <div class="sys-info-grid">
            <div class="sys-info-item"><span class="sys-info-label">IP Address</span><span id="sys-ip" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">Hostname</span><span id="sys-hostname" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">HTTP Port</span><span id="sys-http" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">WebSocket Port</span><span id="sys-ws" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">SSID</span><span id="sys-ssid" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">RSSI</span><span id="sys-rssi" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">Chip Model</span><span id="sys-model" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">Free Heap</span><span id="sys-heap" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">Clients</span><span id="sys-clients" class="sys-info-value">-</span></div>
            <div class="sys-info-item"><span class="sys-info-label">Uptime</span><span id="sys-uptime" class="sys-info-value">-</span></div>
        </div>
    </div>

    <main id="widget-grid" class="widget-grid">
        <!-- Widgets are dynamically injected here by app.js -->
    </main>

    <footer id="dash-footer">
        <div class="footer-left">
            Powered by <a href="https://github.com/mohith789p/Dash32" target="_blank" rel="noopener noreferrer">Dash32</a>
        </div>
        <div class="footer-center">
            <span id="footer-model">-</span> | <span id="footer-version">v1.0.0</span>
        </div>
        <div class="footer-right">
            <span id="update-rate" class="footer-stat">0.0 updates/s</span>
            <span class="footer-divider">|</span>
            <span id="last-sync" class="footer-stat">Last sync: -</span>
        </div>
    </footer>

    <script src="/app.js"></script>
</body>
</html>
)rawliteral";

static const size_t DASH_HTML_SIZE = sizeof(DASH_HTML) - 1;

#endif // DASH_PAGE_HTML_H
