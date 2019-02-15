all: Packet.o route.o
	g++ route.o Packet.o -o vrouter -std=c++14

Packet.o: Packet.cpp
	g++ Packet.cpp -c -std=c++14

route.o: route.cpp
	g++ route.cpp -c -std=c++14

clean:
	rm -rf *.o vrouter
