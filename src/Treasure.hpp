#ifndef TREASURE_HPP
#define TREASURE_HPP

#include <string>
#include <ostream>
#include "Position.hpp"

namespace cat {

class Treasure {

    public:
        Treasure(std::string name, Position position);
        virtual ~Treasure()=default;

        bool found;
        Position position;
        std::string filename;
};

	std::ostream& operator<<(std::ostream& os, const Treasure& treasure);
}

#endif

