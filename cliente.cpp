#include "GameClient.hpp"

int main(){
	cat::GameClient c {"veth0"};
	c.run();
	return 0;
}
