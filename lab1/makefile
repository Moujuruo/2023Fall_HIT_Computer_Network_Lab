CXXFLAGS=-g

all: ProxyServer

ProxyServer: main.o ProxyServer.o ProxyParam.o HttpHeader.o
	g++ main.o ProxyServer.o ProxyParam.o HttpHeader.o -o ProxyServer -lws2_32

main.o: main.cpp
	g++ $(CXXFLAGS) -c main.cpp

ProxyServer.o: ProxyServer.cpp
	g++ $(CXXFLAGS) -c ProxyServer.cpp

ProxyParam.o: ProxyParam.cpp
	g++ $(CXXFLAGS) -c ProxyParam.cpp

HttpHeader.o: HttpHeader.cpp
	g++ $(CXXFLAGS) -c HttpHeader.cpp

.PHONY: clean
clean:
	del -rf *.o ProxyServer.exe
