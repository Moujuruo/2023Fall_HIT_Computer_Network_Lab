#include "ProxyParam.h"

SOCKET ProxyParam::getClientSocket() const {
    return clientSocket;
}

void ProxyParam::setClientSocket(SOCKET clientSocket) {
    this->clientSocket = clientSocket;
}

SOCKET ProxyParam::getServerSocket() const {
    return serverSocket;
}

void ProxyParam::setServerSocket(SOCKET serverSocket) {
    this->serverSocket = serverSocket;
}