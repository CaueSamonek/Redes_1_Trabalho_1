#include "Position.hpp"
#include <cstdlib>
#include <ostream>

using namespace cat;

Position::Position() : x{-1}, y{-1}{}
Position::Position(int x, int y) : x{x}, y{y}{}

Position Position::rand(int modX, int modY){
	return {std::rand()%modX, std::rand()%modY};
}

bool Position::operator==(const Position& other) const {
	return this->x == other.x && this->y == other.y;
}

std::ostream& cat::operator<<(std::ostream& os, const Position& pos) {
    os << "(" << pos.x <<","<< pos.y << ")";
    return os;
}
