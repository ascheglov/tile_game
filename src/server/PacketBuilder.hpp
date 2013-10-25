#pragma once

#include <sstream>

class PacketBuilder
{
public:
    explicit PacketBuilder(const char* type)
    {
        ss << "{\"type\":\"" << type << '"';
    }

    PacketBuilder& field(const char* name, int value)
    {
        ss << ",\"" << name << "\":" << value;
        return *this;
    }

    PacketBuilder& field(const char* name, const std::string& value)
    {
        ss << ",\"" << name << "\":\"" << value << '"';
        return *this;
    }

    std::string close()
    {
        ss << '}';
        return ss.str();
    }

private:
    std::stringstream ss;
};

