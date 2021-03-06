#include "Packet.h"
#include "TableConstructs.h"

#include <memory>
#include <algorithm>
#include <vector>

#include<ifaddrs.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <net/ethernet.h>

namespace shared{ 
    //Constructs a recived packet
    Packet::Packet(uint8_t* data){
        memcpy(this->data, data, 1500);

        //Construct ethernetheader
        memcpy(&ethernetHeader,this->data, ETHER_LEN);

        //printf("Packet Type: %d\n", ntohs(ethernetHeader.ether_type));

        //Check the type
        if(ntohs(ethernetHeader.ether_type) == ARP_CODE){
            //Arp stuff
            memcpy(&detail.arp, &data[ETHER_LEN], ARP_LEN);
            //packetType = ARP_REQUEST;
            //Deterimine the arp type either request or response
            if(detail.arp.ea_hdr.ar_op == htons(ARPOP_REPLY)){
                packetType = ARP_RESPONSE;
            }
            else if(detail.arp.ea_hdr.ar_op == htons(ARPOP_REQUEST)){
                packetType = ARP_REQUEST;
            }
        }
        else if(ntohs(ethernetHeader.ether_type) == IP_CODE){
            memcpy(&ipHeader, &data[ETHER_LEN], IP_LEN);
            //Record the checksum and set the value to 0 in struct.
            memcpy(&this->recordedIpChecksum, &ipHeader.check, 2);
            printf("ipHeaderCheck: %x\n", ipHeader.check);
            ipHeader.check = 0;

            printf("Recorded IP Check: %x\n", recordedIpChecksum);

            //Update the data to have the zeroed checksum;
            memcpy(&this->data[ETHER_LEN], &ipHeader, IP_LEN);

            //TODO only do icmp stuff if of type icmp
            memcpy(&detail.icmp, &data[ETHER_LEN + IP_LEN], ICMP_LEN);
            packetType = ICMP_REQUEST;
        }
        else{
            throw 1; }
    }

    //Empty Packet
    Packet::Packet(){}

    //Constructs an ARP responese
    Packet::Packet(uint8_t* senderIP, uint8_t* senderMAC, uint8_t* targetIP, 
            uint8_t* targetMAC, Packet& request){
        struct arphdr* requestEthHeader = &request.detail.arp.ea_hdr;

        detail.arp.ea_hdr.ar_hrd = requestEthHeader->ar_hrd; 
        detail.arp.ea_hdr.ar_pro = requestEthHeader->ar_pro;
        detail.arp.ea_hdr.ar_hln = requestEthHeader->ar_hln;
        detail.arp.ea_hdr.ar_pln = requestEthHeader->ar_pln;
        detail.arp.ea_hdr.ar_op = htons(ARPOP_REPLY);


        memcpy(detail.arp.arp_sha, senderMAC, 6);
        memcpy(detail.arp.arp_spa, senderIP, 4);
        memcpy(detail.arp.arp_tha, targetMAC, 6);
        memcpy(detail.arp.arp_tpa, targetIP, 4);

        packetType = ARP_RESPONSE;

        //Construct the ethernet header
        //Destination
        memcpy(ethernetHeader.ether_dhost, request.ethernetHeader.ether_shost, 6);

        //Host
        memcpy(ethernetHeader.ether_shost, senderMAC, 6);

        //Type
        ethernetHeader.ether_type = htons(0x0806);


        //Copy the data to the data array
        memcpy(this->data, &ethernetHeader, ETHER_LEN);
        memcpy(&this->data[ETHER_LEN], &detail.arp, ARP_LEN);

    }

    //ICMP response header. 
    Packet::Packet(struct ether_header& etherResponse, struct iphdr& ipResponse, 
            struct icmphdr& icmpResponse, uint8_t* icmpData, uint8_t size){
        memcpy(this->data, &etherResponse, ETHER_LEN);
        memcpy(&this->data[ETHER_LEN], &ipResponse, IP_LEN);
        memcpy(&this->data[ETHER_LEN + IP_LEN], &icmpResponse, ICMP_LEN);
        memcpy(&this->data[ETHER_LEN + IP_LEN + ICMP_LEN], 
                &icmpData[ETHER_LEN + IP_LEN + ICMP_LEN], size);

