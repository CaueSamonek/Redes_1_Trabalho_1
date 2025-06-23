#include "GameServer.hpp"

int main(){
	cat::GameServer s {"veth1"};
	s.run();
	return 0;
}
