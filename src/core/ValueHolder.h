/**
 * @file ValueHolder.h
 * @brief Type-erased value container for dashboard widgets.
 *
 * ValueHolder stores a single value that can be of any supported type
 * (float, double, int, long, bool, or a fixed char buffer). It supports
 * two modes:
 *
 * - **Pointer mode**: The holder reads from an external pointer on each
 *   change-detection check. The developer updates the original variable.
 * - **Manual mode**: The developer calls a setter to push a new value.
 *
 * Change detection is built-in: `hasChanged()` compares the current value
 * against a cached snapshot and returns true if different.
 *
 * @note This class avoids heap allocation entirely.
 */

#ifndef DASH_VALUE_HOLDER_H
#define DASH_VALUE_HOLDER_H

#include "DashConfig.h"
#include "DashTypes.h"
#include <cstring>

namespace dash {

class ValueHolder {
public:
    ValueHolder();

    // ----- Pointer binding (auto-mode) -----

    /** @brief Bind to an external float variable. */
    void bind(float* ptr);

    /** @brief Bind to an external double variable. */
    void bind(double* ptr);

    /** @brief Bind to an external int variable. */
    void bind(int* ptr);

    /** @brief Bind to an external long variable. */
    void bind(long* ptr);

    /** @brief Bind to an external bool variable. */
    void bind(bool* ptr);

    // ----- Manual setters -----

    /** @brief Manually set a float value. */
    void set(float val);

    /** @brief Manually set a double value. */
    void set(double val);

    /** @brief Manually set an integer value. */
    void set(int val);

    /** @brief Manually set a long value. */
    void set(long val);

    /** @brief Manually set a boolean value. */
    void set(bool val);

    /** @brief Manually set a string value (copied into internal buffer). */
    void set(const char* val);

    // ----- State queries -----

    /**
     * @brief Check if the value has changed since the last call to snapshot().
     *
     * For pointer mode, reads the current value from the bound pointer.
     * For manual mode, compares against the last-snapshot value.
     *
     * @return true if the value differs from the cached snapshot.
     */
    bool hasChanged() const;

    /**
     * @brief Save the current value as the cached snapshot.
     *
     * Call this after serializing to mark the value as "sent".
     */
    void snapshot();

    /** @brief Get the current value mode. */
    ValueMode mode() const { return _mode; }

    /** @brief Get the underlying data type. */
    ValueType type() const { return _type; }

    /** @brief Returns true if the holder has a bound pointer or manual value. */
    bool isBound() const { return _mode != ValueMode::Unbound; }

    // ----- Serialization -----

    /**
     * @brief Write the current value as a JSON value fragment into a buffer.
     *
     * Examples of output: `23.50`, `true`, `"hello"`.
     * Does NOT write a key — only the value portion.
     *
     * @param buf  Output buffer.
     * @param size Buffer capacity.
     * @return Number of characters written (excluding null), or -1 on error.
     */
    int toJson(char* buf, size_t size) const;

private:
    /** Union of typed pointers for pointer-mode binding. */
    union Pointer {
        float*  f;
        double* d;
        int*    i;
        long*   l;
        bool*   b;
    };

    /** Union of cached values for change detection. */
    struct Cached {
        union {
            float   f;
            double  d;
            int     i;
            long    l;
            bool    b;
        };
        char str[DASH_STRING_VALUE_MAX_LEN];
    };

    ValueType  _type;
    ValueMode  _mode;
    Pointer    _ptr;
    Cached     _current;    ///< Live value (manual mode) or read cache
    Cached     _snapshot;   ///< Last-sent value for change detection

    /** Read the current value from the pointer into _current. */
    void readFromPointer();

    /** Compare _current against _snapshot. */
    bool differs() const;
};

} // namespace dash

#endif // DASH_VALUE_HOLDER_H