        //printf("%d\n", icmpResponse.un.echo.id);
        this->ethernetHeader = etherResponse;
        this->ipHeader = ipResponse;
        this->detail.icmp = icmpResponse;


    }

    //Equals operator
    Packet& Packet::operator=(Packet other){
        std::swap(this->detail,other.detail);
        std::swap(this->data, other.data);
        std::swap(this->ethernetHeader, other.ethernetHeader);
        std::swap(this->ipHeader, other.ipHeader);
        std::swap(this->packetType, other.packetType);
        std::swap(this->recordedIpChecksum, other.recordedIpChecksum);

        return *this;
    }

    //Arp request
    Packet::Packet(uint8_t* senderIP, uint8_t* senderMAC, uint8_t* targetIP){
        uint8_t broadcastAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        uint8_t targetAddress[6] = {0, 0, 0, 0, 0, 0};

        //Don't ask
        detail.arp.ea_hdr.ar_hrd = 256;
        detail.arp.ea_hdr.ar_pro = 8;
        detail.arp.ea_hdr.ar_hln = 6;
        detail.arp.ea_hdr.ar_pln = 4;
        detail.arp.ea_hdr.ar_op = 256;

        memcpy(detail.arp.arp_sha, senderMAC, 6);
        memcpy(detail.arp.arp_spa, senderIP, 4);
        memcpy(detail.arp.arp_tha, targetAddress, 6);
        memcpy(detail.arp.arp_tpa, targetIP, 4);

        //Form the Ethernet Header
        memcpy(ethernetHeader.ether_dhost, broadcastAddress,6);
        memcpy(ethernetHeader.ether_shost, senderMAC, 6);
        ethernetHeader.ether_type = htons(ARP_CODE);

        memcpy(data, &ethernetHeader, ETHER_LEN);
        memcpy(&data[ETHER_LEN], &detail.arp, ARP_LEN);
    }

    //ICMP error packet.
    //Error data is the raw data from the error packet in question
    Packet::Packet(Packet& other, uint8_t errorType, uint8_t errorCode){

        //Create the new ethernet header
        memcpy(&ethernetHeader.ether_dhost, &other.ethernetHeader.ether_shost, 6);
        memcpy(&ethernetHeader.ether_shost, &other.ethernetHeader.ether_dhost, 6);
        memcpy(&ethernetHeader.ether_type, &other.ethernetHeader.ether_type, sizeof(uint16_t));

        //Copy ethernet header to data
        memcpy(data, &ethernetHeader, ETHER_LEN);


        //Create a new ip header
        memcpy(&ipHeader, &other.ipHeader, IP_LEN);

        //Change the source, destination, reset ttl and checksum.
        memcpy(&ipHeader.saddr, &other.ipHeader.daddr, 4);
        memcpy(&ipHeader.daddr, &other.ipHeader.saddr, 4);
        ipHeader.ttl = 64;
        ipHeader.check = 0;

        //Copy the new header in and recalculate checksum. 
        memcpy(&data[ETHER_LEN], &ipHeader, IP_LEN);
        calculateIpChecksum();

        detail.icmp.type = errorType;
        detail.icmp.code = errorCode;

        //Ensure default values
        //TODO do I need this? Isn't it already set to default values. 
        detail.icmp.un.echo.id = 0;
        detail.icmp.un.echo.sequence = 0;

        //Copy the header to the data
        memcpy(&data[ETHER_LEN + IP_LEN], &detail.icmp, ICMP_LEN);

        //Add the ip header + 8 bytes of the bad packet into the data section
        memcpy(&this->data[ETHER_LEN + IP_LEN + ICMP_LEN], &other.data[ETHER_LEN], IP_LEN + 8);

        calculateIcmpChecksum();
    }

