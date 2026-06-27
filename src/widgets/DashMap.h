/**
 * @file DashMap.h
 * @brief Map widget — displays a marker on a geographic map.
 *
 * Renders a Leaflet.js-based map (requires internet for tile loading)
 * with a marker at the specified latitude/longitude.
 *
 * Usage:
 * @code
 *   double lat = 37.7749, lon = -122.4194;
 *   dashboard.addMap(&lat, &lon);
 * @endcode
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
     * @param lat   Pointer to latitude (double).
     * @param lon   Pointer to longitude (double).
     */
    DashMap(const char* title, double* lat, double* lon);

    /**
     * @brief Manually set coordinates.
     * @param lat Latitude in degrees.
     * @param lon Longitude in degrees.
     */
    void setCoordinates(double lat, double lon);

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;

private:
    double* _latPtr;
    double* _lonPtr;
    double  _lat;
    double  _lon;
    double  _lastLat;
    double  _lastLon;
    bool    _usePointers;

    /** @brief Threshold for coordinate change detection (roughly 11m). */
    static constexpr double kCoordEpsilon = 0.0001;
};

} // namespace dash

#endif // DASH_MAP_H
