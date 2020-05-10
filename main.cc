#include <iostream>

#include "relayBot.h"

int main(){
	RelayBot relayBot(27016);
	relayBot.Init();
	relayBot.Start();

	bool alive = true;
	while(alive)
	{
		std::string command;
		std::getline(std::cin, command);
		if(command == "stop")
		{
			alive = false;
		}
	}
	relayBot.Stop();
	return 0;
}