    Packet Packet::constructResponseARP(struct ifaddrs* interfaceList){
        //Remember that IPs only have 4 elements while MAC addresses have 6
        //The target IP we have and the MAC we are looking for.
        //TODO maybe typedef this to make it easier to read.
        uint32_t targetIP;
        memcpy(&targetIP, detail.arp.arp_tpa, 4);

        uint8_t* targetMAC;

        uint8_t* senderIP = detail.arp.arp_spa;
        uint8_t* senderMAC = detail.arp.arp_sha;

        char* targetInterface;
        struct ifaddrs* temp;
        struct ifaddrs* temp2;

        //Find the interface for the matching ip. 
        for(temp = interfaceList; temp != NULL; temp = temp->ifa_next){
            if(temp->ifa_addr->sa_family == AF_INET){
                //Cant be cast as a static cast
                struct sockaddr_in* foundIP = (struct sockaddr_in*) (temp->ifa_addr);
                //We found the matching ip get the interface name.
                if(foundIP->sin_addr.s_addr == targetIP){
                    targetInterface = temp->ifa_name;

                    //Find the matching MAC address 
                    for(temp2 = interfaceList; temp2 != NULL; temp2 = temp2->ifa_next){
                        //We found the MAC address of the interface we need.
                        if(strcmp(temp2->ifa_name, targetInterface) == 0 && 
                                temp2->ifa_addr->sa_family == AF_PACKET){
                            struct sockaddr_ll* targetMatch = 
                                (struct sockaddr_ll*)(temp2->ifa_addr);
                            targetMAC = targetMatch->sll_addr;

                            Packet response(detail.arp.arp_tpa, targetMAC, senderIP, senderMAC, 
                                    *this);

                            //Construct the response packet
                            //Target becomes the sender and the sender becomes the target.
                            return response;
                        }
                    }
                    break;
                }
            }
        }
        throw 2;
    }

    Packet Packet::constructResponseICMP(){
        //Create the response ethernet header
        struct ether_header responseEther;

        memcpy(&responseEther.ether_dhost, &ethernetHeader.ether_shost, 
                sizeof (ethernetHeader.ether_shost));
        memcpy(&responseEther.ether_shost, &ethernetHeader.ether_dhost, 
                sizeof(ethernetHeader.ether_dhost));

        memcpy(&responseEther.ether_type, &ethernetHeader.ether_type, 
                sizeof(ethernetHeader.ether_type));

        //Construct the response IP 
        struct iphdr responseIP = constructIPResponseHdr();

        struct icmphdr responseICMP;
        responseICMP.type = 0;
        responseICMP.code = 0;
        //Change this later.
        responseICMP.checksum = detail.icmp.checksum;
        responseICMP.un.echo.id = detail.icmp.un.echo.id;
        responseICMP.un.echo.sequence = detail.icmp.un.echo.sequence;

        //Find the size of the data
        int dataSize = ntohs(ipHeader.tot_len) - (IP_LEN + ICMP_LEN);

        Packet reply(responseEther, responseIP, responseICMP, this->data, dataSize);

        return reply;
    }

    void Packet::updateEthernetHeader(struct ForwardingData& forwardingData){
        struct ether_header newHeader;

        newHeader.ether_type = this->ethernetHeader.ether_type;
        memcpy(newHeader.ether_shost, forwardingData.sourceMacAddress, 6);
        memcpy(newHeader.ether_dhost, forwardingData.destinationMacAddress, 6);

        //Copy the data into the data array
        memcpy(this->data, &newHeader, ETHER_LEN);

    }

    //This doesn't take into account the interface name and the socket used.
    void Packet::generateForwardData(struct ForwardingData& result){ 


        memcpy(&result.destinationMacAddress, &detail.arp.arp_sha, 6);
        memcpy(&result.sourceMacAddress, &detail.arp.arp_tha, 6);
        memcpy(&result.ipAddress, &detail.arp.arp_spa, 4);
    }

    void Packet::printARPData() {
        //Print the MAC Address? 
        struct ether_addr temp;
        for(int i = 0; i < 6; ++i){
            temp.ether_addr_octet[i] = detail.arp.arp_sha[i];
        }
        char* mac_str = ether_ntoa(&temp);
        printf("%s\n", mac_str);
        printf("\n");
    }

    //DEPRICATED
    bool Packet::equalIPs(uint8_t* rhs, uint8_t* lhs){
        for(int octet = 0; octet < 4; ++octet){
            if(rhs[octet] != lhs[octet]){
                return false;
            }
        }
        return true;
    }

