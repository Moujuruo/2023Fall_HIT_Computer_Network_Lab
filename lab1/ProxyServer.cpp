#include "ProxyServer.h"
#include <iostream>

#define MAXSIZE 65507 //发送数据报文的最大长度 
#define HTTP_PORT 80 //http 服务器端口
bool cacheFlag = false, needCache = true;


ProxyServer::ProxyServer() {
    // 初始化Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 初始化代理服务器套接字
    proxyServer = socket(AF_INET, SOCK_STREAM, 0);

    proxyServerAddr.sin_family = AF_INET;  // IPv4
    proxyServerAddr.sin_addr.S_un.S_addr = INADDR_ANY; // 任意地址
    proxyServerAddr.sin_port = htons(proxyServerPort); // 端口，转换为网络字节序
}

ProxyServer::~ProxyServer() {
    // 关闭代理服务器套接字
    closesocket(proxyServer);

    // 清理Winsock
    WSACleanup();
}


int ProxyServer::run() {
    std::cout << "正在初始化套接字..." << std::endl;
    if (!initSocket()) {
        std::cerr << "初始化套接字失败！" << std::endl;
        return 0;
    }
    std::cout << "初始化套接字成功！" << std::endl;
    std::cout << "正在监听端口" << proxyServerPort << "..." << std::endl;
    SOCKET accepetSocket = INVALID_SOCKET;
    ProxyParam *lpProxyParam;
    HANDLE hThread;
    DWORD dwThreadId;
    while (true) {
        cacheFlag = false;
        needCache = true;
        accepetSocket = accept(proxyServer, NULL, NULL); // 接受连接
        lpProxyParam = new ProxyParam;  // 为线程参数分配内存
        if (lpProxyParam == NULL) {
            std::cerr << "创建线程参数失败！" << std::endl;
            continue;
        }
        lpProxyParam->setClientSocket(accepetSocket);
        hThread = (HANDLE)_beginthreadex(NULL, 0,
                                         &ProxyServer::proxyThread, (LPVOID)lpProxyParam, 0, 0);
                                        //  _beginthreadex()函数用于创建一个线程，该函数的第一个参数为线程安全属性，第二个参数为堆栈大小，第三个参数为线程函数，第四个参数为传递给线程函数的参数，第五个参数为线程创建后的初始状态，第六个参数为线程ID
                                        // 用到了库函数process.h
        CloseHandle(hThread);           // 关闭线程句柄
        Sleep(200);
    }
    closesocket(proxyServer);
    WSACleanup();
    return 0;
}

