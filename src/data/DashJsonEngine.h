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

struct DashboardSysInfo {
    const char* title;
    const char* ip;
    const char* hostname;
    uint16_t httpPort;
    uint16_t wsPort;
    uint8_t clients;
    int rssi;
    const char* ssid;
    const char* model;
    uint32_t heap;
    uint32_t uptime;
};

class DashJsonEngine {
public:
    DashJsonEngine() = default;

    /**
     * @brief Serialize full configuration for all widgets.
     *
     * Output format:
     * {"type":"config","title":"Title","sys":{...},"widgets":[...full widget objects...]}
     *
     * @param sys      System metadata and stats.
     * @param widgets  Array of widget pointers.
     * @param count    Number of widgets.
     * @param buf      Output buffer.
     * @param size     Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeConfig(const DashboardSysInfo& sys,
                        DashWidget* const* widgets, uint8_t count,
                        char* buf, size_t size) const;

    /**
     * @brief Serialize only dirty (changed) widgets as a delta update.
     *
     * Output format:
     * {"type":"delta","sys":{...},"widgets":[...delta objects...]}
     *
     * @param sys      System metadata and stats.
     * @param widgets  Array of widget pointers.
     * @param count    Number of widgets.
     * @param buf      Output buffer.
     * @param size     Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeDelta(const DashboardSysInfo& sys,
                       DashWidget* const* widgets, uint8_t count,
                       char* buf, size_t size) const;

    /**
     * @brief Serialize all widget values (for periodic full resync).
     *
     * Output format:
     * {"type":"update","sys":{...},"widgets":[...delta-format objects for all...]}
     *
     * @param sys      System metadata and stats.
     * @param widgets  Array of widget pointers.
     * @param count    Number of widgets.
     * @param buf      Output buffer.
     * @param size     Buffer capacity.
     * @return Characters written, or -1 on error.
     */
    int serializeFullUpdate(const DashboardSysInfo& sys,
                            DashWidget* const* widgets, uint8_t count,
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
