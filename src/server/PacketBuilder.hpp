#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include "types.hpp"

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

    PacketBuilder& field(const char* name, ObjectId id)
    {
        return field(name, id.value);
    }

    PacketBuilder& field(const char* name, std::uint64_t value)
    {
        assert(double(value) == value && "value won't fit in JS Number");
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

