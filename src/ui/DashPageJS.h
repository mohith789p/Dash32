/**
 * @file DashPageJS.h
 * @brief Embedded JavaScript application stored in PROGMEM (flash).
 *
 * Client-side SPA that:
 * - Connects to WebSocket server
 * - Receives config and delta/update messages
 * - Dynamically creates widget DOM elements
 * - Updates values with animations
 * - Manages Leaflet maps
 * - Tracks update rate
 * - Auto-reconnects on disconnect
 */

#ifndef DASH_PAGE_JS_H
#define DASH_PAGE_JS_H

#include <pgmspace.h>

static const char DASH_JS[] PROGMEM = R"rawliteral(
(function() {
    'use strict';

    // ---- State ----
    let ws = null;
    let widgets = {};
    let maps = {};
    let markers = {};
    let updateCount = 0;
    let lastRateUpdate = Date.now();
    let reconnectDelay = 1000;
    const MAX_RECONNECT_DELAY = 15000;

    // ---- DOM references ----
    const grid = document.getElementById('widget-grid');
    const connBadge = document.getElementById('connection-status');
    const connText = connBadge.querySelector('.status-text');
    const timeEl = document.getElementById('client-time');
    const rateEl = document.getElementById('update-rate');

    // ---- Utility ----
    function escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }

    function formatValue(val) {
        if (typeof val === 'number') {
            return Number.isInteger(val) ? val.toString() : val.toFixed(2);
        }
        if (typeof val === 'boolean') return val ? 'ON' : 'OFF';
        if (val === null || val === undefined) return '--';
        return String(val);
    }

    // ---- Clock ----
    function updateClock() {
        const now = new Date();
        timeEl.textContent = now.toLocaleTimeString('en-US', { hour12: false });
    }
    setInterval(updateClock, 1000);
    updateClock();

    // ---- Update rate ----
    setInterval(function() {
        const elapsed = (Date.now() - lastRateUpdate) / 1000;
        const rate = elapsed > 0 ? (updateCount / elapsed).toFixed(1) : 0;
        rateEl.textContent = rate + ' updates/s';
        updateCount = 0;
        lastRateUpdate = Date.now();
    }, 2000);

    // ---- Widget renderers ----
    const renderers = {
        card: function(w) {
            const el = document.createElement('div');
            el.className = 'widget widget-card';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';
            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="widget-content">' +
                    '<span class="widget-value" id="val-' + w.id + '">' + formatValue(w.value) + '</span>' +
                    '<span class="widget-unit">' + escapeHtml(w.unit || '') + '</span>' +
                '</div>';
            return el;
        },

        gauge: function(w) {
            var min = w.min !== undefined ? w.min : 0;
            var max = w.max !== undefined ? w.max : 100;
            var val = w.value !== undefined ? w.value : 0;
            var pct = Math.max(0, Math.min(1, (val - min) / (max - min || 1)));
            var arcLen = 188.5; // π * 60 (radius)
            var offset = arcLen * (1 - pct);

            var el = document.createElement('div');
            el.className = 'widget widget-gauge';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';
            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="gauge-container">' +
                    '<svg class="gauge-svg" viewBox="0 0 160 100">' +
                        '<defs>' +
                            '<linearGradient id="gaugeGradient" x1="0%" y1="0%" x2="100%" y2="0%">' +
                                '<stop offset="0%" stop-color="#6366f1"/>' +
                                '<stop offset="100%" stop-color="#06b6d4"/>' +
                            '</linearGradient>' +
                        '</defs>' +
                        '<path class="gauge-bg" d="M 20 90 A 60 60 0 0 1 140 90"/>' +
                        '<path class="gauge-fill" id="gauge-arc-' + w.id + '"' +
                        ' d="M 20 90 A 60 60 0 0 1 140 90"' +
                        ' stroke-dasharray="' + arcLen + '"' +
                        ' stroke-dashoffset="' + offset + '"/>' +
                        '<text class="gauge-value-text" x="80" y="78" id="gauge-val-' + w.id + '">' +
                            formatValue(val) + '</text>' +
                        '<text class="gauge-unit-text" x="80" y="95">' +
                            escapeHtml(w.unit || '') + '</text>' +
                    '</svg>' +
                '</div>' +
                '<div class="gauge-range">' +
                    '<span>' + min + '</span>' +
                    '<span>' + max + '</span>' +
                '</div>';
            return el;
        },

        map: function(w) {
            var el = document.createElement('div');
            el.className = 'widget widget-map';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';
            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="map-container" id="map-' + w.id + '"></div>' +
                '<div class="map-coords" id="coords-' + w.id + '">' +
                    (w.lat || 0).toFixed(6) + ', ' + (w.lon || 0).toFixed(6) +
                '</div>';
            return el;
        },

        led: function(w) {
            var isOn = !!w.value;
            var color = isOn ? (w.colorOn || '#00E676') : (w.colorOff || '#616161');
            var el = document.createElement('div');
            el.className = 'widget widget-led';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';
            el.innerHTML =
                '<div class="led-indicator ' + (isOn ? 'on' : '') + '"' +
                ' id="led-' + w.id + '"' +
                ' style="background:' + color + ';color:' + color + '"></div>' +
                '<div class="led-info">' +
                    '<div class="widget-title" style="margin-bottom:0">' + escapeHtml(w.title) + '</div>' +
                    '<div class="led-state" id="led-state-' + w.id + '">' + (isOn ? 'ON' : 'OFF') + '</div>' +
                '</div>';
            return el;
        },

        text: function(w) {
            var el = document.createElement('div');
            el.className = 'widget widget-text';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';
            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="text-content" id="text-' + w.id + '">' +
                    escapeHtml(w.value || '') + '</div>';
            return el;
        },

        status: function(w) {
            var level = w.level || 'info';
            var el = document.createElement('div');
            el.className = 'widget widget-status';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';
            el.innerHTML =
                '<div class="status-indicator ' + level + '" id="status-dot-' + w.id + '"></div>' +
                '<div>' +
                    '<div class="widget-title" style="margin-bottom:2px">' + escapeHtml(w.title) + '</div>' +
                    '<div class="status-label" id="status-label-' + w.id + '">' +
                        escapeHtml(w.label || w.value || '') + '</div>' +
                '</div>';
            return el;
        }
    };

    // ---- Widget updaters ----
    var updaters = {
        card: function(w, data) {
            var el = document.getElementById('val-' + w.id);
            if (!el) return;
            var newVal = formatValue(data.value);
            if (el.textContent !== newVal) {
                el.textContent = newVal;
                el.classList.add('value-changed');
                setTimeout(function() { el.classList.remove('value-changed'); }, 600);
            }
        },

        gauge: function(w, data) {
            if (data.value === undefined) return;
            var min = w.min || 0;
            var max = w.max || 100;
            var val = data.value;
            var pct = Math.max(0, Math.min(1, (val - min) / (max - min || 1)));
            var arcLen = 188.5;
            var offset = arcLen * (1 - pct);

            var arc = document.getElementById('gauge-arc-' + w.id);
            var text = document.getElementById('gauge-val-' + w.id);
            if (arc) arc.setAttribute('stroke-dashoffset', offset);
            if (text) text.textContent = formatValue(val);
        },

        map: function(w, data) {
            var lat = data.lat !== undefined ? data.lat : w.lat;
            var lon = data.lon !== undefined ? data.lon : w.lon;
            w.lat = lat;
            w.lon = lon;

            var coordEl = document.getElementById('coords-' + w.id);
            if (coordEl) coordEl.textContent = lat.toFixed(6) + ', ' + lon.toFixed(6);

            if (maps[w.id]) {
                var latlng = L.latLng(lat, lon);
                markers[w.id].setLatLng(latlng);
                maps[w.id].panTo(latlng);
            }
        },

        led: function(w, data) {
            var isOn = !!data.value;
            var color = isOn ? (data.colorOn || w.colorOn || '#00E676')
                             : (data.colorOff || w.colorOff || '#616161');
            var indicator = document.getElementById('led-' + w.id);
            var stateEl = document.getElementById('led-state-' + w.id);
            if (indicator) {
                indicator.style.background = color;
                indicator.style.color = color;
                indicator.className = 'led-indicator ' + (isOn ? 'on' : '');
            }
            if (stateEl) stateEl.textContent = isOn ? 'ON' : 'OFF';
        },

        text: function(w, data) {
            var el = document.getElementById('text-' + w.id);
            if (el && data.value !== undefined) el.textContent = data.value;
        },

        status: function(w, data) {
            var dot = document.getElementById('status-dot-' + w.id);
            var label = document.getElementById('status-label-' + w.id);
            if (dot && data.level) dot.className = 'status-indicator ' + data.level;
            if (label && data.label) label.textContent = data.label;
        }
    };

    // ---- Initialize map after DOM insertion ----
    function initMap(w) {
        setTimeout(function() {
            var container = document.getElementById('map-' + w.id);
            if (!container || maps[w.id]) return;
            try {
                var lat = w.lat || 0;
                var lon = w.lon || 0;
                var map = L.map(container, {
                    center: [lat, lon],
                    zoom: 15,
                    zoomControl: false,
                    attributionControl: false
                });
                L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
                    maxZoom: 19
                }).addTo(map);
                var marker = L.circleMarker([lat, lon], {
                    radius: 8,
                    color: '#6366f1',
                    fillColor: '#06b6d4',
                    fillOpacity: 0.8,
                    weight: 2
                }).addTo(map);
                maps[w.id] = map;
                markers[w.id] = marker;
            } catch (e) {
                console.error('Map init error:', e);
            }
        }, 300);
    }

    // ---- Process config message ----
    function handleConfig(msg) {
        grid.innerHTML = '';
        widgets = {};
        maps = {};
        markers = {};

        if (!msg.widgets || !Array.isArray(msg.widgets)) return;

        msg.widgets.forEach(function(w) {
            widgets[w.id] = w;
            var renderer = renderers[w.type];
            if (renderer) {
                var el = renderer(w);
                grid.appendChild(el);
                if (w.type === 'map') initMap(w);
            }
        });
    }

    // ---- Process delta/update message ----
    function handleUpdate(msg) {
        if (!msg.widgets || !Array.isArray(msg.widgets)) return;
        updateCount += msg.widgets.length;

        msg.widgets.forEach(function(data) {
            var w = widgets[data.id];
            if (!w) return;

            // Merge data into widget state
            Object.assign(w, data);

            var updater = updaters[w.type];
            if (updater) updater(w, data);
        });
    }

    // ---- WebSocket ----
    function connect() {
        var host = window.location.hostname;
        var wsUrl = 'ws://' + host + ':81';

        try {
            ws = new WebSocket(wsUrl);
        } catch (e) {
            console.error('WS create error:', e);
            scheduleReconnect();
            return;
        }

        ws.onopen = function() {
            reconnectDelay = 1000;
            connBadge.className = 'status-badge connected';
            connText.textContent = 'Connected';
        };

        ws.onclose = function() {
            connBadge.className = 'status-badge disconnected';
            connText.textContent = 'Disconnected';
            scheduleReconnect();
        };

        ws.onerror = function() {
            connBadge.className = 'status-badge disconnected';
            connText.textContent = 'Error';
        };

        ws.onmessage = function(event) {
            try {
                var msg = JSON.parse(event.data);
                switch (msg.type) {
                    case 'config':
                        handleConfig(msg);
                        break;
                    case 'delta':
                    case 'update':
                        handleUpdate(msg);
                        break;
                }
            } catch (e) {
                console.error('JSON parse error:', e);
            }
        };
    }

    function scheduleReconnect() {
        setTimeout(function() {
            reconnectDelay = Math.min(reconnectDelay * 1.5, MAX_RECONNECT_DELAY);
            connect();
        }, reconnectDelay);
    }

    // ---- Start ----
    connect();

})();
)rawliteral";

static const size_t DASH_JS_SIZE = sizeof(DASH_JS) - 1;

#endif // DASH_PAGE_JS_H
