/**
 * @file DashVideo.h
 * @brief Video widget — embeds live video streams (MJPEG/HTTP/etc).
 */

#ifndef DASH_VIDEO_H
#define DASH_VIDEO_H

#include "DashWidget.h"

namespace dash {

class DashVideo : public DashWidget {
public:
    /**
     * @brief Construct a Video widget.
     * @param title Display title.
     */
    explicit DashVideo(const char* title);

    // ----- Config and state setters -----
    void setURL(const char* url);
    void setAspectRatio(uint8_t width, uint8_t height);
    void setAutoplay(bool autoplay);
    void setMuted(bool muted);
    void showControls(bool show);
    void enableFullscreen(bool enable);
    void setReconnectInterval(uint32_t ms);

    // ----- DashWidget interface -----
    bool checkForChange() override;
    int  serializeValue(char* buf, size_t size) const override;
    int  serializeFull(char* buf, size_t size) const override;

private:
    char     _url[DASH_URL_MAX_LEN];
    char     _lastUrl[DASH_URL_MAX_LEN];
    uint8_t  _aspectW;
    uint8_t  _aspectH;
    bool     _autoplay;
    bool     _muted;
    bool     _controls;
    bool     _fullscreen;
    uint32_t _reconnectInterval;
};

} // namespace dash

#endif // DASH_VIDEO_H
