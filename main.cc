#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>


#include <boost/asio.hpp>
#include <discordpp/bot.hh>
#include <discordpp/plugin-overload.hh>
#include <discordpp/rest-beast.hh>
#include <discordpp/websocket-beast.hh>
#include <discordpp/plugin-responder.hh>

namespace asio = boost::asio;
namespace dpp = discordpp;
using json = nlohmann::json;
using DppBot = dpp::PluginResponder<dpp::PluginOverload<dpp::WebsocketBeast<dpp::RestBeast<dpp::Bot>>>>;

#include "event.h"
#include "networkManager.h"

//Discordpp
std::string m_token;

//Time
double m_currentTime;
std::chrono::time_point<std::chrono::high_resolution_clock> m_curTime;
std::chrono::time_point<std::chrono::high_resolution_clock> m_prevTime;
float m_deltaTime;

//Message queue
SharedQueue<Message> m_messageQueue;
std::thread m_listenLoop;

//Shutdown related
std::atomic<bool> m_alive;
std::promise<void> m_exitSignal;
std::shared_future<void> m_future;
std::thread m_terminateThread;

//Network maanger
int m_listenPort;
NetworkManager m_networkManager;

void Loop(std::shared_ptr<asio::io_context>& aioc, std::shared_ptr<DppBot>& bot);
void CreateHandlers(std::shared_ptr<DppBot>& bot);
void Stop(std::shared_ptr<asio::io_context>& aioc);
void WaitForTerminate();
std::istream &safeGetline(std::istream &is, std::string &t);
void filter(std::string &target, const std::string &pattern);

int main(){
	//Load token
	std::ifstream tokenFile("token.dat");
	if(!tokenFile){
		std::cerr << "Could not read token" << std::endl;
		exit(1);
	}
	safeGetline(tokenFile, m_token);
	tokenFile.close();

	//Create bot
	auto bot = std::make_shared<DppBot>();
	auto aioc = std::make_shared<asio::io_context>();
	
	//Prepare bot
	bot->debugUnhandled = false;
	bot->prefix = "~";
	//Create handlers
	CreateHandlers(std::ref(bot));

	//Prepare threads
	m_alive = true;
	m_curTime = std::chrono::high_resolution_clock::now();
	m_future = m_exitSignal.get_future();
	//Fire threads
	m_listenLoop = std::thread(&Loop, std::ref(aioc), std::ref(bot));
	m_terminateThread = std::thread(&WaitForTerminate);
	m_networkManager.Init(27016, m_messageQueue);

    //Start Bot
	bot->initBot(6, m_token, aioc);
	bot->run();
	Stop(std::ref(aioc));
	m_networkManager.Stop();
	m_networkManager.MessageClient("relaunching");
    if(m_listenLoop.joinable()) m_listenLoop.join();
    if(m_terminateThread.joinable()) m_terminateThread.join();
	return 0;
}

void Loop(std::shared_ptr<asio::io_context>& aioc, std::shared_ptr<DppBot>& bot)
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
			Message* message = &m_messageQueue.front();
			if(message->method == "shut")
			{
				m_networkManager.MessageClient("shut");
				m_networkManager.Stop();
				Stop(std::ref(aioc));
			}
			else if(message->method == "stop")
				m_networkManager.MessageClient("shut");
			else if(message->method != "" && message->content != "" && message->content != "")
			{
				std::cout << message->method << ';' << message->type << ';' << message->content << std::endl;
				bot->call(message->method, message->type, message->content);
			}
			m_messageQueue.pop_front();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}
}

void CreateHandlers(std::shared_ptr<DppBot>& bot)
{
   	json self;
	bot->handlers.insert({
		"MESSAGE_CREATE",
		[&bot, &self](json msg) {
			if(msg["content"].get<std::string>().substr(0,1) == bot->prefix)
			{
				std::string content = msg["content"].get<std::string>().substr(1);
				int pos = content.find(' ');
				std::string command = content.substr(0,pos++);
				content = pos < content.length() - 1 ? content.substr(pos) : "";
				if(msg["author"]["id"].get<std::string>() == "99681408724766720" || command == "join-queue")
				{
					std::string userId = msg["author"]["id"];
					std::string channelId = msg["channel_id"];
					std::string guildId = msg["guild_id"].is_null() ? "" : msg["guild_id"];
					Event event(false, userId, channelId, guildId, command, content);
					m_networkManager.MessageClient(event.ToNetworkable());
				}
			}
		}});
	bot->handlers.insert({
			"CHANNEL_CREATE",
			[&bot, &self](json msg) {
				if(!msg["guild_id"].is_null())
				{
					std::string command = "new-group";
					std::string content = msg["name"].get<std::string>() + " " +
						(msg["parent_id"].is_null() ? "" : msg["parent_id"].get<std::string>()) + " " +
						msg["id"].get<std::string>() + " " +
						std::to_string(msg["position"].get<int>()) + " " +
						std::to_string(msg["type"].get<int>());
					
					Event event(true, "", "", "", command, content);
					m_networkManager.MessageClient(event.ToNetworkable());
				}
			}});
	bot->handlers.insert({
			"CHANNEL_DELETE",
			[&bot, &self](json msg) {
				std::string command = "empty";
				Event event(true, "", "", "", command, "");
				m_networkManager.MessageClient(event.ToNetworkable());
			}});
	bot->handlers.insert({
			"CHANNEL_UPDATE",
			[&bot, &self](json msg) {
				std::string command = "empty";
				Event event(true, "", "", "", command, "");
				m_networkManager.MessageClient(event.ToNetworkable());
			}});
	bot->handlers.insert({
			"GUILD_MEMBERS",
			[&bot, &self](json msg) {
				std::string command = "empty";
				Event event(true, "", "", "", command, "");
				m_networkManager.MessageClient(event.ToNetworkable());
			}});
	bot->handlers.insert({
			"GUILD_MEMBER_UPDATE",
			[&bot, &self](json msg) {
				std::string command = "empty";
				Event event(true, "", "", "", command, "");
				m_networkManager.MessageClient(event.ToNetworkable());
			}});
}

void WaitForTerminate()
{
    while(m_alive && m_future.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout);
}

void Stop(std::shared_ptr<asio::io_context>& aioc)
{
	if(m_future.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout)
		m_exitSignal.set_value();
    m_alive = false;
	aioc->stop();
}

std::istream& safeGetline(std::istream &is, std::string &t)
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

void filter(std::string &target, const std::string &pattern)
{
	while (target.find(pattern) != std::string::npos)
	{
		target = target.substr(0, target.find(pattern)) +
				 target.substr(target.find(pattern) + (pattern).size());
	}
}