all: Packet.o route.o RoutingItem.o RoutingTable.o
	g++ route.o Packet.o RoutingItem.o RoutingTable.o -o vrouter -std=c++11 -g

Packet.o: Packet.cpp
	g++ Packet.cpp -c -std=c++11 -g

route.o: route.cpp
	g++ route.cpp -c -std=c++11 -g

RoutingItem.o: RoutingItem.cpp
	g++ RoutingItem.cpp -c -std=c++11 -g

RoutingTable.o: RoutingTable.cpp
	g++ RoutingTable.cpp -c -std=c++11 -g

clean:
	rm -rf *.o vrouter
