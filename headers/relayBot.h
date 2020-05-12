#ifndef RELAY_BOT_H__
#define RELAY_BOT_H__

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>

//#include <boost/asio.hpp>
//#include <discordpp/bot.hh>
//#include <discordpp/plugin-overload.hh>
//#include <discordpp/rest-beast.hh>
//#include <discordpp/websocket-beast.hh>
//#include <discordpp/plugin-responder.hh>

#include "shared_queue.h"
#include "message.h"
#include "networkManager.h"

//namespace asio = boost::asio;
//namespace dpp = discordpp;
using json = nlohmann::json;
//using DppBot = dpp::PluginResponder<dpp::PluginOverload<dpp::WebsocketBeast<dpp::RestBeast<dpp::Bot>>>>;

class RelayBot
{
public:
    RelayBot() { }
    RelayBot(const int& listenPort);
    ~RelayBot();
    void Init();
    void Start();
    void Loop();
    void Stop();
    bool Alive();
    void LoadToken();
    void WaitForTerminate();
    std::istream &safeGetline(std::istream &is, std::string &t);
    void filter(std::string &target, const std::string &pattern);
private:
    //Discordpp
    std::string m_token;
//    std::shared_ptr<asio::io_context> m_aioc;
//    std::shared_ptr<DppBot> m_bot;

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
    std::shared_future<void> futureObj;
    std::thread m_terminateThread;

    //Network maanger
    int m_listenPort;
    NetworkManager m_networkManager;
};

#endif