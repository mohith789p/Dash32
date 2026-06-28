/**
 * @file DashGauge.h
 * @brief Gauge widget — displays a value on a circular/arc gauge.
 *
 * Shows a title, value, unit, and a visual arc indicating the value
 * relative to a configurable min/max range.
 *
 * Usage:
 * @code
 *   float speed = 0.0f;
 *   DashGauge* g = dashboard.addGauge("Speed", &speed, "km/h");
 *   g->setRange(0, 200);
 * @endcode
 */

#ifndef DASH_GAUGE_H
#define DASH_GAUGE_H

#include "DashWidget.h"
#include "../core/ValueHolder.h"

namespace dash {

class DashGauge : public DashWidget {
public:
    /**
     * @brief Construct a gauge with title and optional unit.
     * @param title Display title.
     * @param unit  Unit string (may be nullptr).
     */
    explicit DashGauge(const char* title, const char* unit = nullptr);

    /**
     * @brief Set the gauge min/max range.
     * @param min Minimum value.
     * @param max Maximum value.
     */
    void setRange(float min, float max);

    // ----- Pointer binding -----
    void bindValue(float* ptr)  { _value.bind(ptr); markDirty(); }
    void bindValue(double* ptr) { _value.bind(ptr); markDirty(); }
    void bindValue(int* ptr)    { _value.bind(ptr); markDirty(); }
    void bindValue(long* ptr)   { _value.bind(ptr); markDirty(); }

    // ----- Manual setters -----
    void setValue(float val)  { _value.set(val); markDirty(); }
    void setValue(double val) { _value.set(val); markDirty(); }
    void setValue(int val)    { _value.set(val); markDirty(); }
    void setValue(long val)   { _value.set(val); markDirty(); }

    // ----- Accessors -----
    float getMin() const { return _rangeMin; }
    float getMax() const { return _rangeMax; }

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;
    int  serializeConfig(char* buf, size_t size) const override;

private:
    ValueHolder _value;
    float       _rangeMin;
    float       _rangeMax;
};

} // namespace dash

#endif // DASH_GAUGE_H
