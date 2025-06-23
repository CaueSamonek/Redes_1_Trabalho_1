#include "NetworkManager.hpp"
#include "EnumMessageType.hpp"
#include "GameConfig.hpp"
#include "Message.hpp"

#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sys/stat.h>

using namespace cat;

NetworkManager::NetworkManager(const std::string nome_interface){
	this->logString="";
	this->messageSequence=0;
	this->lastReceivedSequence=-1;
	this->socket = new Socket(nome_interface);
	this->socket->setTimeoutInterval(NetworkManager::TIMEOUT_INTERVAL_MS);
}

NetworkManager::~NetworkManager(){
	delete this->socket;
}

//achar nome mior
long long NetworkManager::sizeMessageSize(Message m){
	if (m.getType() != EnumMessageType::SIZE)
		return 0;

	long long file_size = 0;
	std::memcpy(&file_size, m.getData().data(), sizeof(file_size));

	return file_size;
}

bool NetworkManager::emJanelaAnterior(unsigned char seq1, unsigned char seq2) {
    // distância circular de seq1 e seq2
    unsigned char dist = (seq2 - seq1) & 0b011111;

    // retorna true se seq1 vem antes de seq2 e está dentro da janela
    return dist <= NetworkManager::SLIDING_WINDOW_SIZE;
}


bool NetworkManager::sendFile(std::string filename){
	std::ifstream file(filename, std::ios::binary);//ler bytes do arquivo
	if (!file.is_open()){
		std::cout << "Erro Ao Abrir O Arquivo" << std::endl;
		return false;
	}

	EnumMessageType type;
	std::string extension = filename.substr(filename.rfind(".")+1);

	if (extension == "txt")      type = EnumMessageType::TEXT;
	else if (extension == "jpg") type = EnumMessageType::IMAGE;
	else if (extension == "mp4") type = EnumMessageType::VIDEO;
	else {
		std::cout << "Formato Não Aceito, Abortando Operação" << std::endl;
		return false;
	}
	//pega tamanho do arquivo e salva na area de dados da mensagem
	struct stat st;
	if (stat(filename.c_str(), &st)){
		std::cerr << "Erro Ao Fazer stat() Em " << filename << std::endl;
		return false;
	}
	long long file_size = st.st_size;
	std::vector<unsigned char> file_size_vec(sizeof(file_size));
	std::memcpy(file_size_vec.data(), &file_size, sizeof(file_size));

	//envia tipo do arquivo com o nome, dps o tamanho, se nao der erro, envia os dados
	std::string name = filename.substr(filename.find("/")+1);
	this->enqueue(type, {name.begin(), name.end()});//filename
	bool deu = this->sendAndWait(EnumMessageType::SIZE, file_size_vec);
	if (!deu){
		this->sendQueue.clear();
		return false;
	}

	//carrega bytes do arquivo no vetor
	std::vector<unsigned char> data {std::istreambuf_iterator<char>(file),
										    std::istreambuf_iterator<char>()};
	unsigned long extra {0};//calcula quanto vai precisar aumentar
	for (unsigned char byte : data)
		if (byte == 129 || byte == 136) extra++;

	long long original {static_cast<long long>(data.size())};
	data.resize(original+extra);

	//adiciona um 255 depois de cada 129 ou 136 para evitar perdas
	for (long long i{original-1}, j{static_cast<long long>(data.size())-1}; i>=0; --i){
		if (data[i] == 129 || data[i] == 136)
			data[j--] = 255;
		data[j--] = data[i];
	}

	this->enqueue(EnumMessageType::DATA, data);
	this->sendAndWait(EnumMessageType::END_OF_FILE);//envia e espera terminar tudo
	this->logString="";//sem erro ao enviar
	return  true;
}