    void Packet::transferMAC(uint8_t* responseMAC, uint8_t* requestMAC){
        for(int octet = 0; octet < 6; ++octet){
            responseMAC[octet] = requestMAC[octet];
        }
    }

    void Packet::transferIP(uint8_t* responseIP, uint8_t* requestIP){
        for(int octet = 0; octet < 4; ++octet){
            responseIP[octet] = requestIP[octet];
        }
    }

    struct iphdr Packet::constructIPResponseHdr(){
        //Construct the response IP 
        struct iphdr responseIP;
        responseIP.ihl = 5; 
        responseIP.version = 4;
        responseIP.tos = ipHeader.tos;
        responseIP.tot_len = ipHeader.tot_len;
        responseIP.id = ipHeader.id;
        responseIP.frag_off = ipHeader.frag_off;
        responseIP.ttl = 64;
        responseIP.protocol = ipHeader.protocol;
        responseIP.check = ipHeader.check;
        responseIP.saddr = ipHeader.daddr;
        responseIP.daddr = ipHeader.saddr;

        return responseIP;
    }

    PacketType Packet::getType() const{
        return this->packetType;
    }

    uint8_t* Packet::getIPAddress() const{
        uint8_t* returnIP = new uint8_t[4];
        memcpy(returnIP, &ipHeader.daddr, 4);
        return returnIP;
    }

    unsigned short Packet::getPacketLength() const {
        return ETHER_LEN + htons(ipHeader.tot_len);
    }

    bool Packet::validIpChecksum(){
        constexpr int HEADER_SECTIONS = 10;

        //The start of the ip header
        uint16_t* buffer = (uint16_t*)&this->data[ETHER_LEN];

        register unsigned long sum = 0;

        int count = HEADER_SECTIONS;

        while(count--){
            sum += *buffer++;

            if(sum & 0xFFFF0000){
                /* carry occurred, so wrap around */
                sum &= 0xFFFF;
                sum++;
            }
        }

        uint16_t result = ~(sum & 0xFFFF);
        printf("IP Checksum: %x\n", result); 
        printf("Recorded Checksum: %x\n", this->recordedIpChecksum);

        //TODO do the actual check
        return result == this->recordedIpChecksum;
    }

    //WARNING This assumes all packet modifications have already been done.
    void Packet::calculateIpChecksum(){
        constexpr int HEADER_SECTIONS = 10;

        //The start of the ip header
        //
        //TODO change to u_short
        uint16_t*  buffer = (uint16_t*)&data[ETHER_LEN];

        register unsigned long sum = 0;

        int count = HEADER_SECTIONS;

        while(count--){
            sum += *buffer++;

            if(sum & 0xFFFF0000){
                /* carry occurred, so wrap around */
                sum &= 0xFFFF;
                sum++;
            }
        }

        ipHeader.check = ~(sum & 0xFFFF);
        memcpy(&data[ETHER_LEN], &ipHeader, IP_LEN);
        //printf("IP Checksum: %x\n", ipHeader.check);
    }

    void Packet::decTTL(){
        //Subtract the ttl and update the data.
        ipHeader.ttl -= 1;
        memcpy(&this->data[ETHER_LEN], &ipHeader, IP_LEN);
    }

    bool Packet::zeroedTTL() const{
        return ipHeader.ttl < 1;
    }

    void Packet::calculateIcmpChecksum(){
        //Calculate the total length needed 
        int icmpLen = ipHeader.tot_len - IP_LEN;

        //The count is the length of the icmp data and header * the number of bits in a byte
        //divided by 16 bits to do the checksum.
        int count = (ICMP_LEN + 28) * 8 / 16;

        //Start at the beginning of the icmp header and go to the end.
        //TODO change to unsigned short
        uint16_t* buffer = (uint16_t*)&data[ETHER_LEN + IP_LEN];

        register unsigned long sum = 0;

        while(count--){
            sum += *buffer++;

            if(sum & 0xFFFF0000){
                /* carry occurred, so wrap around */
                sum &= 0xFFFF;
                sum++;
            }
        }

        detail.icmp.checksum = ~(sum & 0xFFFF);
        memcpy(&data[ETHER_LEN + IP_LEN], &detail.icmp, ICMP_LEN);
    }
}

