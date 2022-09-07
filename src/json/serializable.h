#pragma once
#include <json/json.h>
#define OSPARAM(js, obj) obj = js[##obj]

namespace json
{
    // Anything that is serializable to json
    class Serializable
    {
    public:        
        inline virtual ~Serializable() {}
        inline virtual std::string to_json() const { return ""; }
        inline virtual void from_json(Json::Value& value) {}
    };
}
