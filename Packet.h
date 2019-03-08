#ifndef                     PACKET_H
#define                     PACKET_H

#include "TableConstructs.h"

#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <exception>
#include <string>

namespace shared{

    typedef union HeaderDetail{
        struct ether_arp arp;
        struct icmphdr icmp;
    } HeaderDetail;
    
    enum PacketType{
        ARP_REQUEST,
        ARP_RESPONSE,
        ICMP_REQUEST, 
        ICMP_RESPONSE,
        OTHER
    };

    class Packet{
    public:
        //Won't take a constexpr below?
        uint8_t data[1500];

        //Standard Constructor to parse a packet
        Packet(uint8_t* data);

        //Default constructor
        Packet();

        //Construct an ARP response header
        Packet(uint8_t* senderIP, uint8_t* senderMAC, uint8_t* targetIP, uint8_t* targetMAC, Packet& request);

        //Construct an ICMP response header 
        Packet(struct ether_header& etherResponse, struct iphdr& ipResponse, struct icmphdr& icmpResponse, uint8_t* icmpData, uint8_t size);

        //Construct an ARP request header
        Packet(uint8_t* senderIP, uint8_t* senderMAC, uint8_t* targetIP);

        //Construct an error ICMP packet
        Packet(struct ether_header& etherError, struct iphdr& ipError, uint8_t errorType, 
                uint8_t errorCode, uint8_t* errorData);


        Packet constructResponseARP(struct ifaddrs* interfaceList);
        Packet constructResponseICMP();
        void updateEthernetHeader(struct ForwardingData& forwardData);
        void generateForwardData(struct ForwardingData& result);
        Packet& operator=(Packet other);

        void printARPData();

        PacketType getType() const;

        uint8_t* getIPAddress() const;

        unsigned short getPacketLength() const;

        //Verifies a recived packets checksum
        bool validIpChecksum();

        //Recalculates the checksum
        uint16_t calculateIpChecksum();

    private:
        HeaderDetail detail;
        struct ether_header ethernetHeader;
        struct iphdr ipHeader;
        PacketType packetType;

        void transferMAC(uint8_t* responseMAC, uint8_t* requestMAC);
        void transferIP(uint8_t* responseIP, uint8_t* requestIP);

        struct iphdr constructIPResponseHdr();

        bool equalIPs(uint8_t* rhs, uint8_t* lhs);
        static constexpr unsigned long ARP_CODE = 0x0806;
        static constexpr unsigned long IP_CODE = 0x0800;
        static constexpr unsigned char ETHER_LEN = 14;
        static constexpr unsigned char IP_LEN = 20;
        static constexpr unsigned char ARP_LEN = 28;
        static constexpr unsigned char ICMP_LEN = 8;
        static constexpr unsigned char ARP_TOTAL_LEN = 42;

        static constexpr struct arphdr arpSuplement = {256, 8, 6, 4, 256};

    };



    class BadARP: public std::exception{
    public:
        BadARP(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "");
    private:
        const char* file;
        int line;
        const char* func;
        const char* info;
    };
}

#endif

