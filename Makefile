all: Packet.o route.o
	g++ route.o Packet.o -o vr

Packet.o: Packet.cpp
	g++ Packet.cpp -c

route.o: route.cpp
	g++ route.cpp -c
