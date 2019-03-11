all: Packet.o route.o RoutingManager.o TableConstructs.o
	g++ route.o Packet.o RoutingManager.o TableConstructs.o -o vrouter -std=c++11 -g

release:
	g++ route.cpp Packet.cpp RoutingManager.cpp TableConstructs.cpp -o vrouter -std=c++11 -O3

Packet.o: Packet.cpp
	g++ Packet.cpp -c -std=c++11 -g

route.o: route.cpp
	g++ route.cpp -c -std=c++11 -g

TableConstructs.o: TableConstructs.cpp
	g++ TableConstructs.cpp -c -std=c++11 -g

RoutingManager.o: RoutingManager.cpp
	g++ RoutingManager.cpp -c -std=c++11 -g

clean:
	rm -rf *.o vrouter
