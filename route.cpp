#include "Packet.h"
#include "RoutingManager.h"
#include "TableConstructs.h"


#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include <ifaddrs.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <queue>

void printForwarding(uint8_t* ipAddress, uint8_t* MacAddress);

int main(int argc, char** argv){
    //Define 2 Packet objects one for reciving, the other for sending.
    shared::Packet sendPacket;
    shared::Packet recivePacket;

    //A vector of socket file descriptors and a select set.
    std::vector<SocketFD> sockets;
    std::vector<std::string> interfaces;
    fd_set socketSetMaster;
    std::string tableFile;

    //Holds packets temporarily until they can be forwarded
    //Also hold a counter that tracks how many times a packet has not been sent
    std::queue<shared::Packet> holdingQueue;
    std::queue<uint8_t> failureTracker;
    std::queue<SocketFD> sendingOrder;

    //Get the routing table 
    if(argc == 2){
        tableFile = argv[1];
    }
    else{
        printf("sudo ./vrouter <routing-table-filename>\n");
        return 1;
    }

    shared::RoutingManager routingManager(argv[1]);


    SocketFD packet_socket;
    //get list of interface addresses. This is a linked list. Next
    //pointer is in ifa_next, interface name is in ifa_name, address is
    //in ifa_addr. You will have multiple entries in the list with the
    //same name, if the same interface has multiple addresses. This is
    //common since most interfaces will have a MAC, IPv4, and IPv6
    //address. You can use the names to match up which IPv4 address goes
    //with which MAC address.
    struct ifaddrs *ifaddr, *tmp;
    if(getifaddrs(&ifaddr)==-1){
        perror("getifaddrs");
        return 1;
    }
    //have the list, loop over the list
    for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
        packet_socket == 0;
        //Check if this is a packet address, there will be one per
        //interface.  There are IPv4 and IPv6 as well, but we don't care
        //about those for the purpose of enumerating interfaces. We can
        //use the AF_INET addresses in this list for example to get a list
        //of our own IP addresses
        //
        //Add a socket on the mac address and add the address to the routing manager.
        if(tmp->ifa_addr->sa_family==AF_PACKET && strcmp(tmp->ifa_name, "lo") != 0){
            printf("Interface: %s\n",tmp->ifa_name);
            printf("Creating Socket on interface %s\n",tmp->ifa_name);
            //create a packet socket
            //AF_PACKET makes it a packet socket
            //SOCK_RAW makes it so we get the entire packet
            //could also use SOCK_DGRAM to cut off link layer header
            //ETH_P_ALL indicates we want all (upper layer) protocols
            //we could specify just a specific one
            packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

            if(packet_socket<0){
                perror("socket");
                return 2;
            }
            //Bind the socket to the address, so we only get packets
            //recieved on this specific interface. For packet sockets, the
            //address structure is a struct sockaddr_ll (see the man page
            //for "packet"), but of course bind takes a struct sockaddr.
            //Here, we can use the sockaddr we got from getifaddrs (which
            //we could convert to sockaddr_ll if we needed to)
            if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
                perror("bind");
            }

            //Push the new socket to the vector and put it onto the fd_set data stack.
            sockets.push_back(packet_socket);
            interfaces.push_back(tmp->ifa_name);

            //Add the mac addr to the routing manager as well as the socket 
            routingManager.addMacMapping(tmp->ifa_name, 
                    ((struct sockaddr_ll*)tmp->ifa_addr)->sll_addr);

            //Add the inteface name and the socket mapping
            routingManager.addSocketMapping(tmp->ifa_name, packet_socket);

            FD_SET(sockets.back(), &socketSetMaster);
        }
        //Add a home address if possible.
        else if(tmp->ifa_addr->sa_family == AF_INET){
            struct sockaddr_in* homeAddress = (struct sockaddr_in*)tmp->ifa_addr;
            uint8_t ipAddress[4];
            memcpy(ipAddress,&homeAddress->sin_addr.s_addr, 4);
            routingManager.addIpMapping(tmp->ifa_name, ipAddress);
        }
    }
    //loop and recieve packets. We are only looking at one interface,
    //for the project you will probably want to look at more (to do so,
    //a good way is to have one socket per interface and use select to
    //see which ones have data)
    printf("Ready to recieve now\n");
    while(1){
        int packet_socket = sockets.front();
        uint8_t buf[1500];
        struct sockaddr_ll recvaddr;
        fd_set cycle = socketSetMaster;
        unsigned int recvaddrlen=sizeof(struct sockaddr_ll);

        int nn = select(FD_SETSIZE, &cycle, NULL, NULL, NULL);

        for(auto socket_it = sockets.begin(); socket_it < sockets.end(); ++socket_it){
            if(FD_ISSET(*socket_it, &cycle)){
                //we can use recv, since the addresses are in the packet, but we
                //use recvfrom because it gives us an easy way to determine if
                //this packet is incoming or outgoing (when using ETH_P_ALL, we
                //see packets in both directions. Only outgoing can be seen when
                //using a packet socket with some specific protocol)
                int n = recvfrom(*socket_it, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);

                //ignore outgoing packets (we can't disable some from being sent
                //by the OS automatically, for example ICMP port unreachable
                //messages, so we will just ignore them here)
                if(recvaddr.sll_pkttype==PACKET_OUTGOING){
                    continue;
                }
                else{
                    //start processing all others
                    printf("\n\nGot a %d byte packet\n", n);
                    recivePacket = shared::Packet(buf);

                    //ARP always works so this should be fine. 
                    if(recivePacket.getType() == shared::ARP_REQUEST){
                        printf("Got an ARP Request packet\n");

                        sendPacket = recivePacket.constructResponseARP(ifaddr);
                        send(*socket_it, sendPacket.data, 42, 0);
                    }
                    else if(recivePacket.getType() == shared::ARP_RESPONSE){
                        printf("Got an ARP response packet \n");

                        struct shared::ForwardingData temp;
                        recivePacket.generateForwardData(temp);

                        temp.sendingSocket = *socket_it;
                        temp.interfaceName = routingManager.findRouting(temp.ipAddress);

                        //Add to forwarding List
                        routingManager.addForwarding(temp);
                        continue;
                    }
                    //Check if the packet is for us.
                    //Do stuff with it if it is.
                    else if(routingManager.isHomeIp(recivePacket.getIPAddress())){

                        if(recivePacket.getType() == shared::ICMP_REQUEST){
                            printf("Got an ICMP packet\n");
                            sendPacket = recivePacket.constructResponseICMP();
                            send(*socket_it, sendPacket.data, 98, 0);
                        }
                    }
                    //Its not for us so we look to forward it.
                    else{

                        //First: Is the checksum correct if not then do something.
                        if(!recivePacket.validIpChecksum()){
                            //Drop the packet
                            continue;
                        }

                        //Second: has the ttl been exausted. if yes send an error 
                        //Decrement the ttl
                        recivePacket.decTTL();
                        if(recivePacket.zeroedTTL()){
                            uint8_t errorType = 11;
                            uint8_t errorCode = 0;

                            sendPacket = shared::Packet(recivePacket, errorType, errorCode);

                            send(*socket_it, sendPacket.data, sendPacket.getPacketLength(), 0);

                            continue;
                        }

                        //Its passed the checks update the checksum after changing ttl
                        recivePacket.calculateIpChecksum();

                        uint8_t* destinationIP = recivePacket.getIPAddress();
                        //Check if we have a mapping already
                        //Would this be going to a router? 
                        destinationIP = (routingManager.hasRouterForward(destinationIP)) ?
                            routingManager.getRouterForward(destinationIP) : destinationIP;

                        struct shared::ForwardingData* forward = 
                            routingManager.findForwarding(destinationIP);

                        if(forward != nullptr){
                            //We have forwarding continue for now
                            recivePacket.updateEthernetHeader(*forward);
                            SocketFD sendingSocket = forward->sendingSocket;

                            //Send the socket
                            send(sendingSocket, recivePacket.data, n, 0);
                            printForwarding(destinationIP, forward->destinationMacAddress);
                            continue;
                        }

                        //No mapping, find the interface assocaiated with the prefix
                        std::string targetInterface = routingManager.findRouting(destinationIP);
                        //If we don't find anything in the table send an ICMP error 
                        if(targetInterface == ""){
                            //TODO implement
                            uint8_t errorType = 3; 
                            uint8_t errorCode = 0;

                            sendPacket = shared::Packet(recivePacket, errorType, errorCode);

                            send(*socket_it, sendPacket.data, sendPacket.getPacketLength(), 0);
                            continue;
                        }
                        //Get everything ready to send out an arp request.
                        else{
                            uint8_t* senderIP = routingManager.getIpAddress(targetInterface);
                            uint8_t* senderMac = routingManager.getMacAddress(targetInterface);
                            uint8_t* targetIP = (routingManager.hasRouterForward(destinationIP)) ? 
                                routingManager.getRouterForward(destinationIP) : destinationIP;
                            sendPacket = shared::Packet(senderIP, senderMac, targetIP);
                            //TODO Send the arp packet out and put the recived on in a queue until 
                            //the arp is recived.
                            holdingQueue.push(recivePacket);
                            failureTracker.push(1);
                            sendingOrder.push(*socket_it);
                            SocketFD sendingSocket = routingManager.getSocketName(targetInterface);
                            send(sendingSocket, sendPacket.data, 42, 0);
                            printf("Arp request was made and sent\n");
                            continue;
                        }
                    }
                }
            }
        }
        //Send all of the packets still in the queue.
        //A packet can only be sent if there is a forwarding mapping 
        int currentSize = holdingQueue.size();
        for(int i = 0; i < currentSize; ++i){
            shared::Packet queuePacket = holdingQueue.front();
            holdingQueue.pop();

            uint8_t attempts = failureTracker.front();
            ++attempts;

            printf("Attempt: %d\n", attempts);
            failureTracker.pop();

            SocketFD returningSocket = sendingOrder.front();
            sendingOrder.pop();


            //Search for the mapping for a sending path if nothing comes up then push this back onto
            //the queue
            uint8_t* destinationIP = queuePacket.getIPAddress();

            //Is this packet going to a router or not if it is, then change the destination ip.
            destinationIP = (routingManager.hasRouterForward(destinationIP)) ? 
                routingManager.getRouterForward(destinationIP) : destinationIP;
            struct shared::ForwardingData* found = routingManager.findForwarding(destinationIP);
            //We found a forwarding. change the ethernet header to update the source and dest.
            if(found != nullptr){
                queuePacket.updateEthernetHeader(*found);                
                unsigned short sendingLength = queuePacket.getPacketLength();
                SocketFD sendingSocket = found->sendingSocket;

                //Send the packet
                send(sendingSocket, queuePacket.data, sendingLength, 0);
                printForwarding(destinationIP, found->destinationMacAddress);
            }
            else{
                //TODO change this to create send an ERROR ICMP and just get rid of the packet.
                if(attempts > 3){

                    uint8_t errorType = 3; 
                    uint8_t errorCode = 1;

                    sendPacket= shared::Packet(recivePacket, errorType, errorCode);

                    send(returningSocket, sendPacket.data, sendPacket.getPacketLength(),0);

                }
                else{
                    //Push the packet back onto the queue wait for next cycle. 
                    holdingQueue.push(queuePacket);
                    failureTracker.push(attempts);
                    sendingOrder.push(returningSocket);
                }
            }
        }
    }
    //free the interface list when we don't need it anymore
    freeifaddrs(ifaddr);
    //exit
    return 0;
}

void printForwarding(uint8_t* ipAddress, uint8_t* macAddress){
    uint32_t ipCovert;
    memcpy(&ipCovert, ipAddress, 4);
    struct in_addr address = {ipCovert};
    printf("Forwarding\n");
    printf("Forward IP: %s\n",inet_ntoa(address));
    printf("Forwarding Mac: %x:%x:%x:%x:%x:%x\n", macAddress[0],macAddress[1], macAddress[2], 
            macAddress[3], macAddress[4], macAddress[5]);
}
