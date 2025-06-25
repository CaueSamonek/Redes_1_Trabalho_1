#ifndef GAME_SERVER_HPP
#define GAME_CLIENT_HPP

#include <vector>
#include <string>
#include "Treasure.hpp"
#include "Position.hpp"
#include "NetworkManager.hpp"
#include "GameConfig.hpp"

namespace cat {

class GameServer {
    public:
        GameServer(std::string interface_name, Position gridMax =
													{GameConfig::GRID_NUM_COLUMNS, 
													 GameConfig::GRID_NUM_ROWS});
        virtual ~GameServer();
		
        void run();

    private:
        void createTreasures(std::vector<std::string> filenames);
        bool hasTreasure(Position& pos);       
        Position initPosition();
		void printGameState();
		void resetTreasures();

        std::vector<Position> playerMoveLog;
        std::vector<Treasure> treasures;
        NetworkManager* manager;
        
		Position gridMax;
        Position playerPosition;
        int playerRotation;
};

}


#endif
