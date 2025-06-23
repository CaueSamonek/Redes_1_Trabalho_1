#include <string>
#include "Treasure.hpp"

#include <iomanip>
#include <ostream>

using namespace cat;

Treasure::Treasure(std::string name, Position position)
						: found{false}, position{position}, filename{name}{}


std::ostream& cat::operator<<(std::ostream& os, const Treasure& treasure){
	os << std::left << std::setw(20) << treasure.filename << " : " << "Posição = " << treasure.position << " Achado = ";
	if (treasure.found)
		os << "Sim\n";
	else
		os << "Não\n";
	return os;
}
