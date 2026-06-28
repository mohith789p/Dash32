/**
 * @file DashPageCSS.h
 * @brief Embedded CSS stylesheet stored in PROGMEM (flash).
 *
 * Professional dark theme with:
 * - CSS custom properties for theming
 * - Responsive grid layout
 * - Glassmorphism cards
 * - Smooth micro-animations
 * - Gauge arc rendering via conic-gradient
 */

#ifndef DASH_PAGE_CSS_H
#define DASH_PAGE_CSS_H

#include <pgmspace.h>

static const char DASH_CSS[] PROGMEM = R"rawliteral(
/* ============================================================
   CSS Custom Properties (Design Tokens)
   ============================================================ */
:root {
    --bg-primary: #0a0e1a;
    --bg-secondary: #111827;
    --bg-card: rgba(17, 24, 39, 0.75);
    --bg-card-hover: rgba(30, 41, 59, 0.85);
    --border-card: rgba(99, 102, 241, 0.15);
    --border-card-hover: rgba(99, 102, 241, 0.35);

    --text-primary: #f1f5f9;
    --text-secondary: #94a3b8;
    --text-muted: #64748b;

    --accent-primary: #6366f1;
    --accent-secondary: #06b6d4;
    --accent-gradient: linear-gradient(135deg, #6366f1, #06b6d4);

    --status-ok: #22c55e;
    --status-warning: #f59e0b;
    --status-error: #ef4444;
    --status-info: #3b82f6;

    --radius-sm: 8px;
    --radius-md: 12px;
    --radius-lg: 16px;
    --radius-xl: 20px;

    --shadow-card: 0 4px 24px rgba(0, 0, 0, 0.3), 0 0 0 1px var(--border-card);
    --shadow-card-hover: 0 8px 32px rgba(99, 102, 241, 0.12), 0 0 0 1px var(--border-card-hover);

    --font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    --transition-fast: 150ms ease;
    --transition-base: 250ms ease;
    --transition-slow: 400ms ease;
}

/* ============================================================
   Reset & Base
   ============================================================ */
*, *::before, *::after {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

html {
    font-size: 16px;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
}

body {
    font-family: var(--font-family);
    background: var(--bg-primary);
    color: var(--text-primary);
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    overflow-x: hidden;
    background-image:
        radial-gradient(ellipse at 20% 50%, rgba(99, 102, 241, 0.08) 0%, transparent 50%),
        radial-gradient(ellipse at 80% 20%, rgba(6, 182, 212, 0.06) 0%, transparent 50%);
}

/* ============================================================
   Header
   ============================================================ */
#dash-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 16px 24px;
    background: rgba(10, 14, 26, 0.85);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border-bottom: 1px solid rgba(99, 102, 241, 0.1);
    position: sticky;
    top: 0;
    z-index: 100;
}

.header-left {
    display: flex;
    align-items: center;
    gap: 12px;
}

.logo-icon {
    display: flex;
    align-items: center;
    justify-content: center;
    animation: pulse-glow 3s ease-in-out infinite;
}

@keyframes pulse-glow {
    0%, 100% { filter: drop-shadow(0 0 4px rgba(99, 102, 241, 0.3)); }
    50% { filter: drop-shadow(0 0 12px rgba(99, 102, 241, 0.6)); }
}

#dash-header h1 {
    font-size: 1.2rem;
    font-weight: 600;
    background: var(--accent-gradient);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    letter-spacing: -0.02em;
}

.header-right {
    display: flex;
    align-items: center;
    gap: 16px;
}

.status-badge {
    display: flex;
    align-items: center;
    gap: 6px;
    padding: 4px 12px;
    border-radius: 20px;
    font-size: 0.75rem;
    font-weight: 500;
    transition: all var(--transition-base);
}

.status-badge.connected {
    background: rgba(34, 197, 94, 0.12);
    border: 1px solid rgba(34, 197, 94, 0.3);
    color: #22c55e;
}

.status-badge.disconnected {
    background: rgba(239, 68, 68, 0.12);
    border: 1px solid rgba(239, 68, 68, 0.3);
    color: #ef4444;
}

.status-dot {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: currentColor;
    animation: dot-blink 2s ease-in-out infinite;
}

@keyframes dot-blink {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.3; }
}

.header-time {
    font-size: 0.75rem;
    color: var(--text-muted);
    font-variant-numeric: tabular-nums;
}

/* ============================================================
   Widget Grid
   ============================================================ */
.widget-grid {
    flex: 1;
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
    gap: 20px;
    padding: 24px;
    align-content: start;
}

/* ============================================================
   Widget Card (shared styles)
   ============================================================ */
