#include "relayBot.h"

RelayBot::RelayBot(const int& listenPort)
{
//	m_aioc = std::make_shared<asio::io_context>();
//	m_bot = std::make_shared<DppBot>();

    m_listenPort = listenPort;
}

RelayBot::~RelayBot()
{
    m_listenLoop.join();
    m_terminateThread.join();
}

void RelayBot::Init()
{
   	json self;

    //HANDLERS HERE
}

void RelayBot::Start()
{
	std::cout << "Starting bot...\n\n";
    LoadToken();
//	m_bot->debugUnhandled = false;
//	m_bot->prefix = "/";

	// Prepare threads
	m_alive = true;
	m_curTime = std::chrono::high_resolution_clock::now();
	//Setup network manager
	futureObj = m_exitSignal.get_future();
	// Fire threads
	m_listenLoop = std::thread(&RelayBot::Loop, this);
	m_terminateThread = std::thread(&RelayBot::WaitForTerminate, this);
	m_networkManager.Init(m_listenPort, m_messageQueue);

    //Start Bot
//	m_bot->initBot(6, m_token, m_aioc);
//	m_bot->run();
}

void RelayBot::Loop()
{
	bool delayNextAPIRequest = false;
	float waitForBotTimer = 0.0f;
	float delayTimer = 0.0f;
	float waitTimer = 0.0f;

	while(m_alive == true)
	{
		m_prevTime = m_curTime;
		m_curTime = std::chrono::high_resolution_clock::now();
		m_deltaTime = (float)((std::chrono::duration<double>)(m_curTime - m_prevTime)).count();
		m_currentTime += m_deltaTime;

		if(delayNextAPIRequest)
		{
			delayTimer += m_deltaTime;
			if(delayTimer > 0.550f)
			{
				delayTimer = 0.0f;
				delayNextAPIRequest = false;
			}
		}
		else if(m_messageQueue.size())
		{
			delayNextAPIRequest = true;
			std::cout << m_messageQueue.size() << std::endl;
			Message* message = &m_messageQueue.front();
			std::cout << "method: " << message->method << std::endl;
			std::cout << "type: " << message->type << std::endl;
			std::cout << "content: " << message->content << std::endl;
			if(message->method != "")
			{
				if(message->method == "shut")
				{
					m_networkManager.MessageClient("shut");
					m_networkManager.Stop();
					Stop();
				}
				else if(message->method == "stop")
					m_networkManager.MessageClient("shut");
//				else
//					m_bot->call(message->method, message->type, message->content);
			}
			m_messageQueue.pop_front();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}
}

bool RelayBot::Alive()
{
	return m_alive;
}

void RelayBot::LoadToken()
{
	std::ifstream tokenFile("token.dat");
	if(!tokenFile){
		std::cerr << "Could not read token" << std::endl;
		exit(1);
	}
	safeGetline(tokenFile, m_token);
	tokenFile.close();
}

void RelayBot::WaitForTerminate()
{
    while(m_alive && futureObj.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout);
}

void RelayBot::Stop()
{
	if(futureObj.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout)
		m_exitSignal.set_value();
    m_alive = false;
//	m_aioc->stop();
}

std::istream& RelayBot::safeGetline(std::istream &is, std::string &t)
{
	t.clear();
	std::istream::sentry se(is, true);
	std::streambuf *sb = is.rdbuf();

	for (;;)
	{
		int c = sb->sbumpc();
		switch (c)
		{
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
			{
				sb->sbumpc();
			}
			return is;
		case std::streambuf::traits_type::eof():
			// Also handle the case when the last line has no line ending
			if (t.empty())
			{
				is.setstate(std::ios::eofbit);
			}
			return is;
		default:
			t += (char)c;
		}
	}
}

void RelayBot::filter(std::string &target, const std::string &pattern)
{
	while (target.find(pattern) != std::string::npos)
	{
		target = target.substr(0, target.find(pattern)) +
				 target.substr(target.find(pattern) + (pattern).size());
	}
}