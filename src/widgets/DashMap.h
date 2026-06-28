/**
 * @file DashMap.h
 * @brief Map widget — displays a marker on a geographic map.
 *
 * Renders a Leaflet.js-based map with a marker at the specified
 * latitude/longitude. Supports configurable themes, marker styles,
 * heading rotation, movement history trails, zoom controls,
 * fullscreen, scale, and layer switching.
 *
 * Configuration changes (theme, zoom, marker style, etc.) are
 * transmitted to the browser only as part of the initial config
 * message. Runtime delta packets contain only coordinates and
 * heading — keeping WebSocket traffic minimal.
 *
 * Usage:
 * @code
 *   double lat = 37.7749, lon = -122.4194;
 *   float  hdg = 0.0f;
 *
 *   auto* map = dashboard.addMap(&lat, &lon);
 *
 *   // Optional configuration (call before dashboard.begin() or once
 *   // before the first client connects — triggers a config resync)
 *   map->setTheme(MapTheme::Satellite);
 *   map->setMarker(MarkerStyle::Car);
 *   map->setZoom(14);
 *   map->setFollow(true);
 *   map->setZoomControls(true);
 *   map->enableTrail(true);
 *   map->setTrailLength(100);
 *   map->setFullscreen(true);
 *   map->setScale(true);
 *   map->enableLayerSwitcher(true);
 *   map->setHeadingOffset(90.0f);  // icon faces right → offset 90°
 *   map->setMarkerScale(1.5f);
 *
 *   // Heading — rotation performed entirely in the browser
 *   map->bindHeading(&hdg);
 * @endcode
 *
 * @note Custom SVG strings are NOT transmitted over WebSocket for
 *       security and correctness reasons.  Use the built-in
 *       MarkerStyle enum instead.  setMarkerSVG() has been removed.
 */

#ifndef DASH_MAP_H
#define DASH_MAP_H

#include "DashWidget.h"

namespace dash {

class DashMap : public DashWidget {
public:
    /**
     * @brief Construct a map widget bound to lat/lon pointers.
     * @param title Title for the map widget.
     * @param lat   Pointer to latitude  (double). May be nullptr.
     * @param lon   Pointer to longitude (double). May be nullptr.
     */
    DashMap(const char* title, double* lat, double* lon);

    // =================================================================
    // Coordinate API
    // =================================================================

    /**
     * @brief Manually set coordinates (manual mode).
     *
     * Switches to manual mode — pointer updates are suspended until
     * bindCoordinates() is called again.
     *
     * @param lat Latitude  in decimal degrees.
     * @param lon Longitude in decimal degrees.
     */
    void setCoordinates(double lat, double lon);

    /**
     * @brief Bind (or re-bind) coordinate pointers.
     *
     * Re-enables automatic reading from external variables.
     *
     * @param lat Pointer to latitude.
     * @param lon Pointer to longitude.
     */
    void bindCoordinates(double* lat, double* lon);

    // =================================================================
    // Theme Configuration
    // =================================================================

    /**
     * @brief Set the map tile theme.
     *
     * Triggers a configuration resync to connected browsers.
     * The browser swaps the tile layer without recreating the map.
     *
     * @param theme One of the built-in MapTheme values.
     */
    void setTheme(MapTheme theme);

    // =================================================================
    // Zoom Configuration
    // =================================================================

    /**
     * @brief Set the initial zoom level.
     *
     * Clamped to the valid Leaflet range [1, 19]. Default: 15.
     *
     * @param zoom Zoom level.
     */
    void setZoom(uint8_t zoom);

    /**
     * @brief Enable or disable zoom +/- buttons in the UI.
     * @param enabled Default: false.
     */
    void setZoomControls(bool enabled);

    // =================================================================
    // Follow Mode
    // =================================================================

    /**
     * @brief Enable or disable auto-centering on coordinate updates.
     *
     * When true  — map pans to the marker on every update.
     * When false — marker moves; user can pan freely.
     *
     * @param enabled Default: true.
     */
    void setFollow(bool enabled);

    // =================================================================
    // Marker Configuration
    // =================================================================

    /**
     * @brief Set the marker style from the built-in set.
     *
     * All SVG assets are stored in the browser; only the enum integer
     * is transmitted in the config message.
     *
     * @param style One of the MarkerStyle enum values. Default: Circle.
     */
    void setMarker(MarkerStyle style);

    /**
     * @brief Set marker rotation offset.
     *
     * Use when an SVG icon's natural forward direction is not "up".
     * The browser renders: rotation = heading + headingOffset.
     *
     * @param degrees Offset in degrees. Default: 0.
     */
    void setHeadingOffset(float degrees);

