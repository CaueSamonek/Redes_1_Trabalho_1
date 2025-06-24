#include "GameClient.hpp"

int main(){
	cat::GameClient c {"enp0s31f6"};
	c.run();
	return 0;
}
