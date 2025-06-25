#ifndef GAME_CONFIG_HPP
#define GAME_CONFIG_HPP

#include <SFML/Graphics.hpp>

namespace cat {

class GameConfig {
  public:
	//diretorio destino dos arquivos recebidos
	static constexpr const char* DESTINY_DIR_PATH {"received/"};

	//nome e tamanho da janela
	static constexpr const char* WINDOW_NAME {"Caça Ao Tesouro Very Poggers >:D"};
	static constexpr unsigned int WINDOW_WIDTH {920};
	static constexpr unsigned int WINDOW_HEIGHT {1000};

	//quantidades e tamanhos
	static constexpr unsigned char CHARACTER_SIZE {70};
	static constexpr unsigned char GRID_NUM_ROWS {8};
	static constexpr unsigned char GRID_NUM_COLUMNS {8};
	static constexpr unsigned char BORDER_LINE_WIDTH {2};
	static constexpr unsigned char HEADER_PADDING_Y {80};
	static constexpr unsigned char HEADER_PADDING_X {10};
	static constexpr float GRID_TILE_WIDTH {(WINDOW_WIDTH-2*BORDER_LINE_WIDTH)/GRID_NUM_ROWS};
	static constexpr float GRID_TILE_HEIGHT {(WINDOW_HEIGHT-2*BORDER_LINE_WIDTH-HEADER_PADDING_Y)/GRID_NUM_COLUMNS};

	//caminhos
	static constexpr const char* FONT_PATH {"./SFML/fonts/font.ttf"};
	static constexpr const char* ERROR_ICON_PATH {"./SFML/images/errorIcon.png"};
	static constexpr const char* PLAYER_ICON_PATH {"./SFML/images/playerIcon.png"};
	static constexpr const char* TREASURE_ICON_PATH {"./SFML/images/treasureIcon.png"};
	static constexpr const char* FOOTPRINT_ICON_PATH {"./SFML/images/footprintIcon.png"};

	//escalas
	static constexpr float PLAYER_ICON_SCALE_W {0.1};
	static constexpr float PLAYER_ICON_SCALE_H {0.1};
	static constexpr float FOOTPRINT_ICON_SCALE_W {0.1};
	static constexpr float FOOTPRINT_ICON_SCALE_H {0.1};
	static constexpr float TREASURE_ICON_SCALE_W {0.15};
	static constexpr float TREASURE_ICON_SCALE_H {0.15};
	static constexpr float ERROR_ICON_SCALE_W {0.13};
	static constexpr float ERROR_ICON_SCALE_H {0.13};

	//cores
	static constexpr sf::Color TEXT_COLOR    {sf::Color(198, 160, 246)};
	static constexpr sf::Color SQUARE_COLOR  {sf::Color(198, 160, 246)};
	static constexpr sf::Color BACKGROUND_COLOR {sf::Color(30, 32, 48)};
};

}

#endif