.widget {
    background: var(--bg-card);
    border: 1px solid var(--border-card);
    border-radius: var(--radius-lg);
    padding: 20px;
    backdrop-filter: blur(12px);
    -webkit-backdrop-filter: blur(12px);
    transition: all var(--transition-base);
    position: relative;
    overflow: hidden;
    animation: widget-enter 0.4s ease-out both;
}

.widget::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    height: 2px;
    background: var(--accent-gradient);
    opacity: 0;
    transition: opacity var(--transition-base);
}

.widget:hover {
    background: var(--bg-card-hover);
    border-color: var(--border-card-hover);
    box-shadow: var(--shadow-card-hover);
    transform: translateY(-2px);
}

.widget:hover::before {
    opacity: 1;
}

@keyframes widget-enter {
    from {
        opacity: 0;
        transform: translateY(16px) scale(0.97);
    }
    to {
        opacity: 1;
        transform: translateY(0) scale(1);
    }
}

.widget-title {
    font-size: 0.8rem;
    font-weight: 500;
    color: var(--text-secondary);
    text-transform: uppercase;
    letter-spacing: 0.06em;
    margin-bottom: 12px;
}

.widget-value {
    font-size: 2rem;
    font-weight: 700;
    color: var(--text-primary);
    letter-spacing: -0.02em;
    line-height: 1.1;
    font-variant-numeric: tabular-nums;
    transition: color var(--transition-fast);
}

.widget-value.value-changed {
    color: var(--accent-secondary);
}

.widget-unit {
    font-size: 0.85rem;
    font-weight: 400;
    color: var(--text-muted);
    margin-left: 4px;
}

/* ============================================================
   Card Widget
   ============================================================ */
.widget-card .widget-content {
    display: flex;
    align-items: baseline;
    gap: 4px;
}

/* ============================================================
   Gauge Widget
   ============================================================ */
.widget-gauge {
    text-align: center;
}

.gauge-container {
    position: relative;
    width: 160px;
    height: 100px;
    margin: 0 auto 12px;
}

.gauge-svg {
    width: 160px;
    height: 100px;
}

.gauge-bg {
    fill: none;
    stroke: rgba(99, 102, 241, 0.1);
    stroke-width: 10;
    stroke-linecap: round;
}

