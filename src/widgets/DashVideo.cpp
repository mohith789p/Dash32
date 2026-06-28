/**
 * @file DashVideo.cpp
 * @brief Video widget implementation.
 */

#include "DashVideo.h"
#include <cstdio>
#include <cstring>

namespace dash {

DashVideo::DashVideo(const char* title)
    : DashWidget(title, nullptr, WidgetType::Video)
    , _aspectW(16)
    , _aspectH(9)
    , _autoplay(true)
    , _muted(true)
    , _controls(false)
    , _fullscreen(true)
    , _reconnectInterval(3000)
{
    _url[0] = '\0';
    _lastUrl[0] = '\0';
    _lastUrl[0] = 1; // force change on first update
    _lastUrl[1] = '\0';
}

void DashVideo::setURL(const char* url) {
    safeCopy(_url, url, DASH_URL_MAX_LEN);
    markDirty();
}

void DashVideo::setAspectRatio(uint8_t width, uint8_t height) {
    _aspectW = width;
    _aspectH = height;
    markDirty();
}

void DashVideo::setAutoplay(bool autoplay) {
    _autoplay = autoplay;
    markDirty();
}

void DashVideo::setMuted(bool muted) {
    _muted = muted;
    markDirty();
}

void DashVideo::showControls(bool show) {
    _controls = show;
    markDirty();
}

void DashVideo::enableFullscreen(bool enable) {
    _fullscreen = enable;
    markDirty();
}

void DashVideo::setReconnectInterval(uint32_t ms) {
    _reconnectInterval = ms;
    markDirty();
}

bool DashVideo::checkForChange() {
    if (strncmp(_url, _lastUrl, DASH_URL_MAX_LEN) != 0) {
        memcpy(_lastUrl, _url, DASH_URL_MAX_LEN);
        _dirty = true;
        return true;
    }
    return false;
}

int DashVideo::serializeValue(char* buf, size_t size) const {
    (void)buf;
    (void)size;
    return 0;
}

int DashVideo::serializeConfig(char* buf, size_t size) const {
    int pos = snprintf(buf, size,
        ",\"aspectW\":%u,\"aspectH\":%u,\"autoplay\":%s,\"muted\":%s,"
        "\"controls\":%s,\"fullscreen\":%s,\"reconnect\":%u,\"url\":\"",
        _aspectW, _aspectH,
        _autoplay ? "true" : "false",
        _muted ? "true" : "false",
        _controls ? "true" : "false",
        _fullscreen ? "true" : "false",
        _reconnectInterval);
    if (pos < 0 || static_cast<size_t>(pos) >= size) return -1;

    size_t urlPos = pos;
    for (size_t i = 0; _url[i] != '\0'; ++i) {
        char c = _url[i];
        if (c == '"' || c == '\\') {
            if (urlPos + 2 >= size - 1) break;
            buf[urlPos++] = '\\';
            buf[urlPos++] = c;
        } else if (static_cast<unsigned char>(c) >= 0x20) {
            if (urlPos + 1 >= size - 1) break;
            buf[urlPos++] = c;
        }
    }

    if (urlPos + 1 >= size) return -1;
    buf[urlPos++] = '"';
    buf[urlPos] = '\0';

    return static_cast<int>(urlPos);
}

} // namespace dash
