/**
 * @file DashImage.h
 * @brief Image widget — displays static or dynamic images.
 */

#ifndef DASH_IMAGE_H
#define DASH_IMAGE_H

#include "DashWidget.h"

namespace dash {

enum class ImageFit : uint8_t {
    Contain = 0,
    Cover = 1,
    Fill = 2,
    None = 3
};

class DashImage : public DashWidget {
public:
    /**
     * @brief Construct an Image widget.
     * @param title Display title.
     */
    explicit DashImage(const char* title);

    // ----- Config and state setters -----
    void setURL(const char* url);
    void setFit(ImageFit fit);
    void setRefreshInterval(uint32_t ms);
    void enableFullscreen(bool enable);
    void showBorder(bool show);

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;
    int  serializeConfig(char* buf, size_t size) const override;

private:
    char     _url[DASH_URL_MAX_LEN];
    char     _lastUrl[DASH_URL_MAX_LEN];
    ImageFit _fit;
    uint32_t _refreshInterval;
    bool     _fullscreen;
    bool     _border;
};

} // namespace dash

#endif // DASH_IMAGE_H
