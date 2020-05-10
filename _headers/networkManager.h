#ifndef NETWORK_MANAGER_H__
#define NETWORK_MANAGER_H__

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <thread>
#include <chrono>
#include <future>

#include "shared_queue.h"
#include "message.h"

class NetworkManager
{
public:
    NetworkManager() { }
	~NetworkManager();
	void Init(std::shared_future<void>&& serverFuture, const int serverPort, SharedQueue<Message>& messageQueue);
	void MessageClient(std::string message);
private:
	bool SetUpClientEnvironment(const int serverPort);
	void AcceptConnection(sockaddr_in& address);
	void ListenToClient();
	int GetMessageLength(std::string& cutMessage);
	bool SendString(std::string cutMessage);
	void WaitForTerminate();
private:
	std::shared_future<void> m_serverFuture;
	std::atomic<bool> m_alive;
	std::thread m_terminateThread;
	std::thread m_listeningThread;
    std::thread m_clientThread;

	int m_serverPort;
	int m_listeningSocket;
	int m_clientSocket;

    bool m_socketActive;
    SharedQueue<Message>* m_messageQueue;
    const size_t BUFFER_LENGTH = 4096;
};

#endif