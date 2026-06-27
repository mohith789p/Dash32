/**
 * @file DashText.h
 * @brief Text display widget — shows a block of text content.
 *
 * Usage:
 * @code
 *   DashText* txt = dashboard.addText("Log");
 *   txt->setText("System initialized successfully");
 * @endcode
 */

#ifndef DASH_TEXT_H
#define DASH_TEXT_H

#include "DashWidget.h"
#include "../core/DashConfig.h"
#include <cstring>

namespace dash {

class DashText : public DashWidget {
public:
    /**
     * @brief Construct a text widget.
     * @param title Display title.
     */
    explicit DashText(const char* title);

    /**
     * @brief Set the text content.
     * @param text Content string (max DASH_TEXT_MAX_LEN - 1 chars).
     */
    void setText(const char* text);

    /** @brief Get the current text content. */
    const char* getText() const { return _text; }

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;

private:
    char _text[DASH_TEXT_MAX_LEN];
    char _lastText[DASH_TEXT_MAX_LEN];
};

} // namespace dash

#endif // DASH_TEXT_H
