#ifndef GAME_CLIENT_HPP
#define GAME_CLIENT_HPP

#include "GameConfig.hpp"
#include "Window.hpp"
#include "NetworkManager.hpp"
#include "Position.hpp"
#include "EnumMessageType.hpp"

namespace cat {

class GameClient {
  public:
  	GameClient(std::string nome_interface);
	virtual ~GameClient();

	void run();
	void addGrid();
	void addText();

	EnumMessageType processKeyPressed(const sf::Event::KeyPressed* keyPressed);
	Position sendMove(EnumMessageType move);
	void setPlayerPosition(float center_x, float center_y);
	long long getDiskFreeSpace(long long minFree = 10<<10);//deixa pelo menos 10 MB livre
	void errorCheck(int px, int py);	

  private:
	Window* window;
	NetworkManager* netManager;
};

}

#endif //GAME_CLIENT_HPP
