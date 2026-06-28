/**
 * @file DashPageJS.h
 * @brief Embedded JavaScript application stored in PROGMEM (flash).
 *
 * Client-side SPA that:
 * - Connects to WebSocket server
 * - Receives config and delta/update messages
 * - Dynamically creates widget DOM elements
 * - Updates values with animations
 * - Manages Leaflet maps with themes, markers, trails, controls
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
    var ws = null;
    var widgets = {};
    var maps = {};
    var markers = {};
    var markerHdg = {};  // Per-widget cumulative display heading (unwrapped)
    var tileLayers = {};
    var trails = {};
    var trailData = {};
    var animState = {};
    var updateCount = 0;
    var lastRateUpdate = Date.now();
    var reconnectDelay = 1000;
    var MAX_RECONNECT_DELAY = 15000;

    // ---- Tile themes [url, attribution, maxZoom] ----
    var THEMES = {
        'darkmatter': [
            'https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png',
            '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> &copy; <a href="https://carto.com/">CARTO</a>',
            19
        ],
        'positron': [
            'https://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}{r}.png',
            '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> &copy; <a href="https://carto.com/">CARTO</a>',
            19
        ],
        'voyager': [
            'https://{s}.basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}{r}.png',
            '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> &copy; <a href="https://carto.com/">CARTO</a>',
            19
        ],
        'satellite': [
            'https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}',
            'Tiles &copy; Esri, Maxar, Earthstar, USDA, USGS, AeroGRID, IGN, GIS Community',
            18
        ],
        'terrain': [
            'https://{s}.tile.opentopomap.org/{z}/{x}/{y}.png',
            '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> &copy; <a href="https://opentopomap.org">OpenTopoMap</a>',
            17
        ]
    };
    var THEME_NAMES = {
        'darkmatter': 'Dark Matter',
        'positron': 'Positron',
        'voyager': 'Voyager',
        'satellite': 'Satellite',
        'terrain': 'Terrain'
    };

    // ---- SVG Markers ----
    var MARKER_SVGS = {
        'circle': null,
        'pin': '<svg viewBox="0 0 24 36" width="100%" height="100%"><path d="M12 0C5.4 0 0 5.4 0 12c0 9 12 24 12 24s12-15 12-24C24 5.4 18.6 0 12 0z" fill="#{fill}"/><circle cx="12" cy="12" r="5" fill="#fff"/></svg>',
        'car': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M6 22h20l2 4H4l2-4zm1-2l2-12h2l1 4h8l1-4h2l2 12H7zm9-14c-2 0-3-1-3-3s1-3 3-3 3 1 3 3-1 3-3 3z" fill="#{fill}"/></svg>',
        'truck': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M4 24h24l2 4H2l2-4zm0-2l2-14h4V4h12v4h4l2 14H4zm8-14v4h8V8h-8z" fill="#{fill}"/></svg>',
        'motorcycle': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M16 2c-3 0-5 2-5 5v4l-5 2v4l5-1v6l-3 2v2l3-1 5 2 5-2 3 1v-2l-3-2v-6l5 1v-4l-5-2V7c0-3-2-5-5-5z" fill="#{fill}"/></svg>',
        'bicycle': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M16 4a4 4 0 100 8 4 4 0 000-8zm-6 10c-1 0-2 1-2 2l1 8h2l1-6h8l1 6h2l1-8c0-1-1-2-2-2h-12z" fill="#{fill}"/></svg>',
        'drone': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M16 4l-4 8h-6l-2 4h6l-4 8h4l6-8h6l2-4h-6l4-8h-4z" fill="#{fill}"/></svg>',
        'boat': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M4 20l12-16 12 16H4zm12-8l-6 6h12l-6-6zM2 22h28v4H2v-4z" fill="#{fill}"/></svg>',
        'aircraft': '<svg viewBox="0 0 32 32" width="100%" height="100%"><path d="M16 2L6 12v10l4 2v-6h12v6l4-2V12L16 2zm-4 12a2 2 0 110-4 2 2 0 010 4zm8 0a2 2 0 110-4 2 2 0 010 4z" fill="#{fill}"/></svg>'
    };

    // ---- DOM references ----
    var grid = document.getElementById('widget-grid');
    var connBadge = document.getElementById('connection-status');
    var connText = connBadge.querySelector('.status-text');
    var timeEl = document.getElementById('client-time');
    var rateEl = document.getElementById('update-rate');
    var sysBtn = document.getElementById('sys-info-btn');
    var sysPanel = document.getElementById('sys-info-panel');

    // ---- System Info Toggler ----
    if (sysBtn && sysPanel) {
        sysBtn.addEventListener('click', function() {
            sysPanel.classList.toggle('collapsed');
            sysBtn.classList.toggle('active', !sysPanel.classList.contains('collapsed'));
        });
    }

    // ---- Formatting & Helpers ----
    function formatUptime(seconds) {
        if (!seconds) return '0s';
        var d = Math.floor(seconds / (3600*24));
        var h = Math.floor((seconds % (3600*24)) / 3600);
        var m = Math.floor((seconds % 3600) / 60);
        var s = Math.floor(seconds % 60);
        var parts = [];
        if (d > 0) parts.push(d + 'd');
        if (h > 0) parts.push(h + 'h');
        if (m > 0) parts.push(m + 'm');
        if (s > 0 || parts.length === 0) parts.push(s + 's');
        return parts.join(' ');
    }

    function formatHeap(bytes) {
        if (!bytes) return '0 B';
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(2) + ' MB';
        if (bytes >= 1024) return (bytes / 1024).toFixed(1) + ' KB';
        return bytes + ' B';
    }

    function updateSysInfo(sys) {
        if (!sys) return;
        if (sys.ip !== undefined) document.getElementById('sys-ip').textContent = sys.ip;
        if (sys.hostname !== undefined) document.getElementById('sys-hostname').textContent = sys.hostname ? sys.hostname + '.local' : '-';
        if (sys.http !== undefined) document.getElementById('sys-http').textContent = sys.http;
        if (sys.ws !== undefined) document.getElementById('sys-ws').textContent = sys.ws;
        if (sys.ssid !== undefined) document.getElementById('sys-ssid').textContent = sys.ssid || '-';
        if (sys.rssi !== undefined) document.getElementById('sys-rssi').textContent = sys.rssi ? sys.rssi + ' dBm' : '-';
        if (sys.model !== undefined) {
            document.getElementById('sys-model').textContent = sys.model;
            document.getElementById('footer-model').textContent = sys.model;
        }
        if (sys.heap !== undefined) document.getElementById('sys-heap').textContent = formatHeap(sys.heap);
        if (sys.clients !== undefined) document.getElementById('sys-clients').textContent = sys.clients;
        if (sys.uptime !== undefined) document.getElementById('sys-uptime').textContent = formatUptime(sys.uptime);
    }

    function updateLastSync() {
        var now = new Date();
        var timeStr = now.toTimeString().split(' ')[0];
        document.getElementById('last-sync').textContent = 'Last sync: ' + timeStr;
    }

    // ---- Utility ----
    function escapeHtml(str) {
        var d = document.createElement('div');
        d.textContent = str;
        return d.innerHTML;
    }

    function formatValue(val) {
        if (typeof val === 'number') return Number.isInteger(val) ? val.toString() : val.toFixed(2);
        if (typeof val === 'boolean') return val ? 'ON' : 'OFF';
        if (val === null || val === undefined) return '--';
        return String(val);
    }

    // ---- Clock ----
    function updateClock() {
        timeEl.textContent = new Date().toLocaleTimeString('en-US', { hour12: false });
    }
    setInterval(updateClock, 1000);
    updateClock();

    // ---- Update rate ----
    setInterval(function() {
        var elapsed = (Date.now() - lastRateUpdate) / 1000;
        rateEl.textContent = (elapsed > 0 ? (updateCount / elapsed).toFixed(1) : 0) + ' updates/s';
        updateCount = 0;
        lastRateUpdate = Date.now();
    }, 2000);

    // ---- Create Leaflet marker from config ----
    function createMarker(w, lat, lon) {
        var ms = w.marker || 'circle';
        var scale = w.mScale || 1.0;
        var offset = w.hdgOff || 0.0;
        var heading = (w.hdg || 0.0) + offset;

        if (ms === 'circle') {
            return L.circleMarker([lat, lon], {
                radius: 8 * scale, color: '#6366f1', fillColor: '#06b6d4',
                fillOpacity: 0.8, weight: 2
            });
        }

        var svgStr = (MARKER_SVGS[ms] || '').replace(/#{fill}/g, '#6366f1');
        if (!svgStr) {
            return L.circleMarker([lat, lon], {
                radius: 8 * scale, color: '#6366f1', fillColor: '#06b6d4',
                fillOpacity: 0.8, weight: 2
            });
        }

        var baseSize = 28;
        var scaledSize = baseSize * scale;
        var halfSize = scaledSize / 2;

        // Determine if this marker type supports rotation
        var noRotate = (ms === 'pin') || (w.rotate === false);
        var rotation = noRotate ? 0 : heading;

        // Initialize cumulative display heading for shortest-path tracking
        markerHdg[w.id] = noRotate ? 0 : heading;

        var icon = L.divIcon({
            html: '<div class="dash-marker" style="width:100%;height:100%;display:inline-block;transform:rotate(' +
                  rotation + 'deg);transform-origin:center center">' + svgStr + '</div>',
            className: 'dash-marker-icon',
            iconSize: [scaledSize, scaledSize],
            iconAnchor: [halfSize, halfSize]
        });
        return L.marker([lat, lon], { icon: icon });
    }

    // ---- Shortest-path heading interpolation ----
    // Computes the shortest angular delta and applies it to the
    // cumulative display heading so CSS transition never spins
    // more than 180 degrees.
    function updateMarkerHeading(w) {
        var ms = w.marker || 'circle';
        if (ms === 'circle' || ms === 'pin') return;
        if (w.rotate === false) return;

        var el = document.querySelector('#widget-' + w.id + ' .dash-marker');
        if (!el) return;

        var offset = w.hdgOff || 0.0;
        var targetHdg = (w.hdg || 0.0) + offset;

        // Get previous cumulative display heading
        var prev = markerHdg[w.id];
        if (prev === undefined) {
            prev = targetHdg;
            markerHdg[w.id] = prev;
        }

        // Shortest angular delta
        var delta = targetHdg - (prev % 360);
        if (delta > 180) delta -= 360;
        if (delta < -180) delta += 360;

        // Accumulate into unwrapped heading
        var displayHdg = prev + delta;
        markerHdg[w.id] = displayHdg;

        el.style.transform = 'rotate(' + displayHdg + 'deg)';
    }

    // ---- Create tile layer ----
    function createTileLayer(themeKey) {
        var t = THEMES[themeKey] || THEMES['darkmatter'];
        return L.tileLayer(t[0], { maxZoom: t[2], attribution: t[1] });
    }

    // ---- Change theme without recreating the map ----
    function changeMapTheme(id, newTheme) {
        var map = maps[id];
        if (!map) return;
        var oldLayer = tileLayers[id];
        if (oldLayer) map.removeLayer(oldLayer);

        var newLayer = createTileLayer(newTheme);
        newLayer.addTo(map);
        tileLayers[id] = newLayer;
    }

    // ---- Update/recreate marker when style/scale/offset changes ----
    function updateMarkerOnMap(w) {
        var map = maps[w.id];
        if (!map) return;
        var oldMarker = markers[w.id];
        if (oldMarker) map.removeLayer(oldMarker);

        var lat = w.lat || 0;
        var lon = w.lon || 0;
        var marker = createMarker(w, lat, lon);
        marker.addTo(map);
        markers[w.id] = marker;
    }

    // ---- Widget renderers ----
    var renderers = {
        card: function(w) {
            var el = document.createElement('div');
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
            var arcLen = 188.5;
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

            var controls = '';
            if (w.follow !== false) {
                controls += '<button class="map-btn map-follow-btn active" id="follow-' + w.id +
                    '" title="Follow mode">&#9673;</button>';
            } else {
                controls += '<button class="map-btn map-follow-btn" id="follow-' + w.id +
                    '" title="Follow mode">&#9673;</button>';
            }
            if (w.trail) {
                controls += '<button class="map-btn map-fit-btn" id="fit-' + w.id +
                    '" title="Fit Route">&#x269E;</button>';
            }
            if (w.fullscreen) {
                controls += '<button class="map-btn map-fs-btn" id="fs-' + w.id +
                    '" title="Fullscreen">&#x26F6;</button>';
            }

            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="map-wrap">' +
                    '<div class="map-container" id="map-' + w.id + '"></div>' +
                    '<div class="map-controls">' + controls + '</div>' +
                    '<div class="map-coords" id="coords-' + w.id + '">' +
                        (w.lat || 0).toFixed(6) + ', ' + (w.lon || 0).toFixed(6) +
                    '</div>' +
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
        },

        image: function(w) {
            var el = document.createElement('div');
            el.className = 'widget widget-image';
            if (w.border === false) el.classList.add('widget-no-border');
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';

            var controls = '';
            if (w.fullscreen) {
                controls += '<button class="media-btn image-fs-btn" id="fs-' + w.id + '" title="Fullscreen">' +
                    '<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
                        '<path d="M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3"></path>' +
                    '</svg></button>';
            }

            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="image-wrap" id="image-wrap-' + w.id + '">' +
                    '<div class="image-container">' +
                        '<img class="image-element image-fit-' + (w.fit || 'contain') + '" id="img-' + w.id + '" src="" alt="' + escapeHtml(w.title) + '"/>' +
                        '<div class="media-overlay active" id="overlay-' + w.id + '">' +
                            '<div class="spinner"></div>' +
                            '<div class="overlay-title">Loading Image</div>' +
                        '</div>' +
                    '</div>' +
                    '<div class="media-controls">' + controls + '</div>' +
                '</div>';

            var img = el.querySelector('#img-' + w.id);
            var overlay = el.querySelector('#overlay-' + w.id);

            img.onload = function() {
                overlay.classList.remove('active');
            };

            img.onerror = function() {
                overlay.innerHTML =
                    '<svg viewBox="0 0 24 24" width="32" height="32" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
                        '<rect x="3" y="3" width="18" height="18" rx="2" ry="2"></rect>' +
                        '<line x1="1" y1="1" x2="23" y2="23"></line>' +
                        '<path d="M21 21H3a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h18a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2z"></path>' +
                        '<circle cx="8.5" cy="8.5" r="1.5"></circle>' +
                        '<polyline points="21 15 16 10 5 21"></polyline>' +
                    '</svg>' +
                    '<div class="overlay-title">Failed to Load Image</div>' +
                    '<div class="overlay-desc">Verify URL and network connection</div>';
                overlay.classList.add('active');
            };

            var url = w.url || '';
            if (url) {
                if (w.refresh > 0) {
                    setInterval(function() {
                        var bustUrl = url + (url.indexOf('?') >= 0 ? '&' : '?') + '_cb=' + Date.now();
                        img.src = bustUrl;
                    }, w.refresh);
                }
                img.src = url;
            } else {
                overlay.innerHTML =
                    '<svg viewBox="0 0 24 24" width="32" height="32" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
                        '<circle cx="12" cy="12" r="10"></circle>' +
                        '<line x1="12" y1="8" x2="12" y2="12"></line>' +
                        '<line x1="12" y1="16" x2="12.01" y2="16"></line>' +
                    '</svg>' +
                    '<div class="overlay-title">No URL Configured</div>';
                overlay.classList.add('active');
            }

            setTimeout(function() {
                var wrap = el.querySelector('#image-wrap-' + w.id);
                var fsBtn = el.querySelector('#fs-' + w.id);
                if (fsBtn && wrap) {
                    fsBtn.addEventListener('click', function() {
                        if (!document.fullscreenElement) {
                            wrap.requestFullscreen().catch(function(err) {
                                console.error('Error enabling fullscreen:', err);
                            });
                        } else {
                            document.exitFullscreen();
                        }
                    });
                }
            }, 0);

            return el;
        },

        video: function(w) {
            var el = document.createElement('div');
            el.className = 'widget widget-video';
            el.id = 'widget-' + w.id;
            el.style.animationDelay = (w.id * 60) + 'ms';

            var controls = '';
            if (w.fullscreen) {
                controls += '<button class="media-btn video-fs-btn" id="fs-' + w.id + '" title="Fullscreen">' +
                    '<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
                        '<path d="M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3"></path>' +
                    '</svg></button>';
            }

            el.innerHTML =
                '<div class="widget-title">' + escapeHtml(w.title) + '</div>' +
                '<div class="video-wrap" id="video-wrap-' + w.id + '">' +
                    '<div class="video-container" id="video-container-' + w.id + '">' +
                        '<div class="media-overlay active" id="overlay-' + w.id + '">' +
                            '<div class="spinner"></div>' +
                            '<div class="overlay-title">Connecting to Stream</div>' +
                        '</div>' +
                    '</div>' +
                    '<div class="media-controls">' + controls + '</div>' +
                '</div>';

            var container = el.querySelector('#video-container-' + w.id);
            var overlay = el.querySelector('#overlay-' + w.id);
            var reconnectInterval = w.reconnect || 3000;
            var reconnectTimer = null;
            var mediaElement = null;

            function isMjpeg(url) {
                if (!url) return false;
                var low = url.toLowerCase();
                return low.includes('stream') || low.includes('mjpg') || low.includes('mjpeg') || low.includes(':81') || low.includes('/video');
            }

            function clearReconnect() {
                if (reconnectTimer) {
                    clearTimeout(reconnectTimer);
                    reconnectTimer = null;
                }
            }

            function handleDisconnect() {
                clearReconnect();
                overlay.innerHTML =
                    '<svg viewBox="0 0 24 24" width="32" height="32" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
                        '<path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"></path>' +
                        '<line x1="12" y1="9" x2="12" y2="13"></line>' +
                        '<line x1="12" y1="17" x2="12.01" y2="17"></line>' +
                    '</svg>' +
                    '<div class="overlay-title">Stream Disconnected</div>' +
                    '<div class="overlay-desc">Reconnecting in ' + (reconnectInterval/1000) + 's...</div>';
                overlay.classList.add('active');

                reconnectTimer = setTimeout(function() {
                    overlay.innerHTML =
                        '<div class="spinner"></div>' +
                        '<div class="overlay-title">Reconnecting...</div>';
                    loadStream();
                }, reconnectInterval);
            }

            function loadStream() {
                var url = w.url || '';
                if (!url) {
                    overlay.innerHTML =
                        '<svg viewBox="0 0 24 24" width="32" height="32" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
                            '<circle cx="12" cy="12" r="10"></circle>' +
                            '<line x1="12" y1="8" x2="12" y2="12"></line>' +
                            '<line x1="12" y1="16" x2="12.01" y2="16"></line>' +
                        '</svg>' +
                        '<div class="overlay-title">No stream URL configured</div>';
                    overlay.classList.add('active');
                    return;
                }

                if (mediaElement) {
                    try {
                        container.removeChild(mediaElement);
                    } catch(e){}
                }

                if (isMjpeg(url)) {
                    var img = document.createElement('img');
                    img.className = 'video-element';
                    img.alt = w.title;
                    mediaElement = img;

                    img.onload = function() {
                        overlay.classList.remove('active');
                        clearReconnect();
                    };

                    img.onerror = function() {
                        handleDisconnect();
                    };

                    img.src = url + (url.indexOf('?') >= 0 ? '&' : '?') + '_cb=' + Date.now();
                    container.insertBefore(img, overlay);
                } else {
                    var vid = document.createElement('video');
                    vid.className = 'video-element';
                    if (w.autoplay !== false) vid.autoplay = true;
                    if (w.muted !== false) vid.muted = true;
                    vid.playsInline = true;
                    if (w.controls) vid.controls = true;
                    mediaElement = vid;

                    vid.onplaying = function() {
                        overlay.classList.remove('active');
                        clearReconnect();
                    };

                    vid.onstalled = vid.onerror = vid.onwaiting = function() {
                        if (!reconnectTimer) {
                            reconnectTimer = setTimeout(handleDisconnect, 1500);
                        }
                    };

                    vid.src = url;
                    container.insertBefore(vid, overlay);
                }
            }

            loadStream();

            w._reload = function() {
                clearReconnect();
                loadStream();
            };

            setTimeout(function() {
                var wrap = el.querySelector('#video-wrap-' + w.id);
                var fsBtn = el.querySelector('#fs-' + w.id);
                if (fsBtn && wrap) {
                    fsBtn.addEventListener('click', function() {
                        if (!document.fullscreenElement) {
                            wrap.requestFullscreen().catch(function(err) {
                                console.error('Error enabling fullscreen:', err);
                            });
                        } else {
                            document.exitFullscreen();
                        }
                    });
                }
            }, 0);

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
            if (data.hdg !== undefined) w.hdg = data.hdg;

            var coordEl = document.getElementById('coords-' + w.id);
            if (coordEl) coordEl.textContent = lat.toFixed(6) + ', ' + lon.toFixed(6);

            if (!maps[w.id]) return;
            var latlng = L.latLng(lat, lon);

            // Smooth animation
            var st = animState[w.id];
            if (st && st.frame) cancelAnimationFrame(st.frame);
            var prev = markers[w.id].getLatLng();
            var startTime = performance.now();
            var dur = 500;
            animState[w.id] = {};
            (function animate() {
                var t = Math.min(1, (performance.now() - startTime) / dur);
                var ease = t < 0.5 ? 2*t*t : -1+(4-2*t)*t;
                var cLat = prev.lat + (lat - prev.lat) * ease;
                var cLon = prev.lng + (lon - prev.lng) * ease;
                var pos = L.latLng(cLat, cLon);
                markers[w.id].setLatLng(pos);
                if (w._follow && maps[w.id]) maps[w.id].panTo(pos, {animate:false});
                if (t < 1) {
                    animState[w.id].frame = requestAnimationFrame(animate);
                } else {
                    markers[w.id].setLatLng(latlng);
                    if (w._follow) maps[w.id].panTo(latlng);
                }
            })();

            // Heading
            if (data.hdg !== undefined) updateMarkerHeading(w);

            // Trail
            if (w.trail && trails[w.id]) {
                var td = trailData[w.id];
                td.push([lat, lon]);
                while (td.length > (w.trailLen || 50)) td.shift();
                trails[w.id].setLatLngs(td);
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
        },

        image: function(w, data) {
            if (data.url !== undefined && data.url !== w.url) {
                w.url = data.url;
                var img = document.getElementById('img-' + w.id);
                var overlay = document.getElementById('overlay-' + w.id);
                if (img) {
                    overlay.classList.add('active');
                    overlay.innerHTML = '<div class="spinner"></div><div class="overlay-title">Loading Image</div>';
                    img.src = w.url;
                }
            }
        },

        video: function(w, data) {
            if (data.url !== undefined && data.url !== w.url) {
                w.url = data.url;
                if (typeof w._reload === 'function') {
                    w._reload();
                }
            }
        }
    };

    // ---- Initialize map after DOM insertion ----
    function initMap(w) {
        requestAnimationFrame(function() {
            requestAnimationFrame(function() {
                var container = document.getElementById('map-' + w.id);
                if (!container || maps[w.id]) return;
                try {
                    var lat = w.lat || 0;
                    var lon = w.lon || 0;
                    var zoom = w.zoom || 15;
                    var themeKey = w.theme || 'darkmatter';
                    w._follow = w.follow !== false;

                    var map = L.map(container, {
                        center: [lat, lon],
                        zoom: zoom,
                        zoomControl: !!w.zoomCtrl,
                        attributionControl: true
                    });

                    // Base tile layer
                    var baseLayers = {};
                    var activeLayer = createTileLayer(themeKey);
                    activeLayer.addTo(map);
                    tileLayers[w.id] = activeLayer;

                    // Layer switcher
                    if (w.layers) {
                        for (var key in THEMES) {
                            if (THEMES.hasOwnProperty(key)) {
                                baseLayers[THEME_NAMES[key]] = createTileLayer(key);
                            }
                        }
                        baseLayers[THEME_NAMES[themeKey]] = activeLayer;
                        L.control.layers(baseLayers, null, {position:'topright'}).addTo(map);
                        
                        // Sync layers on manual user click
                        map.on('baselayerchange', function(e) {
                            for (var key in THEME_NAMES) {
                                if (THEME_NAMES.hasOwnProperty(key) && THEME_NAMES[key] === e.name) {
                                    w.theme = key;
                                    tileLayers[w.id] = e.layer;
                                    break;
                                }
                            }
                        });
                    }

                    // Scale
                    if (w.scale) L.control.scale({position:'bottomright'}).addTo(map);

                    // Marker
                    var marker = createMarker(w, lat, lon);
                    marker.addTo(map);
                    markers[w.id] = marker;
                    maps[w.id] = map;

                    // Trail
                    if (w.trail) {
                        trailData[w.id] = [[lat, lon]];
                        trails[w.id] = L.polyline([[lat, lon]], {
                            color: '#06b6d4', weight: 2, opacity: 0.7,
                            dashArray: '6,4'
                        }).addTo(map);
                    }

                    // Follow button
                    var followBtn = document.getElementById('follow-' + w.id);
                    if (followBtn) {
                        followBtn.addEventListener('click', function() {
                            w._follow = !w._follow;
                            this.classList.toggle('active', w._follow);
                            if (w._follow) map.panTo(markers[w.id].getLatLng());
                        });
                    }

                    // Fit Route button
                    var fitBtn = document.getElementById('fit-' + w.id);
                    if (fitBtn && w.trail) {
                        fitBtn.addEventListener('click', function() {
                            var trail = trails[w.id];
                            if (trail) {
                                var bounds = trail.getBounds();
                                if (bounds.isValid()) {
                                    map.fitBounds(bounds, {padding: [20, 20]});
                                }
                            }
                        });
                    }

                    // Fullscreen button
                    var fsBtn = document.getElementById('fs-' + w.id);
                    if (fsBtn) {
                        fsBtn.addEventListener('click', function() {
                            var wrap = container.parentElement;
                            if (!document.fullscreenElement &&
                                !document.webkitFullscreenElement &&
                                !document.msFullscreenElement) {
                                var req = wrap.requestFullscreen || wrap.webkitRequestFullscreen || wrap.msRequestFullscreen;
                                if (req) req.call(wrap);
                            } else {
                                var exit = document.exitFullscreen || document.webkitExitFullscreen || document.msExitFullscreen;
                                if (exit) exit.call(document);
                            }
                        });
                        var onFSChange = function() {
                            setTimeout(function() { map.invalidateSize(); }, 100);
                        };
                        document.addEventListener('fullscreenchange', onFSChange);
                        document.addEventListener('webkitfullscreenchange', onFSChange);
                        document.addEventListener('msfullscreenchange', onFSChange);
                    }

                    // Fix size on visibility
                    setTimeout(function() { map.invalidateSize(); }, 100);

                } catch (e) {
                    console.error('Map init error:', e);
                }
            });
        });
    }

    // ---- Process config message ----
    function handleConfig(msg) {
        if (msg.title) {
            document.getElementById('dash-title').textContent = msg.title;
            document.title = msg.title;
        }

        if (msg.sys) {
            updateSysInfo(msg.sys);
        }

        // If widgets already exist, check if we are just updating config for existing ones
        var isUpdate = Object.keys(widgets).length > 0;

        if (!msg.widgets || !Array.isArray(msg.widgets)) return;

        if (!isUpdate) {
            grid.innerHTML = '';
            widgets = {};
            maps = {};
            markers = {};
            markerHdg = {};
            tileLayers = {};
            trails = {};
            trailData = {};
            animState = {};
        }

        if (msg.widgets.length === 0) {
            grid.innerHTML = 
                '<div class="empty-state">' +
                    '<div class="empty-state-icon">' +
                        '<svg viewBox="0 0 24 24" width="48" height="48" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">' +
                            '<rect x="3" y="3" width="18" height="18" rx="2" ry="2"></rect>' +
                            '<line x1="9" y1="9" x2="15" y2="9"></line>' +
                            '<line x1="9" y1="13" x2="15" y2="13"></line>' +
                            '<line x1="9" y1="17" x2="13" y2="17"></line>' +
                        '</svg>' +
                    '</div>' +
                    '<h2>No Widgets Configured</h2>' +
                    '<p>Use the ESP32Dashboard API in your sketch to add widgets to the dashboard grid.</p>' +
                    '<code>dashboard.addCard("Temperature", &temp);</code>' +
                '</div>';
            return;
        }

        msg.widgets.forEach(function(w) {
            if (isUpdate && widgets[w.id]) {
                var old = widgets[w.id];
                
                // If it's a map widget, dynamically apply config changes
                if (w.type === 'map' && maps[w.id]) {
                    var map = maps[w.id];
                    
                    // Theme changed
                    if (w.theme && w.theme !== old.theme) {
                        changeMapTheme(w.id, w.theme);
                    }
                    
                    // Marker style or marker scale or heading offset or rotate changed
                    if (w.marker !== old.marker || w.mScale !== old.mScale ||
                        w.hdgOff !== old.hdgOff || w.rotate !== old.rotate) {
                        old.marker = w.marker;
                        old.mScale = w.mScale;
                        old.hdgOff = w.hdgOff;
                        old.rotate = w.rotate;
                        updateMarkerOnMap(old);
                    }

                    // Follow changed
                    if (w.follow !== undefined && w.follow !== old.follow) {
                        old.follow = w.follow;
                        old._follow = w.follow !== false;
                        var fBtn = document.getElementById('follow-' + w.id);
                        if (fBtn) fBtn.classList.toggle('active', old._follow);
                    }

                    // Zoom changed
                    if (w.zoom && w.zoom !== old.zoom) {
                        old.zoom = w.zoom;
                        map.setZoom(w.zoom);
                    }
                    
                    // Update current properties
                    Object.assign(old, w);
                } else {
                    Object.assign(old, w);
                    var updater = updaters[old.type];
                    if (updater) updater(old, w);
                }
            } else {
                widgets[w.id] = w;
                var renderer = renderers[w.type];
                if (renderer) {
                    var el = renderer(w);
                    grid.appendChild(el);
                    if (w.type === 'map') initMap(w);
                }
            }
        });
    }

    // ---- Process delta/update message ----
    function handleUpdate(msg) {
        if (msg.sys) {
            updateSysInfo(msg.sys);
        }
        if (!msg.widgets || !Array.isArray(msg.widgets)) return;
        updateCount += msg.widgets.length;

        msg.widgets.forEach(function(data) {
            var w = widgets[data.id];
            if (!w) return;
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
                updateLastSync();
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
