all: Packet.o route.o
	clang++ route.o Packet.o -o vr

Packet.o: Packet.cpp
	clang++ Packet.cpp -c

route.o: route.cpp
	clang++ route.cpp -c