.gauge-fill {
    fill: none;
    stroke: url(#gaugeGradient);
    stroke-width: 10;
    stroke-linecap: round;
    transition: stroke-dashoffset 0.6s cubic-bezier(0.4, 0, 0.2, 1);
}

.gauge-value-text {
    font-size: 1.5rem;
    font-weight: 700;
    fill: var(--text-primary);
    text-anchor: middle;
    font-family: var(--font-family);
}

.gauge-unit-text {
    font-size: 0.7rem;
    fill: var(--text-muted);
    text-anchor: middle;
    font-family: var(--font-family);
}

.gauge-range {
    display: flex;
    justify-content: space-between;
    font-size: 0.65rem;
    color: var(--text-muted);
    padding: 0 8px;
}

/* ============================================================
   Map Widget
   ============================================================ */
.widget-map {
    grid-column: span 2;
    padding: 0;
    overflow: hidden;
}

.widget-map .widget-title {
    padding: 16px 20px 8px;
}

.map-wrap {
    position: relative;
    width: 100%;
}

.map-container {
    height: 280px;
    width: 100%;
    border-radius: 0 0 var(--radius-lg) var(--radius-lg);
}

.map-coords {
    position: absolute;
    bottom: 8px;
    left: 8px;
    background: rgba(10, 14, 26, 0.85);
    backdrop-filter: blur(8px);
    padding: 4px 10px;
    border-radius: var(--radius-sm);
    font-size: 0.7rem;
    color: var(--text-secondary);
    font-variant-numeric: tabular-nums;
    z-index: 500;
}

.map-controls {
    position: absolute;
    top: 8px;
    right: 8px;
    display: flex;
    flex-direction: column;
    gap: 4px;
    z-index: 500;
}

.map-btn {
    width: 30px;
    height: 30px;
    border: 1px solid rgba(99, 102, 241, 0.3);
    border-radius: var(--radius-sm);
    background: rgba(10, 14, 26, 0.85);
    backdrop-filter: blur(8px);
    color: var(--text-secondary);
    font-size: 1rem;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: all var(--transition-fast);
    line-height: 1;
}

.map-btn:hover {
    background: rgba(99, 102, 241, 0.2);
    border-color: var(--accent-primary);
    color: var(--text-primary);
}

.map-btn.active {
    background: rgba(99, 102, 241, 0.3);
    border-color: var(--accent-primary);
    color: var(--accent-secondary);
}

/* Marker icon reset */
.dash-marker-icon {
    background: none !important;
    border: none !important;
}

.dash-marker {
    transform-origin: center center;
    transition: transform 180ms cubic-bezier(.2,.8,.2,1);
}

/* Fullscreen map */
.map-wrap:fullscreen,
.map-wrap:-webkit-full-screen {
    background: var(--bg-primary);
}

.map-wrap:fullscreen .map-container,
.map-wrap:-webkit-full-screen .map-container {
    height: 100vh;
    border-radius: 0;
}

/* Leaflet theme overrides */
.leaflet-control-attribution {
    font-size: 0.6rem !important;
    background: rgba(10, 14, 26, 0.7) !important;
    color: var(--text-muted) !important;
}
.leaflet-control-attribution a {
    color: var(--text-secondary) !important;
}
.leaflet-control-layers {
    background: rgba(10, 14, 26, 0.9) !important;
    border: 1px solid rgba(99, 102, 241, 0.2) !important;
    border-radius: var(--radius-sm) !important;
    color: var(--text-secondary) !important;
}
.leaflet-control-layers label {
    color: var(--text-secondary) !important;
    font-size: 0.75rem;
}
.leaflet-control-scale-line {
    background: rgba(10, 14, 26, 0.7) !important;
    border-color: var(--text-muted) !important;
    color: var(--text-secondary) !important;
    font-size: 0.6rem !important;
}

/* ============================================================
   LED Widget
   ============================================================ */
.widget-led {
    display: flex;
    align-items: center;
    gap: 16px;
}

.led-indicator {
    width: 36px;
    height: 36px;
    border-radius: 50%;
    transition: all var(--transition-base);
    flex-shrink: 0;
}

.led-indicator.on {
    box-shadow: 0 0 16px 4px currentColor;
}

.led-info {
    flex: 1;
}

.led-state {
    font-size: 0.85rem;
    font-weight: 500;
    margin-top: 2px;
}

/* ============================================================
   Text Widget
   ============================================================ */
.widget-text .text-content {
    font-size: 0.9rem;
    color: var(--text-secondary);
    line-height: 1.6;
    white-space: pre-wrap;
    word-break: break-word;
    max-height: 200px;
    overflow-y: auto;
}

.widget-text .text-content::-webkit-scrollbar {
    width: 4px;
}

.widget-text .text-content::-webkit-scrollbar-thumb {
    background: var(--text-muted);
    border-radius: 2px;
}

/* ============================================================
   Status Widget
   ============================================================ */
.widget-status {
    display: flex;
    align-items: center;
    gap: 12px;
}

.status-indicator {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    flex-shrink: 0;
}

.status-indicator.ok      { background: var(--status-ok); box-shadow: 0 0 8px var(--status-ok); }
.status-indicator.warning  { background: var(--status-warning); box-shadow: 0 0 8px var(--status-warning); }
.status-indicator.error    { background: var(--status-error); box-shadow: 0 0 8px var(--status-error); }
.status-indicator.info     { background: var(--status-info); box-shadow: 0 0 8px var(--status-info); }

.status-label {
    font-size: 0.9rem;
    font-weight: 500;
}

/* ============================================================
   Footer
   ============================================================ */
#dash-footer {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 12px 24px;
    border-top: 1px solid rgba(99, 102, 241, 0.08);
    font-size: 0.7rem;
    color: var(--text-muted);
}

#dash-footer strong {
    background: var(--accent-gradient);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
}

.footer-stat {
    font-variant-numeric: tabular-nums;
}

/* ============================================================
   Responsive
   ============================================================ */
@media (max-width: 768px) {
    .widget-grid {
        grid-template-columns: 1fr;
        padding: 16px;
        gap: 14px;
    }

    .widget-map {
        grid-column: span 1;
    }

    .map-container {
        height: 200px;
    }

    #dash-header h1 {
        font-size: 1rem;
    }

    .header-time {
        display: none;
    }
}

@media (max-width: 480px) {
    .widget-value {
        font-size: 1.6rem;
    }

    .gauge-container {
        width: 130px;
        height: 80px;
    }

    .gauge-svg {
        width: 130px;
        height: 80px;
    }
}

/* ============================================================
   Scrollbar
   ============================================================ */
::-webkit-scrollbar {
    width: 6px;
}

::-webkit-scrollbar-track {
    background: transparent;
}

::-webkit-scrollbar-thumb {
    background: var(--text-muted);
    border-radius: 3px;
}

::-webkit-scrollbar-thumb:hover {
    background: var(--text-secondary);
}
)rawliteral";

static const size_t DASH_CSS_SIZE = sizeof(DASH_CSS) - 1;

#endif // DASH_PAGE_CSS_H
