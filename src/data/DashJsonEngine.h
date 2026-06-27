/**
 * @file DashJsonEngine.h
 * @brief JSON serialization engine for dashboard state.
 *
 * Provides three serialization modes:
 * - **Config**: Full widget array with metadata (sent on client connect).
 * - **Delta**: Only widgets whose values have changed.
 * - **Full**: All widget values (periodic resync).
 *
 * All methods write to caller-provided buffers (no heap allocation).
 */

#ifndef DASH_JSON_ENGINE_H
#define DASH_JSON_ENGINE_H

#include "../widgets/DashWidget.h"
#include "../core/DashConfig.h"
#include <cstddef>
#include <cstdint>

namespace dash {

class DashJsonEngine {
public:
    DashJsonEngine() = default;

    /**
     * @brief Serialize full configuration for all widgets.
     *
     * Output format:
     * {"type":"config","widgets":[...full widget objects...]}
     *
     * @param widgets  Array of widget pointers.
     * @param count    Number of widgets.
     * @param buf      Output buffer.
     * @param size     Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeConfig(DashWidget* const* widgets, uint8_t count,
                        char* buf, size_t size) const;

    /**
     * @brief Serialize only dirty (changed) widgets as a delta update.
     *
     * Output format:
     * {"type":"delta","widgets":[...delta objects...]}
     *
     * @param widgets  Array of widget pointers.
     * @param count    Number of widgets.
     * @param buf      Output buffer.
     * @param size     Buffer capacity.
     * @return Characters written, or -1 on error. Returns 0 if nothing dirty.
     */
    int serializeDelta(DashWidget* const* widgets, uint8_t count,
                       char* buf, size_t size) const;

    /**
     * @brief Serialize all widget values (for periodic full resync).
     *
     * Output format:
     * {"type":"update","widgets":[...delta-format objects for all...]}
     *
     * @param widgets  Array of widget pointers.
     * @param count    Number of widgets.
     * @param buf      Output buffer.
     * @param size     Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeFullUpdate(DashWidget* const* widgets, uint8_t count,
                            char* buf, size_t size) const;

private:
    /**
     * @brief Append a widget's serialization to the buffer at position pos.
     * @param widget   The widget to serialize.
     * @param buf      Output buffer start.
     * @param size     Total buffer capacity.
     * @param pos      Current write position (updated on return).
     * @param full     If true, serialize full config; otherwise delta.
     * @param isFirst  If true, don't prepend comma.
     * @return true on success, false on overflow.
     */
    bool appendWidget(const DashWidget* widget, char* buf, size_t size,
                      int& pos, bool full, bool isFirst) const;
};

} // namespace dash

#endif // DASH_JSON_ENGINE_H
