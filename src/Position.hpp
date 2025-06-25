#ifndef POSITION_HPP
#define POSITION_HPP

#include <ostream>

namespace cat {

class Position {
  public: 
	Position();
    Position(int x, int y);
    virtual ~Position()=default;
    
	static Position rand(int modX, int modY);
	bool operator==(const Position& other) const;
    
	int x, y;
};
	std::ostream& operator<<(std::ostream& os, const Position& pos);
}

#endif
