#include <iostream>

#include "relayBot.h"

int main(){
	RelayBot relayBot(27016);
	relayBot.Init();
	relayBot.Start();

	while(relayBot.Alive())
	{
		std::string command;
		std::getline(std::cin, command);
		if(command == "stop")
			relayBot.Stop();
	}
	return 0;
}

