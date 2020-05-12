#ifndef MESSAGE_H__
#define MESSAGE_H__

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "basicLib.h"

using json = nlohmann::json;

struct Message
{
    Message() {}
    Message(const std::string& message)
    {
        std::vector<std::string> parameters = Split(message, ';');
        if(parameters.size() == 3)
        {
            this->method = parameters[0];
            this->type = parameters[1];
            this->content = parameters[2] == "" ? "" : json::parse(parameters[2]);
        }
    }
    ~Message() { }

    std::string method = "";
    std::string type = "";
    json content = "";
};

#endif