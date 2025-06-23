#include "Message.hpp"

#include <iostream>
#include <vector>

using namespace cat;

Message::Message(){
	this->message = {Message::START_CODE,1,1,1};//mensagem invalida qualquer
	this->message.resize(Message::MIN_BYTE_LENGTH);
}


Message::Message(const std::vector<unsigned char> message){
	this->message = message;
	if (this->message.size() < Message::MIN_BYTE_LENGTH)
		this->message.resize(Message::MIN_BYTE_LENGTH);
}

Message::Message(unsigned char sequence, EnumMessageType type,
							const std::vector<unsigned char> data) : Message(){
	//atualiza dados e tamanho
	this->setData(data);
	
	//7 bits de tamanho e 1 bit de sequencia no byte 1
	this->message[1] = (this->message[1] & ~0b1) + ((sequence & 0b10000)>>4);
	
	//4 bits de sequencia e 4 de tipo no byte 2
	this->message[2] = (sequence<<4) + type;

	//atualiza checksum
	this->message[3] = this->calculate_checksum();
}

unsigned char Message::getStartCode(){
	return this->message[0];
}

unsigned char Message::getSize(){
	return this->message[1]>>1;
}
	
unsigned char Message::getSequence(){
	return ((this->message[1] & 1) << 4) + (this->message[2] >> 4);
}

unsigned char Message::getType(){
	return this->message[2] & 0x0f;//pega os 4 bits menos significativos
}

unsigned char Message::getChecksum(){
	return this->message[3];
}

std::vector<unsigned char> Message::getData(){
	std::vector<unsigned char>::iterator start = this->message.begin()
													+ Message::DATA_BYTE_OFFSET;
	//retorna soh o conteudo de facto, armazena padding mas ignora, n retorna
	return std::vector<unsigned char> (start, start + this->getSize());
}

void Message::setData(const std::vector<unsigned char> data){
	if (data.size() > Message::DATA_BYTE_MAX_LENGTH){
		std::cerr << "data precisa ser menor q 128\n";
		return;
	}

	//apaga dados antigos, se houver
	this->message.erase(this->message.begin() + Message::DATA_BYTE_OFFSET, this->message.end());
	//concatena os dados novos
	this->message.insert(this->message.end(), data.begin(), data.end());
	
	//garante que possui o minimo necessario pra enviar a mensagem
	if (this->message.size() < Message::MIN_BYTE_LENGTH)
		this->message.resize(Message::MIN_BYTE_LENGTH);

	//atualiza campo de tamanho e checksum devido a novos dados
	this->message[1] = (data.size() << 1) + (this->message[1]%2);
	this->message[3] = this->calculate_checksum();
}

unsigned char Message::calculate_checksum(){
	//soma bytes de size sequence e type
	int soma {this->getSize() + this->getType() + this->getSequence()};

	//soma bytes de data
	for (unsigned char byte : this->getData())
		soma += byte;

	return static_cast<unsigned char>(soma%256);
}

//calcula e compara checksum (sobre size, sequence, type e data)
bool Message::verify(){
	return this->getChecksum() == this->calculate_checksum();
}

void Message::print(){
	std::cout << "\n\n\n------ MENSAGEM -------";
	std::cout << "\nStart Code: " << (int)Message::START_CODE;
	std::cout << "\nTamanho   : " << (int)this->getSize();
	std::cout << "\nSequencia : " << (int)this->getSequence();
	std::cout << "\nTipo      : " << (int)this->getType();
	std::cout << "\nChecksum  : " << (int)this->getChecksum();
	std::cout << "\nDados     : ";

	for (int c : this->getData())
		std::cout << c << " ";
	std::cout <<"\n-----------------------\n\n"<< std::endl;
}

Message::operator unsigned char*(){
	return this->message.data();
}

Message::operator std::vector<unsigned char>(){
	return this->message;
}

Message::operator std::vector<unsigned char>&(){
	return this->message;
}
