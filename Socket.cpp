#include "Socket.hpp"

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>

#include <iostream>
#include <vector>
 
using namespace cat;

Socket::Socket(std::string nome_interface_rede){
    this->soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	int ifindex = if_nametoindex(nome_interface_rede.c_str());	

	this->address={};
	this->address.sll_ifindex = ifindex;
	this->address.sll_family = AF_PACKET;
    this->address.sll_protocol = htons(ETH_P_ALL);

    if (this->soquete == -1){
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }

    if (bind(this->soquete, reinterpret_cast<struct sockaddr*>(&(this->address)), sizeof(this->address)) == -1){
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }
 
 	//modo promiscuo
    struct packet_mreq mr={};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(this->soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1){
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }
	this->setTimeoutInterval(500);//default 500 ms sobrescrito por NetworkManager
}

void Socket::setTimeoutInterval(unsigned int timeoutInterval){
	//recv timeout
	const unsigned int timeoutMillis {timeoutInterval};
	struct timeval timeout;
	timeout.tv_sec = timeoutMillis / 1000;
	timeout.tv_usec = (timeoutMillis % 1000) * 1000;
	setsockopt(this->soquete, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout),
																		sizeof(timeout));
}

int Socket::send(std::vector<unsigned char>& dados){
	return sendto(this->soquete, dados.data(), dados.size(), 0, reinterpret_cast<struct sockaddr*>(&this->address), sizeof(this->address));
}

int Socket::receive(std::vector<unsigned char>& buffer){
    return recv(this->soquete, buffer.data(), buffer.size(), 0);
}