bool ProxyServer::initSocket() {
    // 绑定代理服务器套接字
    if (bind(proxyServer, (SOCKADDR *)&proxyServerAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        return false;
    }

    // 监听代理服务器套接字
    if (listen(proxyServer, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen socket." << std::endl;
        return false;
    }

    return true;
}

void ProxyServer::getRedirect(char *buffer, const char *url) {
    // 取·buffer 第一行 第一个空格之前的内容
    char *p = strstr(buffer, " ");
    char temp[MAXSIZE];
    const char *content = "HTTP/1.1 302 Moved Temporarily\r\n";
    ZeroMemory(temp, sizeof(temp));
    // content -> temp
    memcpy(temp, content, strlen(content));
    const char *content2 = "Connection:keep-alive\r\n";
    strcat(temp, content2);
    const char *content3 = "Cache-Control:max-age=0\r\n";
    strcat(temp, content3);
    const char *content4 = "Location: ";
    strcat(temp, content4);
    strcat(temp, url);
    strcat(temp, "\r\n\r\n");
    ZeroMemory(buffer, strlen(buffer));
    memcpy(buffer, temp, strlen(temp));   
    printf("改造后的响应头：\n----------------\n%s\n----------------\n", buffer);
}


unsigned int __stdcall ProxyServer::proxyThread(LPVOID lpParameter) {

    char buffer[MAXSIZE], fileBuffer[MAXSIZE];
    char *redirectBuffer;
    char *cacheBuffer, *dateBuffer;
    char fileName[1024];
    char hostname[1024];
    bool redirectFlag = false;


    memset(buffer, 0, sizeof(buffer));
    memset(fileBuffer, 0, sizeof(fileBuffer));
    
    int recvSize;
    int ret;
    recvSize = recv(((ProxyParam *)lpParameter)->getClientSocket(), buffer, MAXSIZE, 0); // 接收数据
    HttpHeader *httpHeader = new HttpHeader();
    cacheBuffer = new char[recvSize + 1];
    memset(cacheBuffer, 0, recvSize + 1);
    memcpy(cacheBuffer, buffer, recvSize);
    parseHttpHead(cacheBuffer, httpHeader);
    if (!strlen(httpHeader->getMethod())) {
        Sleep(200);
        closesocket(((ProxyParam *)lpParameter)->getClientSocket());
        closesocket(((ProxyParam *)lpParameter)->getServerSocket());
        delete (ProxyParam *)lpParameter;
        _endthreadex(0);
        return 0;
    }
    dateBuffer = new char[recvSize + 1];
    memset(dateBuffer, 0, strlen(buffer) + 1);
    memcpy(dateBuffer, buffer, strlen(buffer) + 1);

    memset(fileName, 0, sizeof(fileName));
    
    if (!strcmp(httpHeader->getHost(), "jwes.hit.edu.cn")) {
        printf("\n=====================================\n\n");
        printf("钓鱼成功：您所前往的网站已被引导至http://jwts.hit.edu.cn\n");
        httpHeader->setHost("jwts.hit.edu.cn", 22);
        httpHeader->setUrl("http://jwts.hit.edu.cn", 22);
        redirectFlag = true;
    }

    getFilename(httpHeader->getUrl(), fileName, sizeof(fileName)); // 获取文件名
    printf("文件名：%s\n", fileName);

    int ret_new = gethostname(hostname, sizeof(hostname));         // 获取主机名
    printf("主机名：%s\n----------------\n", hostname);

    HOSTENT *hostent = gethostbyname(hostname);                 // 获取主机信息
    char *ip = inet_ntoa(*(in_addr *)*hostent->h_addr_list);    // 获取IP地址
    if (!strcmp(ip, "127.0.0.1")) {
        printf("本地访问\n您的主机被屏蔽\n");
        goto error;
    }

    if (strstr(httpHeader->getUrl(), "gov.cn")) {
        printf("本地访问%s\n您的主机被屏蔽\n", httpHeader->getUrl());
        goto error;
    }

    FILE *in;
    char date[50];
    memset(date, 0, sizeof(date));
    if (fopen_s(&in, fileName, "rb") == 0) {
        printf("缓存命中\n");
        fread(fileBuffer, sizeof(char), MAXSIZE, in);
        fclose(in);
        getDate(fileBuffer, date);
        printf("缓存时间：%s\n", date);
        changeHTTP(buffer, date);
        // printf("改造后的请求头：\n----------------\n%s\n----------------\n", buffer);
        cacheFlag = true;
        goto success;
    }

    delete cacheBuffer;

success:
    SOCKET serverSockett;
    serverSockett = ((ProxyParam *)lpParameter)->getServerSocket();
    if (strlen(httpHeader->getHost()) == 0)
        goto error;
    if (!connectToServer(&serverSockett, httpHeader->getHost())) {
        printf("连接服务器失败！%s \n", httpHeader->getHost());
        goto error;
    }
    ((ProxyParam *)lpParameter)->setServerSocket(serverSockett);
    if (!redirectFlag) 
        printf("连接服务器成功！\n请求报文: \n%s\n", buffer);
    ret = send(((ProxyParam *)lpParameter)->getServerSocket(), buffer, strlen(buffer) + 1, 0); // 发送数据
    if (ret == -1) {
        int errCode = WSAGetLastError();
        printf("发送数据失败，错误代码：%d\n", errCode);
    }
    recvSize = recv(((ProxyParam *)lpParameter)->getServerSocket(), buffer, MAXSIZE, 0); // 接收数据
    if (recvSize <= 0) {
        printf("接收数据失败！\n");
        goto error;
    }
    // if (redirectFlag) {
    //     redirectBuffer = new char[recvSize + 1]; 
    //     memset(redirectBuffer, 0, strlen(buffer) + 1);
    //     memcpy(redirectBuffer, buffer, strlen(buffer) + 1);
    //     getRedirect(redirectBuffer, httpHeader->getUrl());
    //     ret = send(((ProxyParam *)lpParameter)->getClientSocket(), redirectBuffer, strlen(redirectBuffer) + 1, 0); // 发送数据
    //     goto error;
    // }
    if (cacheFlag) {
        getCache(buffer, fileName);
    }
    if (!redirectFlag)
        printf("响应报文: \n%s\n---------------\n", buffer);
    if (needCache) {
        saveCache(buffer, httpHeader->getUrl());
    }
    std::cout << "clientSocket: " << ((ProxyParam *)lpParameter)->getClientSocket() << std::endl;
    ret = send(((ProxyParam *)lpParameter)->getClientSocket(), buffer, sizeof(buffer), 0); // 发送数据

error:
    Sleep(200);
    closesocket(((ProxyParam *)lpParameter)->getClientSocket());
    closesocket(((ProxyParam *)lpParameter)->getServerSocket());
    delete (ProxyParam *)lpParameter;
    _endthreadex(0);
    return 0;
}

void ProxyServer::parseHttpHead(char *buffer, HttpHeader *httpHeader) {
    char *p;
    char *ptr;
    const char *split = "\r\n";
    p = strtok_s(buffer, split, &ptr); // 分割字符串
    // strtok_s()函数用于分割字符串，第一个参数为要分割的字符串，第二个参数为分割字符串，第三个参数为保存分割结果的指针
    if (p[0] == 'G') { // GET 
        httpHeader->setMethod("GET");
        httpHeader->setUrl(p + 4, strlen(p) - 13);
    }
    else if (p[0] == 'P') { // POST
        httpHeader->setMethod("POST");
        httpHeader->setUrl(p + 5, strlen(p) - 14);
    }
    else {
        // printf("暂不支持该方法！%s\n", p);
        return;
    }
    p = strtok_s(NULL, split, &ptr); // NULL表示使用上一次的分割结果
    while (p) {
        if (p[0] == 'H') { // Host
            httpHeader->setHost(p + 6, strlen(p) - 6);
        }
        else if (p[0] == 'C') { // Cookie
            if (strlen(p) > 8) {
                char header[8];
                ZeroMemory(header, sizeof(header)); // zeromemory函数用于将内存块清零, 和memset的区别是前者是系统函数, 后者是C函数
                memcpy(header, p, 6);
                if (strcmp(header, "Cookie") == 0) {
                    httpHeader->setCookie(p + 8, strlen(p) - 8);
                }
            }
        }
        p = strtok_s(NULL, split, &ptr);
    }
}

bool ProxyServer::connectToServer(SOCKET *serverSocket, const char *host) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(HTTP_PORT); // 端口，转换为网络字节序
    HOSTENT *hostent = gethostbyname(host); // 获取主机信息
    if (!hostent) {
        printf("获取主机信息失败！\n错误代码为%d\n", WSAGetLastError());
        return false;
    }
    in_addr Inaddr = *((in_addr *)*hostent->h_addr_list); // 获取主机地址
    serverAddr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(Inaddr)); // 点分十进制转换为网络字节序
    *serverSocket = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字
    if (INVALID_SOCKET == *serverSocket) {
        printf("创建套接字失败！\n错误代码为%d\n", WSAGetLastError());
        return false;
    }
    if (connect(*serverSocket, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) { // 连接服务器
        printf("连接服务器失败！\n错误代码为%d\n", WSAGetLastError());
        return false;
    }
    return true;
}

void ProxyServer::getFilename(const char *url, char *filename, size_t maxLen) {
    size_t len = strlen(url);
    size_t copyLen = (len < maxLen) ? len : maxLen - 1; // 考虑数组边界，保留一个位置给字符串结尾的'\0'
    size_t filenameIndex = 0;

    for (size_t i = 0; i < copyLen; i++) {
        if (url[i] != '/' && url[i] != ':' && url[i] != '.') { // 过滤掉非法字符
            filename[filenameIndex++] = url[i];
        }
    }

    filename[filenameIndex] = '\0'; // 添加字符串结尾标志
}

bool ProxyServer::getDate(char *buffer, char *date) {
    char *p;
    char *ptr;
    const char *split = "\r\n";
    p = strtok_s(buffer, split, &ptr); // 分割字符串
    while (p) {
        if (p[0] == 'D') { // Date
            if (strlen(p) > 6) {
                char header[6];
                ZeroMemory(header, sizeof(header));
                memcpy(header, p, 4);
                if (strcmp(header, "Date") == 0) {
                    memcpy(date, p + 6, strlen(p) - 6);
                    return true;
                }
            }
        }
        p = strtok_s(NULL, split, &ptr);
    }
    return false;
}

void ProxyServer::changeHTTP(char *buffer, char *value) {
    const char* fieldToReplace = "Host";
    const char* newField = "If-Modified-Since: ";
    char* pos = strstr(buffer, fieldToReplace);
    char *check = strstr(buffer, newField);
    char *pos1 = strstr(pos, "\r\n");
    if (check != NULL) {
        char temp[MAXSIZE];
        strncpy(temp, buffer, pos - buffer);
        strncat(temp, pos, pos1 - pos);
        char* oldHostEnd = strstr(pos + strlen(fieldToReplace), "\r\n");
        if (oldHostEnd != NULL) {
            // 要把host后面到newField前面的内容补上
            strncat(temp, oldHostEnd, check - oldHostEnd);
        }
        strcat(temp, newField);
        strcat(temp, value);
        strcat(temp, "\r\n");
        // 要把 newField 后面的内容补上
        char *oldHostEnd1 = strstr(check + strlen(newField), "\r\n");
        if (oldHostEnd1 != NULL) {
            strcat(temp, oldHostEnd1);
        }
        ZeroMemory(buffer, strlen(buffer));
        memcpy(buffer, temp, strlen(temp));
    }
    else if (pos != NULL) {
        char temp[MAXSIZE];
        strncpy(temp, buffer, pos - buffer);
        strcat(temp, newField);
        strcat(temp, value);
        strcat(temp, "\r\n");
        // 要把 host 那行补上
        strncat(temp, pos, pos1 - pos);

        char* oldHostEnd = strstr(pos + strlen(fieldToReplace), "\r\n");
        if (oldHostEnd != NULL) {
            strcat(temp, oldHostEnd);
        }
        ZeroMemory(buffer, strlen(buffer));
        memcpy(buffer, temp, strlen(temp));
    }
}

bool getFileContents(char *filename, char *buffer, size_t bufferSize) {
    FILE *file;
    if (fopen_s(&file, filename, "rb") == 0) {
        size_t bytesRead = fread(buffer, sizeof(char), bufferSize, file);
        fclose(file);
        return bytesRead > 0;
    }
    return false;
}

void ProxyServer::getCache(char *buffer, char *filename) {
    const char *split = "\r\n";
    char *p, *ptr, status[4];
    char tempBuffer[MAXSIZE + 1];

    memset(status, 0, sizeof(status));
    memset(tempBuffer, 0, sizeof(tempBuffer));
    strncpy(tempBuffer, buffer, sizeof(tempBuffer) - 1);

    p = strtok_s(tempBuffer, split, &ptr); // 提取第一行
    strncpy(status, p + 9, sizeof(status) - 1);
    printf("获取状态码：%s\n", status);

    if (strcmp(status, "304") == 0) {
        printf("获取本地缓存！\n");
        if (getFileContents(filename, buffer, MAXSIZE)) {
            // 缓存文件内容成功
            needCache = false;
        }
    }
}

bool writeFile(const char *filename, const char *data) {
    FILE *file;
    if (fopen_s(&file, filename, "wb") == 0) {  // wb 以二进制写入
        size_t bytesWritten = fwrite(data, sizeof(char), strlen(data), file); // 写入文件
        fclose(file);
        return bytesWritten > 0;
    }
    return false;
}
void ProxyServer::saveCache(char *buffer, const char *url) {
    const char *split = "\r\n";
    char *p, *ptr, status[4];
    char tempBuffer[MAXSIZE + 1];

    memset(status, 0, sizeof(status));
    memset(tempBuffer, 0, sizeof(tempBuffer));
    strncpy(tempBuffer, buffer, sizeof(tempBuffer) - 1);

    p = strtok_s(tempBuffer, split, &ptr); // 提取第一行
    strncpy(status, p + 9, sizeof(status) - 1); // 提取状态码

    if (strcmp(status, "200") == 0) {
        char filename[100];
        memset(filename, 0, sizeof(filename));
        getFilename(url, filename, sizeof(filename)); // 获取文件名
        if (writeFile(filename, buffer)) {
            printf("缓存成功！\n");
        }
        else {
            printf("缓存失败：无法写入文件。\n");
        }
    }
}