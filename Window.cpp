#include "Window.hpp"
#include "GameConfig.hpp"
#include "EnumImageType.hpp"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp> 

using namespace cat;
Window::Window(std::string name, unsigned int width, unsigned int height){
	this->font = nullptr;
	this->player = this->treasure = this->footprint = nullptr;
	this->player_texture = this->treasure_texture = this->footprint_texture = nullptr;
	
	this->create (sf::VideoMode({width, height}), name,
									sf::Style::Titlebar | sf::Style::Close);
}

Window::~Window(){
	this->texts.clear();
	this->sprites.clear();
	this->rectangles.clear();	

	delete this->font;
	delete this->player;
	delete this->treasure;
	delete this->footprint;

	this->close();
}

void Window::addRectangle(sf::RectangleShape& rect){
	this->rectangles.push_back(rect);
}

void Window::addText(sf::Text& txt){
	this->texts.push_back(txt);
}

void Window::setFont(const std::string font_path){
	if (this->font)
		delete this->font;
	this->font = new sf::Font(font_path);
}

void Window::setIcon(sf::Sprite*& sprite, sf::Texture*& texture, const std::string icon_path){
	if (texture){
		delete texture;
		delete sprite;
	}
	
	texture = new sf::Texture(icon_path);
	sprite  = new sf::Sprite(*texture);

	sf::Vector2<float> size {sprite->getLocalBounds().size};
	sprite->setOrigin({size.x/2, size.y/2});
}

void Window::setPlayerIcon(const std::string icon_path){
	this->setIcon(this->player, this->player_texture, icon_path);
	this->player->setScale({GameConfig::PLAYER_ICON_SCALE_W,
											GameConfig::PLAYER_ICON_SCALE_H});
}

void Window::setErrorIcon(const std::string icon_path){
	this->setIcon(this->error, this->error_texture, icon_path);
	this->error->setScale({GameConfig::ERROR_ICON_SCALE_W,
											GameConfig::ERROR_ICON_SCALE_H});
}

void Window::setTreasureIcon(const std::string icon_path){
	this->setIcon(this->treasure, this->treasure_texture, icon_path);
	this->treasure->setScale({GameConfig::TREASURE_ICON_SCALE_W,
											GameConfig::TREASURE_ICON_SCALE_H});
}

void Window::setFootprintIcon(const std::string icon_path){
	this->setIcon(this->footprint, this->footprint_texture, icon_path);
	this->footprint->setScale({GameConfig::FOOTPRINT_ICON_SCALE_W,
											GameConfig::FOOTPRINT_ICON_SCALE_H});
}

sf::Sprite Window::cloneSprite(const sf::Sprite* original){
    sf::Sprite copy = sf::Sprite(original->getTexture());

    copy.setScale(original->getScale());
	copy.setOrigin(original->getOrigin());

    return copy;
}

void Window::addImage(EnumImageType type, float x, float y){
	//ignora posicao negativa
    if (x < 0 || y < 0)
        return;

    //adiciona se nao houver nada naquela posicao, mas se for um tesouro, troca por ele
    for (std::vector<sf::Sprite>::iterator it {this->sprites.begin()};
													it != this->sprites.end();){
        sf::Vector2f p = it->getPosition();
		//calcula de pixel pra posicao em grid 8x8
		p = {(p.x - GameConfig::GRID_TILE_WIDTH / 2) / GameConfig::GRID_TILE_WIDTH,
			(p.y - GameConfig::GRID_TILE_HEIGHT - GameConfig::HEADER_PADDING_Y / 3)
													/ GameConfig::GRID_TILE_HEIGHT};
		//se estiver na mesma posicao
        if (p.x == x && p.y == y){
			//se for o player ou tesouro nao remove sprite	
			if (&it->getTexture() == this->player_texture
							|| &it->getTexture() == this->treasure_texture){
				it++;
				continue;
			}

			if (type == EnumImageType::TREASURE){
				it = this->sprites.erase(it);
				break;
			}else
				return;
		} else ++it;
    }

	sf::Sprite* original = this->footprint;
	if (type == EnumImageType::PLAYER)   original = this->player;
	if (type == EnumImageType::TREASURE) original = this->treasure;
	if (type == EnumImageType::ERROR) original = this->error;
    sf::Sprite sprite = cloneSprite(original);

    sprite.setPosition({
        x * GameConfig::GRID_TILE_WIDTH + GameConfig::GRID_TILE_WIDTH/2,
        y * GameConfig::GRID_TILE_HEIGHT + GameConfig::GRID_TILE_HEIGHT + GameConfig::HEADER_PADDING_Y/3
    });


	if (type == EnumImageType::PLAYER)
		this->sprites.insert(this->sprites.begin(), sprite);//adiciona player no comeco
    else
		this->sprites.push_back(sprite);
}

void Window::updateCounter(char setValue){
	std::string s {this->texts[1].getString()};

	if (setValue < 0) s[0]++;
	else s[0] = '0'+setValue;

	this->texts[1].setString(s);
	this->texts[1].setPosition({GameConfig::WINDOW_WIDTH - GameConfig::HEADER_PADDING_X
										- this->texts[1].getLocalBounds().size.x, 0});
}
sf::Font& Window::getFont(){
	return *this->font;
}

void Window::setPlayerRotation(int rotation){
	this->sprites[0].setRotation(sf::degrees(rotation*90));
}

void Window::setPlayerPosition(float center_x, float center_y){
	if (center_x < 0 || center_y < 0 || this->sprites.empty())
		return;

	//se existe um player, ele fica no index 0
	this->sprites[0].setPosition({
		center_x * GameConfig::GRID_TILE_WIDTH + GameConfig::GRID_TILE_WIDTH/2,
		center_y * GameConfig::GRID_TILE_HEIGHT + GameConfig::GRID_TILE_HEIGHT + GameConfig::HEADER_PADDING_Y/3
	});
}

void Window::clearImages(){
	this->sprites.clear();
}

void Window::updateDisplay(){
	this->clear(GameConfig::BACKGROUND_COLOR);

	for (sf::RectangleShape& rect : this->rectangles)
		this->draw(rect);

	for (sf::Text& txt : this->texts)
		this->draw(txt);
	
	//printa primeiro as footprints, dps tesouros e dps o player
	for (int i{static_cast<int>(this->sprites.size()-1)}; i>=0; --i) 
		this->draw(this->sprites[i]);
    
	this->display();
}

