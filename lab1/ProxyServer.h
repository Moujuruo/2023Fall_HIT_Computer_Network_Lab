#pragma once
#include "HttpHeader.h"
#include "ProxyParam.h"
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib,"Ws2_32.lib") 


class ProxyServer {
    
public:
    ProxyServer();
    ~ProxyServer();

    int run();


private:
    bool initSocket();
    static void parseHttpHead(char *buffer, HttpHeader *httpHeader);
    static bool connectToServer(SOCKET *serverSocket, const char *host);
    static unsigned int __stdcall proxyThread(LPVOID lpParameter);
    static void getFilename(const char *url, char *filename, size_t maxLen);
    static bool getDate(char *buffer, char *date);
    static void changeHTTP(char *buffer, char *value);
    static void getCache(char *buffer, char *filename);
    static void saveCache(char *buffer, const char *url);
    static void getRedirect(char *buffer, const char *url);

    SOCKET proxyServer;
    sockaddr_in proxyServerAddr;
    const int proxyServerPort = 10240;
};