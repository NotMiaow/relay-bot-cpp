#include <iostream>

#include <thread>
#include <chrono>

#include "relayBot.h"

int main(){
	RelayBot relayBot(27016);
	relayBot.Init();
	relayBot.Start();
	while(relayBot.Alive())	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	return 0;
}

