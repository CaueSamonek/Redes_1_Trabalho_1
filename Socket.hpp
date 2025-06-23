#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <linux/if_packet.h>//struct socketaddr_ll
#include <string>
#include <vector>

namespace cat {

class Socket {
  public:
    Socket(std::string nome_interface_rede);
	virtual ~Socket()=default;

    int send(std::vector<unsigned char>& dados);
    int receive(std::vector<unsigned char>& dados);
	void setTimeoutInterval(unsigned int timeoutInterval);

  private:
	int soquete;
	struct sockaddr_ll address;
};

}

#endif //SOCKET_HPP
