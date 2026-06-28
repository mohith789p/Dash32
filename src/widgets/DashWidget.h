/**
 * @file DashWidget.h
 * @brief Abstract base class for all dashboard widgets.
 *
 * Every widget has:
 * - A unique title (used as identifier)
 * - An optional unit string
 * - A type tag for frontend rendering
 * - A dirty flag for delta-based serialization
 *
 * Subclasses implement serializeValue() and checkForChange().
 */

#ifndef DASH_WIDGET_H
#define DASH_WIDGET_H

#include "../core/DashConfig.h"
#include "../core/DashTypes.h"

namespace dash {

class DashWidget {
public:
    /**
     * @brief Construct a widget with a title, unit, and type.
     * @param title  Display title (max DASH_TITLE_MAX_LEN - 1 chars).
     * @param unit   Unit string (may be nullptr).
     * @param type   Widget type tag.
     */
    DashWidget(const char* title, const char* unit, WidgetType type);

    /** Virtual destructor for proper polymorphic cleanup. */
    virtual ~DashWidget() = default;

    // Non-copyable, non-movable (widgets are managed by pointer in a fixed array)
    DashWidget(const DashWidget&) = delete;
    DashWidget& operator=(const DashWidget&) = delete;

    // ----- Accessors -----

    /** @brief Get the widget title. */
    const char* getTitle() const { return _title; }

    /** @brief Get the unit string. */
    const char* getUnit() const { return _unit; }

    /** @brief Get the widget type. */
    WidgetType getType() const { return _type; }

    /** @brief Get the widget ID (index in the dashboard). */
    uint8_t getId() const { return _id; }

    /** @brief Set the widget ID (called by ESP32Dashboard on registration). */
    void setId(uint8_t id) { _id = id; }

    // ----- Dirty tracking -----

    /** @brief Returns true if the widget value has changed since last send. */
    bool isDirty() const { return _dirty; }

    /** @brief Mark the widget as clean (value has been sent). */
    void clearDirty() { _dirty = false; }

    /** @brief Force the widget to be marked dirty. */
    void markDirty() { _dirty = true; }

    /** @brief Returns true if the widget configuration has changed since last send. */
    virtual bool isConfigDirty() const { return false; }

    /** @brief Clear the configuration-dirty flag. */
    virtual void clearConfigDirty() {}

    // ----- Polymorphic interface -----

    /**
     * @brief Check if the underlying value has changed; update _dirty flag.
     *
     * Called by the dashboard before serialization. If the value has changed,
     * the widget marks itself dirty and takes a snapshot.
     *
     * @return true if the value changed.
     */
    virtual bool checkForChange() = 0;

    /**
     * @brief Serialize only the value portion as JSON into the buffer.
     *
     * Example output: `"value":23.50`
     *
     * @param buf   Output buffer.
     * @param size  Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    virtual int serializeValue(char* buf, size_t size) const = 0;

    /**
     * @brief Serialize the full widget configuration as a JSON object.
     *
     * Includes id, type, title, unit, and current value.
     * Example: `{"id":0,"type":"card","title":"Temp","unit":"°C","value":23.50}`
     *
     * @param buf   Output buffer.
     * @param size  Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeFull(char* buf, size_t size) const;

    /**
     * @brief Serialize a delta update as a JSON object.
     *
     * Includes only id and value.
     * Example: `{"id":0,"value":23.50}`
     *
     * @param buf   Output buffer.
     * @param size  Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeDelta(char* buf, size_t size) const;

protected:
    char       _title[DASH_TITLE_MAX_LEN];
    char       _unit[DASH_UNIT_MAX_LEN];
    WidgetType _type;
    uint8_t    _id;
    bool       _dirty;

    /** @brief Get the string name of the widget type for JSON. */
    const char* typeName() const;
};

} // namespace dash

#endif // DASH_WIDGET_H
