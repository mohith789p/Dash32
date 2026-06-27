/**
 * @file DashCard.h
 * @brief Card widget — displays a labeled value with optional unit.
 *
 * The most common widget type. Shows a title, a numeric or string value,
 * and a unit badge. Supports both pointer-binding and manual value setting.
 *
 * Usage:
 * @code
 *   // Pointer mode
 *   float temp = 22.5f;
 *   dashboard.addCard("Temperature", &temp, "°C");
 *
 *   // Manual mode
 *   DashCard* card = dashboard.addCard("Status");
 *   card->setValue("Online");
 * @endcode
 */

#ifndef DASH_CARD_H
#define DASH_CARD_H

#include "DashWidget.h"
#include "../core/ValueHolder.h"

namespace dash {

class DashCard : public DashWidget {
public:
    /**
     * @brief Construct a card with title and optional unit.
     * @param title  Display title.
     * @param unit   Unit string (may be nullptr).
     */
    explicit DashCard(const char* title, const char* unit = nullptr);

    // ----- Pointer binding -----

    /** @brief Bind the card value to a float pointer. */
    void bindValue(float* ptr)  { _value.bind(ptr); markDirty(); }

    /** @brief Bind the card value to a double pointer. */
    void bindValue(double* ptr) { _value.bind(ptr); markDirty(); }

    /** @brief Bind the card value to an int pointer. */
    void bindValue(int* ptr)    { _value.bind(ptr); markDirty(); }

    /** @brief Bind the card value to a long pointer. */
    void bindValue(long* ptr)   { _value.bind(ptr); markDirty(); }

    /** @brief Bind the card value to a bool pointer. */
    void bindValue(bool* ptr)   { _value.bind(ptr); markDirty(); }

    // ----- Manual setters -----

    /** @brief Set the card value to a float. */
    void setValue(float val)       { _value.set(val); markDirty(); }

    /** @brief Set the card value to a double. */
    void setValue(double val)      { _value.set(val); markDirty(); }

    /** @brief Set the card value to an int. */
    void setValue(int val)         { _value.set(val); markDirty(); }

    /** @brief Set the card value to a long. */
    void setValue(long val)        { _value.set(val); markDirty(); }

    /** @brief Set the card value to a bool. */
    void setValue(bool val)        { _value.set(val); markDirty(); }

    /** @brief Set the card value to a string. */
    void setValue(const char* val) { _value.set(val); markDirty(); }

    // ----- DashWidget interface -----

    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;

private:
    ValueHolder _value;
};

} // namespace dash

#endif // DASH_CARD_H
