#ifndef EVENT_H__
#define EVENT_H__

#include <string>
#include <sstream>

struct Event
{
	Event() { }
	Event(bool fromAPI, std::string userId,std::string channelId, std::string guildId, std::string userName, std::string messageId, std::string command, std::string content) :
        fromAPI(fromAPI), userId(userId), channelId(channelId), guildId(guildId), userName(userName), messageId(messageId), command(command), content(content) { }
	~Event() { }
	std::string ToDebuggable()
	{
		std::ostringstream os;
		os << '{' << "EventInfo" << fromAPI << ';' << userId << ';' << channelId << ';' << guildId << ';' <<
			userName << ';' << messageId << ';' << command << ';' << content << '}';
		return os.str();
	}
	std::string ToNetworkable()
	{
		std::ostringstream os;
		os << fromAPI << ';' << userId << ';' << channelId << ';' << guildId << ';' << userName << ';' << messageId << ';' << command << ';' << content;
		return os.str();
	}
	
	bool fromAPI;
	std::string userId;
	std::string channelId;
	std::string guildId;
	std::string userName;
	std::string messageId;
    std::string command;
    std::string content;
};

#endif