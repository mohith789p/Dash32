/**
 * @file DashImage.cpp
 * @brief Image widget implementation.
 */

#include "DashImage.h"
#include <cstdio>
#include <cstring>

namespace dash {

static const char* fitToString(ImageFit fit) {
    switch (fit) {
        case ImageFit::Contain: return "contain";
        case ImageFit::Cover:   return "cover";
        case ImageFit::Fill:    return "fill";
        default:                return "none";
    }
}

DashImage::DashImage(const char* title)
    : DashWidget(title, nullptr, WidgetType::Image)
    , _fit(ImageFit::Contain)
    , _refreshInterval(0)
    , _fullscreen(true)
    , _border(true)
{
    _url[0] = '\0';
    _lastUrl[0] = '\0';
    _lastUrl[0] = 1; // force change on first update
    _lastUrl[1] = '\0';
}

void DashImage::setURL(const char* url) {
    safeCopy(_url, url, DASH_URL_MAX_LEN);
    markDirty();
}

void DashImage::setFit(ImageFit fit) {
    _fit = fit;
    markDirty();
}

void DashImage::setRefreshInterval(uint32_t ms) {
    _refreshInterval = ms;
    markDirty();
}

void DashImage::enableFullscreen(bool enable) {
    _fullscreen = enable;
    markDirty();
}

void DashImage::showBorder(bool show) {
    _border = show;
    markDirty();
}

bool DashImage::checkForChange() {
    if (strncmp(_url, _lastUrl, DASH_URL_MAX_LEN) != 0) {
        memcpy(_lastUrl, _url, DASH_URL_MAX_LEN);
        _dirty = true;
        return true;
    }
    return false;
}

int DashImage::serializeValue(char* buf, size_t size) const {
    (void)buf;
    (void)size;
    return 0;
}

int DashImage::serializeConfig(char* buf, size_t size) const {
    int pos = snprintf(buf, size,
        ",\"fit\":\"%s\",\"refresh\":%u,\"fullscreen\":%s,\"border\":%s,\"url\":\"",
        fitToString(_fit), _refreshInterval,
        _fullscreen ? "true" : "false",
        _border ? "true" : "false");
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
