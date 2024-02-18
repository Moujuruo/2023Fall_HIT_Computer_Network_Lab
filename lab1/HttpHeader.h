#pragma once
#include <cstring>

class HttpHeader {
public:
    HttpHeader();

    const char* getMethod() const;
    void setMethod(const char* method);

    const char* getUrl() const;
    void setUrl(const char* url, int length);

    const char* getHost() const;
    void setHost(const char* host, int length);

    const char* getCookie() const;
    void setCookie(const char* cookie, int length);

private:
    char method[4];
    char url[1024];
    char host[1024];
    char cookie[1024 * 10];
};