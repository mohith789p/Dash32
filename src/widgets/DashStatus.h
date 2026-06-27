/**
 * @file DashStatus.h
 * @brief Status indicator widget — shows a label with severity level.
 *
 * Rendered as a colored badge (OK=green, Warning=amber, Error=red, Info=blue).
 *
 * Usage:
 * @code
 *   DashStatus* s = dashboard.addStatus("WiFi");
 *   s->setStatus("Connected", StatusLevel::OK);
 *   s->setStatus("Reconnecting...", StatusLevel::Warning);
 * @endcode
 */

#ifndef DASH_STATUS_H
#define DASH_STATUS_H

#include "DashWidget.h"
#include "../core/DashConfig.h"
#include "../core/DashTypes.h"

namespace dash {

class DashStatus : public DashWidget {
public:
    /**
     * @brief Construct a status widget.
     * @param title Display title.
     */
    explicit DashStatus(const char* title);

    /**
     * @brief Set the status label and severity level.
     * @param label  Status text (max DASH_STATUS_LABEL_MAX_LEN - 1 chars).
     * @param level  Severity level.
     */
    void setStatus(const char* label, StatusLevel level = StatusLevel::Info);

    /** @brief Get the current status label. */
    const char* getLabel() const { return _label; }

    /** @brief Get the current severity level. */
    StatusLevel getLevel() const { return _level; }

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;

private:
    char        _label[DASH_STATUS_LABEL_MAX_LEN];
    char        _lastLabel[DASH_STATUS_LABEL_MAX_LEN];
    StatusLevel _level;
    StatusLevel _lastLevel;

    /** @brief Get CSS-friendly name for a status level. */
    static const char* levelName(StatusLevel level);
};

} // namespace dash

#endif // DASH_STATUS_H
