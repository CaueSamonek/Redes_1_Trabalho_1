#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SFML/Graphics.hpp>
#include <vector>

#include "EnumImageType.hpp"

namespace cat {

class Window : public sf::RenderWindow {
  public:
  	Window(std::string name, unsigned int width, unsigned int height);
	virtual ~Window();

	void setErrorIcon(const std::string icon_path);
	void setPlayerIcon(const std::string icon_path);
	void setTreasureIcon(const std::string icon_path);
	void setFootprintIcon(const std::string icon_path);
	void setFont(const std::string font_path);
	
	void addText(sf::Text& text);
	void addRectangle(sf::RectangleShape& rect);
	void addImage(EnumImageType type, float x=-1, float y=-1);

	void setPlayerRotation(int rotation);
	void setPlayerPosition(float center_x, float center_y);
	void clearImages();

  	void updateDisplay();
	void updateCounter(char setValue=-1);
	sf::Font& getFont();
	sf::Text& getText(int index);

  private:
	sf::Font* font;
	
	sf::Sprite* error;
	sf::Sprite* player;
	sf::Sprite* treasure;
	sf::Sprite* footprint;

	sf::Texture* error_texture;
	sf::Texture* player_texture;
	sf::Texture* treasure_texture;
	sf::Texture* footprint_texture;

	std::vector<sf::Text> texts;
	std::vector<sf::Sprite> sprites;
	std::vector<sf::RectangleShape> rectangles;

	sf::Sprite cloneSprite(const sf::Sprite* original);
	void setIcon(sf::Sprite*& sprite, sf::Texture*& texture, const std::string icon_path);
};
}

#endif //WINDOW_HPP
