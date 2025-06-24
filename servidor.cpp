#include "GameServer.hpp"

int main(){
	cat::GameServer s {"enp0s31f6"};
	s.run();
	return 0;
}
