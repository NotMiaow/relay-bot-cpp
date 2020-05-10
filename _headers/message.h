#ifndef MESSAGE_H__
#define MESSAGE_H__

#include <string>
#include <nlohmann/json.hpp>
#include "basicLib.h"

struct Message
{
    Message() {}
    Message(std::string message)
    {
        std::vector<std::string> parameters;
    	if (content.length() - message.length() != 0)
	    	parameters = Split(content, ';');
        
        if(parameters.size() == 3)
        {
            this->method = parameters[0];
            this->type = parameters[1];
            this->content = parameters[2];
        }
    }
    ~Message() { }

    std::string method = "";
    std::string type = "";
    std::string content = "";
};

#endif