all: Packet.o route.o
	g++ route.o Packet.o -o vr -std=c++11

Packet.o: Packet.cpp
	g++ Packet.cpp -c -std=c++11

route.o: route.cpp
	g++ route.cpp -c -std=c++11