//picota e adiciona as partes na lista de mensagens a serem enviadas
void NetworkManager::enqueue(EnumMessageType type, const std::vector<unsigned char> data){
	unsigned long long i{0};
	unsigned long long len {data.size()};
	unsigned char max {Message::DATA_BYTE_MAX_LENGTH};
	unsigned long long data_full_msg = len - len % max;

	for (; i < data_full_msg; i+=max){
		std::vector<unsigned char> v (data.begin()+i, data.begin()+i+max);
		this->sendQueue.push_back(Message (this->messageSequence++, type, v));
	}

	if(len%max){
		std::vector<unsigned char> v (data.begin()+i, data.begin()+i+len%max);
		this->sendQueue.push_back(Message (this->messageSequence++, type, v));
	}

	if (!len)
		this->sendQueue.push_back(Message (this->messageSequence++, type));
}

//adiciona na fila e envia a janela
void NetworkManager::send(EnumMessageType type, std::vector<unsigned char> data){
	this->enqueue(type, data);
	this->send();
}

//envia mensagens e espera confirmacao da ultima
bool NetworkManager::sendAndWait(EnumMessageType type, std::vector<unsigned char> data){
	this->enqueue(type, data);

	//seq da ultima mensagem, a mensagem desejada que faz essa funcao terminar
	unsigned char seq {this->sendQueue.back().getSequence()};
	this->send();//envia mensagens na janela

	//sai do loop quando recebe ack de 'seq'
	std::optional<Message> mOpt;
	Message m;
	
	int countErros {0};//NACKs ou TOs

	//timeout referente a janela, se mover janela timer reinicia
	long long start = this->timestamp_ms();
	while (true){
		if (countErros >= 10){
			this->sync();
			return false;
		}

		if (this->timestamp_ms() - start >= NetworkManager::TIMEOUT_INTERVAL_MS){
#ifdef DEBUG
			std::cout << "Contador de Erros de Envio" << countErros << std::endl;
#endif
			countErros++;	
			this->send();
			start = this->timestamp_ms();//reseta timer
		}

		mOpt = this->receive();
		if (mOpt.has_value())
			m = mOpt.value();
		else continue;

		if (m.getType() == EnumMessageType::ACK || m.getType() == EnumMessageType::NACK){
			//se for a msg esperada, verifica tamanho pra evitar msgs de msm sequencia
			if (m.getData()[0] == seq && this->sendQueue.size() <= NetworkManager::SLIDING_WINDOW_SIZE)
				return true;	

			if (m.getType() == EnumMessageType::NACK)
				countErros++;
			start = this->timestamp_ms();//reseta timer pois recebeu resposta
		} else if (m.getType() == EnumMessageType::ERROR){
			this->logString = "Erro Ao Enviar Arquivo: ";
			switch (m.getData()[0]){
				case 0:
					this->logString += "Erro De Permissão"; break;
				case 1:
					this->logString += "Erro De Armazenamento"; break;
				default:
					this->logString += "Erro Desconhecido";
			}
			return false;
		}else countErros++;
	}
}

void NetworkManager::send(unsigned char newMessages){
	//quantidade de mensagens a serem enviadas, tamanho da janela ou menos
	unsigned int max = NetworkManager::SLIDING_WINDOW_SIZE;
	if (max > this->sendQueue.size()){
		max = this->sendQueue.size();
	}

	if (newMessages > max)
		newMessages = max;

	//envia somente as novas mensagens que entraram na janela
	for (unsigned int i{max-newMessages}; i < max; ++i)
		this->socket->send(this->sendQueue[i]);
}

long long NetworkManager::timestamp_ms(){
	std::chrono::duration time = std::chrono::steady_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
}

