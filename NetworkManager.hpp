#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include "Socket.hpp"
#include "Message.hpp"
#include "EnumMessageType.hpp"

#include <vector>
#include <deque>
#include <array>
#include <optional>
#include <sys/statvfs.h>//ver espaco livre                                      
#include <sys/stat.h>//ver tamanho de arquivo 

namespace cat {

class NetworkManager {
	static constexpr unsigned char SLIDING_WINDOW_SIZE {3};
	static constexpr unsigned int TIMEOUT_INTERVAL_MS {10};

  public:
	NetworkManager(const std::string nome_interface);
	virtual ~NetworkManager();

	void sync();

	bool sendFile(std::string filename);
	void receiveFile(std::string& filepath);

	void send(EnumMessageType type, const std::vector<unsigned char> data={});
	bool sendAndWait(EnumMessageType type, const std::vector<unsigned char> data={});

	std::optional<Message> receive();
	Message receive(EnumMessageType expectedType);
	std::string logString;
	
  private:
  	//variaveis de uso geral
	Socket* socket;
	std::deque<Message> sendQueue;
	unsigned char messageSequence;
	unsigned char lastReceivedSequence;
	
	Message receiveMessage();
	long long timestamp_ms();
	bool flowControl(Message msg);
	void send(unsigned char newMessages = SLIDING_WINDOW_SIZE);
	bool emJanelaAnterior(unsigned char seq1, unsigned char seq2);
	void enqueue(EnumMessageType type, const std::vector<unsigned char> data={});
	
	//verificar erro de "sem espaço em disco"
	long long sizeMessageSize(Message m);
	long long getDiskFreeSpace(long long minFree = 10<<10);//deixa pelo menos 10 MB livre
};

}

#endif //NETWORK_MANAGER_HPP
