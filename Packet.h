#ifndef                     PACKET_H
#define                     PACKET_H

#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>

namespace shared{

    typedef union HeaderDetail{
        struct ether_arp arp;
        struct icmphdr icmp;
    } HeaderDetail;

    class Packet{
    public:
        char data[5000];
        Packet(char* data);
        //Construct an ARP request header
        //Packet(...);

        //Construct an ICMP Request header
        //Packet(...);

        Packet constructResponseARP(Packet& request);
        Packet constructResponseICMP(Packet& request);

        ~Packet();

    private:
        HeaderDetail detail;
        bool arp;
    };
}

#endif

