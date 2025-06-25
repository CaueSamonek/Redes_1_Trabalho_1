#include "Message.hpp"
#include "Position.hpp"
#include "GameClient.hpp"
#include "GameConfig.hpp"
#include "EnumImageType.hpp"
#include "NetworkManager.hpp"
#include "EnumMessageType.hpp"

#include <iostream>
#include <cstdlib>
#include <string>
#include <sys/statvfs.h>//ver espaco livre
#include <sys/stat.h>//ver tamanho de arquivo

namespace fs = std::filesystem;
using namespace cat;

GameClient::GameClient(std::string nome_interface){
	this->netManager = new NetworkManager(nome_interface);
	this->window = new Window(GameConfig::WINDOW_NAME, GameConfig::WINDOW_WIDTH,
													GameConfig::WINDOW_HEIGHT);
	this->window->setFont(GameConfig::FONT_PATH);
	this->window->setErrorIcon(GameConfig::ERROR_ICON_PATH);
	this->window->setPlayerIcon(GameConfig::PLAYER_ICON_PATH);
	this->window->setTreasureIcon(GameConfig::TREASURE_ICON_PATH);
	this->window->setFootprintIcon(GameConfig::FOOTPRINT_ICON_PATH);
	this->addText();
	this->addGrid();
	
	//diretorio destino de arquivos transferidos
	fs::create_directories(GameConfig::DESTINY_DIR_PATH);
}

GameClient::~GameClient(){
	delete this->netManager;
	delete this->window;
}

EnumMessageType GameClient::processKeyPressed(const sf::Event::KeyPressed* keyPressed){
	EnumMessageType type;

	switch (keyPressed->scancode){
		case sf::Keyboard::Scancode::Up:     type = (EnumMessageType::MOVE_UP);    break;
		case sf::Keyboard::Scancode::Down:   type = (EnumMessageType::MOVE_DOWN);  break;
		case sf::Keyboard::Scancode::Left:   type = (EnumMessageType::MOVE_LEFT);  break;
		case sf::Keyboard::Scancode::Right:  type = (EnumMessageType::MOVE_RIGHT); break;
		case sf::Keyboard::Scancode::Escape: type = (EnumMessageType::INIT);       break;
		default: type = EnumMessageType::ERROR;
	}

	return type;
}

void GameClient::run(){
	this->netManager->sync();
	this->netManager->sendAndWait(EnumMessageType::INIT);
    
    float px=-1;
    float py=-1;
	
    while (this->window->isOpen()){
		std::optional <cat::Message> mOpt = this->netManager->receive();
		if (mOpt.has_value()){
			Message m {mOpt.value()};
			if (m.getType() == EnumMessageType::OK){	
				Position newPos (m.getData()[0], m.getData()[1]);

				if (px==-1 && py==-1)
                	this->window->addImage(EnumImageType::PLAYER, newPos.x, newPos.y);
				else
					this->window->addImage(EnumImageType::FOOTPRINT, px, py);

			    px = newPos.x;
			    py = newPos.y;
                this->window->setPlayerPosition(newPos.x, newPos.y);
				this->window->setPlayerRotation(m.getData()[2]);

			//soh entra se deu certo, tratamento de erro feito em NetworkManager
			}
			else if (m.getType() == EnumMessageType::TEXT ||
									m.getType() == EnumMessageType::IMAGE ||
									m.getType() == EnumMessageType::VIDEO){
				std::string base (GameConfig::DESTINY_DIR_PATH);
				std::vector<unsigned char> v {m.getData()};
				std::string name (v.begin(),v.end());
				std::string filepath {base + name};

				this->netManager->receiveFile(filepath);
				this->window->updateCounter();
				this->window->addImage(EnumImageType::TREASURE, px, py);

				//pega variaveis necessarias pra interface
   				std::string display =  std::string(std::getenv("DISPLAY"));
			    std::string xauth = std::string(std::getenv("XAUTHORITY"));

				std::string bashCMD {"sudo DISPLAY=" + display
									+" XAUTHORITY=" + xauth
									+" open " + filepath};

				std::system(bashCMD.c_str());

				//descartar teclas apertadas durante recebimento
				while (this->window->pollEvent());

			}else
			if (m.getType() == EnumMessageType::HANDSHAKE){
				px=py=-1;
				this->window->updateCounter(0);
				this->window->clearImages();
				this->netManager->sendAndWait(EnumMessageType::INIT);
			}
		}


		if (const std::optional event = this->window->pollEvent()){
			//evento de fechar janela
			if (event->is<sf::Event::Closed>())
				this->window->close();

			//evento de apertar tecla
			const sf::Event::KeyPressed* keyPressed {event->getIf<sf::Event::KeyPressed>()};
			if (keyPressed){
				EnumMessageType move = this->processKeyPressed(keyPressed);

				if (move == EnumMessageType::INIT){
					px=py=-1;
					this->window->updateCounter(0);
					this->window->clearImages();
					this->netManager->sendAndWait(EnumMessageType::INIT);
				}else
				if (move != cat::EnumMessageType::ERROR)
					this->netManager->sendAndWait(move);//envia movimento e espera confirmacao
			}
			//limpa fila de eventos pra nao sobrecarregar => descartar acumulo de eventos
			while(this->window->pollEvent());
		}
		//verifica se ocorreu algum erro de transferencia de arquivo
		//adiciona detalhes a interface grafica se necessario
		this->errorCheck(px,py);
		//atualiza tela
		this->window->updateDisplay();
	}
}

void GameClient::errorCheck(int px, int py){
	//adiciona icone de erro na posicao atual do perasonagem
	if (!this->netManager->logString.empty())
		this->window->addImage(EnumImageType::ERROR, px, py);
	this->netManager->logString="";
}

void GameClient::addGrid(){
	for (int i{0}; i < GameConfig::GRID_NUM_ROWS; ++i)
		for (int j{0}; j < GameConfig::GRID_NUM_COLUMNS; ++j){

			sf::RectangleShape r = sf::RectangleShape({GameConfig::GRID_TILE_WIDTH,
													GameConfig::GRID_TILE_HEIGHT});

			float x = i*GameConfig::GRID_TILE_WIDTH;
			float y = j*GameConfig::GRID_TILE_HEIGHT;
			x += GameConfig::BORDER_LINE_WIDTH;
			y += GameConfig::HEADER_PADDING_Y;

			r.setPosition({x,y});
			r.setFillColor(sf::Color::Transparent);
			r.setOutlineColor(GameConfig::SQUARE_COLOR);
			r.setOutlineThickness(GameConfig::BORDER_LINE_WIDTH);

			this->window->addRectangle(r);
		}
}

void GameClient::addText(){
	sf::Text text {this->window->getFont()};
	text.setCharacterSize(GameConfig::CHARACTER_SIZE);
	text.setFillColor(GameConfig::TEXT_COLOR);

	text.setString("Tesouros:");
	text.setPosition({GameConfig::HEADER_PADDING_X, 0});
	this->window->addText(text);

	text.setString("0/8");
	float posX {text.getLocalBounds().size.x};
	text.setPosition({GameConfig::WINDOW_WIDTH-GameConfig::HEADER_PADDING_X-posX, 0});
	this->window->addText(text);
}
