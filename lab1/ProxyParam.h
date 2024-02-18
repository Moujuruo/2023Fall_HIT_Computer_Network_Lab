#pragma once
#include <WinSock2.h>

class ProxyParam {
public:
    SOCKET getClientSocket() const;
    void setClientSocket(SOCKET clientSocket);

    SOCKET getServerSocket() const;
    void setServerSocket(SOCKET serverSocket);

private:
    SOCKET clientSocket;
    SOCKET serverSocket;
};