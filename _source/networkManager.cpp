#include "networkManager.h"

NetworkManager::~NetworkManager()
{
	close(m_listeningSocket);
	m_listeningThread.join();
    m_clientThread.join();
	m_terminateThread.join();
}

void NetworkManager::Init(std::shared_future<void>&& serverFuture, const int serverPort, SharedQueue<Message>& messageQueue)
{
	m_serverFuture = serverFuture;
    m_alive = true;
	m_terminateThread = std::thread(&NetworkManager::WaitForTerminate, this);

	m_serverPort = serverPort;
	m_messageQueue = &messageQueue;
	m_socketActive = false;
	SetUpClientEnvironment(serverPort);
}

bool NetworkManager::SetUpClientEnvironment(const int serverPort)
{
	sockaddr_in address;
	int opt = 1;

	if((m_listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		std::cout << "Failed at creating socket file descriptor." << std::endl;
		return false;
	}

	if(setsockopt(m_listeningSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		std::cout << "Failed at setsocketopt." << std::endl;
		return false;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(serverPort);

	if (bind(m_listeningSocket, (sockaddr*)&address, sizeof(address)) < 0) 
    { 
        std::cout << "Failed at bind." << std::endl; 
        return false;
    } 

    if (listen(m_listeningSocket, 3) < 0) 
    { 
        std::cout << "Failed at listen." << std::endl;; 
		return false;
    }

	m_listeningThread = std::thread(&NetworkManager::AcceptConnection, this, std::ref(address));
	return true;
}

void NetworkManager::AcceptConnection(sockaddr_in& address)
{
	while (m_alive)
	{
        if (m_socketActive)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int addrlen = sizeof(address);
        if((m_clientSocket = accept(m_listeningSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        m_socketActive = true;
        if (m_clientThread.joinable()) m_clientThread.join();
        m_clientThread = std::thread(&NetworkManager::ListenToClient, this);

		std::cout << "Mia has reached out, we shall be chatting. :)" << std::endl;
	}
}

void NetworkManager::ListenToClient()
{
	int iResult = 0;
	char recvbuf[BUFFER_LENGTH];
	int recvbuflen = BUFFER_LENGTH;
	std::string message = "";
	int messageLength = 0;
	do {
		iResult = read(m_clientSocket, recvbuf, recvbuflen);
		if (iResult > 0)
		{
			//Convert to string
			for(int i = 0; i < iResult; i++)
				message += recvbuf[i];
			
			//Get message length if not already
			if(messageLength == 0)
			{
				messageLength = GetMessageLength(message);
				if(messageLength == 0)
				{
					close(m_clientSocket);
					m_socketActive = false;
					continue;
				}
			}
			//Done receiving
			if(message.length() == messageLength)
			{
				Message mess(message);
				m_messageQueue->push_back(mess);
				message = "";
				messageLength = 0;
			}
			//Error
			else if(message.length() > messageLength)
			{
				close(m_clientSocket);
				m_socketActive = false;
				message = "";
				messageLength = 0;
			}
		}
		else {
			close(m_clientSocket);
			m_socketActive = false;
			return;
		}
	} while (iResult > 0 && m_alive);
	close(m_clientSocket);
	m_socketActive = false;

	iResult = shutdown(m_clientSocket, SHUT_WR);
}

int NetworkManager::GetMessageLength(std::string& cutMessage)
{
	int iMessageLength = 0;
	std::string sMessageLength = "";
	int pos = cutMessage.find_first_of(';');
	if(pos != std::string::npos)
	{
		sMessageLength = cutMessage.substr(0,pos);
		iMessageLength = std::stoi(sMessageLength);
		cutMessage = cutMessage.substr(pos + 1);
	}
	return iMessageLength;
}

void NetworkManager::MessageClient(std::string message)
{
	//Add message length to message
	message = std::to_string(message.length())  + ";" + message;

	//Send message
	std::string cutMessage;
	int i;
	for(i = 0; i + BUFFER_LENGTH < message.length(); i += BUFFER_LENGTH)
	{
		cutMessage = message.substr(i, BUFFER_LENGTH);
		if(!SendString(cutMessage))
			return;
	}	
	if(i < message.length())
	{
		cutMessage = message.substr(i);
		if(!SendString(cutMessage))
			return;
	}	
}

bool NetworkManager::SendString(std::string cutMessage)
{
	int iSendResult = send(m_clientSocket, cutMessage.c_str(), (int)cutMessage.length(), 0);
	if (iSendResult <= 0)
	{
        close(m_clientSocket);
        m_socketActive = false;
		return false;
	}
	return true;
}

void NetworkManager::WaitForTerminate()
{
    while(m_serverFuture.wait_for(std::chrono::milliseconds(400)) == std::future_status::timeout);
    m_alive = false;

	//Connect to itself to move past the accept bind in m_listeningThread
	//Allowing to join() in the destructor
	int serverStopSocket = 0;
	sockaddr_in serverAddress;
	if ((serverStopSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(m_serverPort);
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
		return;
	if(connect(serverStopSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		return;

    if(m_socketActive)
    {
        shutdown(m_clientSocket, SHUT_RDWR);
        close(m_clientSocket);
        m_socketActive = false;
    }	
	close(serverStopSocket);
}