    /**
     * @brief Set marker scale factor.
     *
     * Scales the SVG marker in the browser. Has no effect on the
     * Circle marker style.
     *
     * @param scale Multiplier (e.g. 0.75, 1.0, 1.5, 2.0). Default: 1.0.
     *              Clamped to [0.25, 4.0].
     */
    void setMarkerScale(float scale);

    // =================================================================
    // Heading / Rotation
    // =================================================================

    /**
     * @brief Bind a heading pointer for automatic marker rotation.
     *
     * Rotation is performed entirely in the browser via CSS transform.
     * The heading is included in delta packets as "hdg" only while
     * a pointer is bound or setHeading() has been called.
     * Ignored for the Circle marker style.
     *
     * @param heading Pointer to a float in degrees (0–360).
     */
    void bindHeading(float* heading);

    /**
     * @brief Manually set the heading angle.
     *
     * Switches to manual heading mode. Pointer updates are suspended.
     *
     * @param heading Heading in degrees (0–360).
     */
    void setHeading(float heading);

    // =================================================================
    // Trail / History
    // =================================================================

    /**
     * @brief Enable or disable the movement history trail.
     *
     * The browser stores the previous N coordinates and draws a
     * Leaflet polyline. Oldest points are automatically discarded.
     *
     * @param enabled Default: false.
     */
    void enableTrail(bool enabled);

    /**
     * @brief Set the maximum number of trail points retained.
     *
     * @param length Number of coordinate pairs. Default: 50.
     *               Clamped to [2, 500].
     */
    void setTrailLength(uint16_t length);

    // =================================================================
    // Controls
    // =================================================================

    /**
     * @brief Enable or disable the fullscreen button.
     *
     * Uses the browser Fullscreen API (requestFullscreen / exitFullscreen).
     *
     * @param enabled Default: false.
     */
    void setFullscreen(bool enabled);

    /**
     * @brief Enable or disable the Leaflet scale control.
     * @param enabled Default: false.
     */
    void setScale(bool enabled);

    /**
     * @brief Enable or disable the layer-switcher control.
     *
     * Allows the user to switch between all five built-in themes
     * in the browser without reconnecting. The map is not recreated.
     *
     * @param enabled Default: false.
     */
    void enableLayerSwitcher(bool enabled);

    // =================================================================
    // DashWidget Interface
    // =================================================================

    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;

    /**
     * @brief Serialize full configuration (sent on client connect or
     *        after a configuration change).
     *
     * Encodes all map settings (theme, zoom, marker, trail, controls)
     * as JSON fields alongside the current coordinates.
     */
    int serializeFull(char* buf, size_t size) const;

    // =================================================================
    // Internal dirty tracking
    // =================================================================

    /**
     * @brief Returns true if configuration (not runtime values) changed
     *        since the last full synchronization.
     */
    bool isConfigDirty() const override { return _configDirty; }

    /** @brief Clear the configuration-dirty flag. */
    void clearConfigDirty() override { _configDirty = false; }

private:
    // ---- Coordinate state ----
    double* _latPtr;
    double* _lonPtr;
    double  _lat;
    double  _lon;
    double  _lastLat;   ///< NAN on first run → forces initial broadcast
    double  _lastLon;
    bool    _usePointers;

    // ---- Heading state ----
    float*  _headingPtr;
    float   _heading;
    float   _lastHeading; ///< NAN on first run
    bool    _hasHeading;  ///< True when heading tracking is active

    // ---- Configuration ----
    MapTheme    _theme;
    MarkerStyle _markerStyle;
    uint8_t     _zoom;
    bool        _zoomControls;
    bool        _follow;
    bool        _trail;
    uint16_t    _trailLength;
    bool        _fullscreen;
    bool        _scale;
    bool        _layerSwitcher;
    float       _headingOffset;  ///< Browser-side rotation offset (degrees)
    float       _markerScale;    ///< Browser-side SVG scale multiplier

    // ---- Dirty flags ----
    bool _configDirty; ///< True when any config field changed

    // ---- Change-detection thresholds ----

    /** Roughly 11 m at the equator. */
    static constexpr double kCoordEpsilon   = 0.0001;

    /** Minimum heading change that triggers a delta (degrees). */
    static constexpr float  kHeadingEpsilon = 1.0f;

    // ---- Helpers ----

    /** Mark a configuration field as changed (triggers config resync). */
    void markConfigDirty();

    /**
     * @brief Compute the shortest signed angle between two headings.
     *
     * Returns a value in (-180, 180].
     */
    static float headingDiff(float a, float b);

    /** @brief Return the string name of a MapTheme for JSON. */
    static const char* themeName(MapTheme t);

    /** @brief Return the string name of a MarkerStyle for JSON. */
    static const char* markerName(MarkerStyle s);
};

} // namespace dash

#endif // DASH_MAP_H
