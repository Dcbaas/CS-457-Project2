#include "Packet.h"

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


typedef int SocketFD;
int main(){
    //Define 2 Packet objects one for reciving, the other for sending.
    shared::Packet sendPacket;
    shared::Packet recivePacket;
    std::vector<SocketFD> sockets;
    fd_set socketSetMaster;


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
        if(tmp->ifa_addr->sa_family==AF_PACKET){
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
            FD_SET(sockets.back(), &socketSetMaster);
        }
    }
    //loop and recieve packets. We are only looking at one interface,
    //for the project you will probably want to look at more (to do so,
    //a good way is to have one socket per interface and use select to
    //see which ones have data)
    printf("Ready to recieve now\n");
    while(1){
        int packet_socket = sockets.front();
        char buf[1500];
        struct sockaddr_ll recvaddr;
        unsigned int recvaddrlen=sizeof(struct sockaddr_ll);
        try{
            //we can use recv, since the addresses are in the packet, but we
            //use recvfrom because it gives us an easy way to determine if
            //this packet is incoming or outgoing (when using ETH_P_ALL, we
            //see packets in both directions. Only outgoing can be seen when
            //using a packet socket with some specific protocol)
            int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
            //ignore outgoing packets (we can't disable some from being sent
            //by the OS automatically, for example ICMP port unreachable
            //messages, so we will just ignore them here)
            if(recvaddr.sll_pkttype==PACKET_OUTGOING)
                continue;
            //start processing all others
            printf("Got a %d byte packet\n", n);

            recivePacket = shared::Packet(buf);
            if(recivePacket.isARP()){
                printf("Got an ARP packet\n");
                //            sendPacket.printARPData();
                sendPacket = recivePacket.constructResponseARP(ifaddr);
                send(packet_socket, sendPacket.data, 42, 0);
            }
            else if(recivePacket.isICMP()){
                printf("Got an ICMP packet\n");
                sendPacket = recivePacket.constructResponseICMP();
                send(packet_socket, sendPacket.data, 98, 0);
            }
        }
        catch(int e){
            if(e == 1){

                printf("Bad packet was recived and ignored\n");
            }
            else if(e == 2){
                printf("Interface was not found for response\n");
            }
        }

        //what else to do is up to you, you can send packets with send,
        //just like we used for TCP sockets (or you can use sendto, but it
        //is not necessary, since the headers, including all addresses,
        //need to be in the buffer you are sending)

    }
    //free the interface list when we don't need it anymore
    freeifaddrs(ifaddr);
    //exit
    return 0;
}