void NetworkManager::receiveFile(std::string& filepath){
	std::ofstream file (filepath, std::ios::binary); 

	std::optional<Message> mOpt;
	std::vector<unsigned char> data;
	Message m;
	do {
		mOpt = this->receive();
		if (mOpt.has_value())
			m = mOpt.value();
		else continue;

		if (m.getType() == EnumMessageType::DATA){
			std::vector<unsigned char> v = m.getData();
			data.insert(data.end(), v.begin(), v.end());
		} else if (m.getType() == HANDSHAKE)
			return;

	} while(m.getType() != EnumMessageType::END_OF_FILE);
	
	//remove '255' extras, adicionados pra evitar perda
	unsigned long idx {0};
	for (unsigned long i{0}; i < data.size(); ++i){
		data[idx++] = data[i];
		if ((data[i] == 129 || data[i] == 136) && i+1 < data.size() &&  data[i+1] == 255)
			++i;
	}
	data.resize(idx);

	file.write(reinterpret_cast<const char*>(data.data()), data.size());
#ifdef DEBUG
	std::cout << "receiveFile(): Recebeu Arquivo: "<< filepath <<std::endl;
#endif
	this->logString="";//recebeu sem erros
}

void NetworkManager::sync(){
	this->sendQueue.clear();
	long long start = timestamp_ms();
	while(1){
		if (timestamp_ms() - start > NetworkManager::TIMEOUT_INTERVAL_MS){
			this->socket->send(Message(0, EnumMessageType::HANDSHAKE,{
				(unsigned char)((this->lastReceivedSequence +1)%32),
				(unsigned char)((this->messageSequence -1)%32)
			}));//envia infos de handshake
			start = timestamp_ms();
		}
		std::optional<Message> mOpt = this->receive();
		if (!mOpt.has_value())
			continue;
		Message m = mOpt.value();
		if (m.getType() == EnumMessageType::ACK || m.getType() == EnumMessageType::HANDSHAKE)
			return;
	}
}

long long NetworkManager::getDiskFreeSpace(long long minFree){
    struct statvfs stat;

    if (statvfs("/", &stat) != 0){
        std::cout << "Erro Ao Fazer statvfs()" << std::endl;
        return 0;
    }

    //bytes livres com uma folga
    long long freeSpace = stat.f_bsize * stat.f_bavail - minFree;
    return freeSpace >= 0 ? freeSpace : 0;
}


