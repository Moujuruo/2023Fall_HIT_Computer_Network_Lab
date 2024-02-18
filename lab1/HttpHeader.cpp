#include "HttpHeader.h"

HttpHeader::HttpHeader() {
    memset(this, 0, sizeof(HttpHeader));
}

const char* HttpHeader::getMethod() const {
    return method;
}

void HttpHeader::setMethod(const char* method) {
    strncpy(this->method, method, sizeof(this->method) - 1);
}

const char* HttpHeader::getUrl() const {
    return url;
}

void HttpHeader::setUrl(const char* url, int length) {
    strncpy(this->url, url, length);
}

const char* HttpHeader::getHost() const {
    return host;
}

void HttpHeader::setHost(const char* host, int length) {
    strncpy(this->host, host, length);
}

const char* HttpHeader::getCookie() const {
    return cookie;
}

void HttpHeader::setCookie(const char* cookie, int length) {
    strncpy(this->cookie, cookie, length);
}