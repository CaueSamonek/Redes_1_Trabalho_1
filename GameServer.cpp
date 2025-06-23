#include "GameServer.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;
using namespace cat;

GameServer::GameServer(std::string interface_name, Position gridMax) : gridMax{gridMax}{
	this->manager = new NetworkManager(interface_name);

    std::string dirPath = "objetos/";
    std::vector<std::string> filenames;

    if (fs::exists(dirPath) && fs::is_directory(dirPath)){
		for (const fs::directory_entry& entry : fs::directory_iterator(dirPath))
        	if (fs::is_regular_file(entry))
            	filenames.push_back(dirPath + entry.path().filename().string());
    }
	std::sort(filenames.begin(),filenames.end());	
    std::srand(static_cast<unsigned int>(std::time(0)));
    this->createTreasures(filenames);
	//diretorio destino de arquivos transferidos
	fs::create_directories(GameConfig::DESTINY_DIR_PATH);
}

GameServer::~GameServer(){
	delete this->manager;
}

void GameServer::printGameState(){
	//"flush"
	std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	std::cout << "==================== GAME STATE ====================\n\n";
	std::cout << "--------------------   Player   --------------------\n";
	
	if (this->playerMoveLog.size()>=2){
		long long i {0};
		for (; i < static_cast<long long>(this->playerMoveLog.size()-2); ++i)
			std::cout << this->playerMoveLog[i] << " -> ";
		std::cout << this->playerMoveLog[this->playerMoveLog.size()-2]<<"\n\n";
	}

	std::cout << "\nPosição Atual: " << this->playerMoveLog[this->playerMoveLog.size()-1] << "\n\n";
	
	std::cout << "--------------------  Tesouros  --------------------\n\n";
	for (Treasure& t : this->treasures) std::cout << t;
	
	if (this->manager->logString != ""){
		std::cout << "\n-------------------- Last Error --------------------\n\n";
		int len = 25-(this->manager->logString.size())/2;
		std::cout << std::string(len,' ') << this->manager->logString;
	}
	std::cout << "\n\n====================================================" << std::endl;
}


void GameServer::run(){
  	this->manager->sync();

	while (true){
		std::optional<Message> mOpt = this->manager->receive();
		if (!mOpt.has_value()) continue;
		Message m = mOpt.value();

		switch (static_cast<EnumMessageType>(m.getType())){
			case cat::INIT:
				this->playerRotation = 0;
				this->resetTreasures();//aleatoriza posicoes e seta found = false
				this->playerPosition = this->initPosition();
				this->playerMoveLog.clear();
				this->manager->logString="";
				break;
			case cat::MOVE_UP:
				this->playerPosition.y = this->playerPosition.y > 0 ?
										this->playerPosition.y-1 : 0;
				this->playerRotation = 0;
				break;
			case cat::MOVE_RIGHT:
				this->playerPosition.x = this->playerPosition.x < this->gridMax.x-1 ?
										this->playerPosition.x+1 : this->gridMax.x-1;
				this->playerRotation = 1;
				break;
			case cat::MOVE_DOWN:
				this->playerPosition.y = this->playerPosition.y < this->gridMax.y-1 ?
										this->playerPosition.y+1 : this->gridMax.y-1;
				this->playerRotation = 2;
				break;
			case cat::MOVE_LEFT:
				this->playerPosition.x = this->playerPosition.x > 0 ?
										this->playerPosition.x-1 : 0;
				this->playerRotation = 3;
				break;
			default: continue;
		}

		if(!this->manager->sendAndWait(EnumMessageType::OK, {
								static_cast<unsigned char>(playerPosition.x),
								static_cast<unsigned char>(playerPosition.y),
								static_cast<unsigned char>(playerRotation)}))
				continue;
		this->playerMoveLog.push_back(this->playerPosition);
		for (Treasure& t : this->treasures)
			if (!t.found && playerPosition == t.position){
				if (!this->manager->sendFile(t.filename)){

					break;
				}
				t.found = true;
				std::cout << "Arquivo " << t.filename << " Enviado" << std::endl;
			}
		
		this->printGameState();
	}
}

void GameServer::createTreasures(std::vector<std::string> filenames){
	for (std::string& filename : filenames){
        Treasure treasure(filename, this->initPosition());
    	this->treasures.push_back(treasure);
	}
}

void GameServer::resetTreasures(){
	//reseta tudo 'liberando posicoes' para depois randomiza-las
	for (Treasure& t : this->treasures){
		t.found = false;
		t.position = Position(-1,-1);
	}

	for (Treasure& t : this->treasures)
		t.position = initPosition();
}

bool GameServer::hasTreasure(Position& position){
	for (Treasure& t : this->treasures)
	    if (t.position == position)
   	    	return true;
    return false;
}

//retorna uma posicao sem tesouro
Position GameServer::initPosition(){
    Position p;
	do
		p = Position::rand(this->gridMax.x, this->gridMax.y);
	while (this->hasTreasure(p));
    return p;
}