bool NetworkManager::flowControl(Message msg){
	//se a mensagem chegou com erro, envia nack
	if (!msg.verify()){

		this->socket->send(Message (0, EnumMessageType::NACK,
												{this->lastReceivedSequence}));
#ifdef DEBUG
		std::cout << "Mensagem Recebida Com Erro" << std::endl;
#endif
		return false;
	}

	if (msg.getType() == EnumMessageType::HANDSHAKE){
		this->sendQueue.clear();
		this->messageSequence = msg.getData()[0];
		this->lastReceivedSequence = msg.getData()[1];
		this->socket->send(Message(0,EnumMessageType::ACK, {this->lastReceivedSequence}));
		return true;
	}

	if (msg.getType() == EnumMessageType::TEXT ||
										msg.getType() == EnumMessageType::IMAGE ||
										msg.getType() == EnumMessageType::VIDEO){
		this->lastReceivedSequence = msg.getSequence();

		std::string base {GameConfig::DESTINY_DIR_PATH};
		std::vector<unsigned char> v {msg.getData()};
		std::string filename {v.begin(), v.end()};
		std::string path = base + filename;

		std::ofstream file (path, std::ios::binary);//tenta abrir arquivo
#ifdef DEBUG
		if (!(std::rand()%2) || !file){
			errno = std::rand()%2 ? errno : EACCES;
#else
		if (!file){
#endif
			unsigned char errorType = 2;//soh existem erros 0 e 1
			this->logString = "Erro Ao Enviar Arquivo: Erro Desconhecido";
			if (errno == EACCES){
				errorType = 0;
				this->logString = "Erro Ao Enviar Arquivo: Erro De Permissão";
			}
			this->sendAndWait(EnumMessageType::ERROR, {errorType});
			return false;
		}
		
		//da forma q sendFile foi implementado, dps do tipo sempre vem o tamanho
		//espera receber o tamanho e verifica se tem espaco
		Message t;
		do
			t = this->receiveMessage();
		while (t.getStartCode() != Message::START_CODE &&
										t.getType() != EnumMessageType::SIZE);
		this->lastReceivedSequence = t.getSequence();
		
		long long spaceDisk {this->getDiskFreeSpace()};
		long long spaceFile {this->sizeMessageSize(t)};
#ifdef DEBUG
		if (!(std::rand()%2) || spaceDisk - spaceFile < 0){
#else
		if (spaceDisk - spaceFile < 0){
#endif
			this->logString = "Erro Ao Enviar Arquivo: Erro De Armazenamento";
			this->sendAndWait(EnumMessageType::ERROR, {1});
			return false;
		}
		//sem erros para recebimento de arquivo
		this->logString="";
		this->socket->send(Message(0,EnumMessageType::ACK,{this->lastReceivedSequence}));
		return true;
	}

	//recebeu ack com a sequencia da mensagem aceita
	//manda somente as novas mensagens que entraram na janela
	if (msg.getType() == EnumMessageType::ACK || msg.getType() == EnumMessageType::NACK){
		unsigned char ackedMessages {0};
		unsigned char ackedSeq {msg.getData()[0]};

		while (!this->sendQueue.empty() &&
		this->emJanelaAnterior(this->sendQueue.front().getSequence(), ackedSeq)){
			++ackedMessages;
			this->sendQueue.pop_front();
		}	

		if (msg.getType() == EnumMessageType::ACK)
			this->send(ackedMessages);//manda soh as novas msgs da janela
		else //nack
			this->send(NetworkManager::SLIDING_WINDOW_SIZE);//reenvia todas as mensagens da janela	
		return true;
	}

	//se pulou alguma mensagem, envia nack
	unsigned char msgSeq = msg.getSequence();
	unsigned char expectedSeq {static_cast<unsigned char>((this->lastReceivedSequence+1) & 0b011111)};

	if (msgSeq != expectedSeq){
		if (this->emJanelaAnterior(msg.getSequence(), expectedSeq)){

			this->socket->send(Message(0,EnumMessageType::ACK,{this->lastReceivedSequence}));
		}
		else
			this->socket->send(Message(0,EnumMessageType::NACK,{this->lastReceivedSequence}));
#ifdef DEBUG
		std::cout << "Sequencia Recebida Diferente do Esperado" << std::endl;
#endif
		return false;
	}

	//manda ack da msg aceita e atualiza ultima sequencia aceita
	this->lastReceivedSequence = msg.getSequence();
	this->socket->send(Message(0,EnumMessageType::ACK,{this->lastReceivedSequence}));
	return true;
}


Message NetworkManager::receive(EnumMessageType type){
	while (true){
		std::optional<cat::Message> mOpt = this->receive();
		if (!mOpt.has_value()) continue;
		cat::Message m = mOpt.value();
	
		if (m.getType() == type)
			return m;
	}
}
//le uma mensagem valida ou retorna mensagem vazia
std::optional<Message> NetworkManager::receive(){
	Message msg = this->receiveMessage();

	if (msg.getStartCode() != Message::START_CODE)
		return std::nullopt;

	if(!this->flowControl(msg))//trata de acks, nacks e movimento da janela
		return std::nullopt;

	return msg;
}

//recebe uma mensagem sem fazer o controle de fluxo, somente uso interno
Message NetworkManager::receiveMessage(){
	std::vector<unsigned char> v;

	v.resize(Message::MAX_BYTE_LENGTH);//coloca buffer no maximo

	int readSize {this->socket->receive(v)};
	readSize = readSize <= 0 ? 0 : readSize;

	v.resize(readSize);//reduz pro tamanho utilizado
	Message msg (v);//constroi mensagem	
	return msg;
}
