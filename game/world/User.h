#pragma once

#include <cstring>
// TODO: Put all user related definitions here from net common!
#include "../../Proj8315Common/src/Common.h"


class User
{
private:
    char _name[USER_NAME_SIZE];
    bool _deployed = false;

public:
    User(const char* name, size_t nameLen);
    User(const User& other);

    inline const char* getName() const { return _name; }
    inline bool isDeployed() const { return _deployed; }

    bool operator==(const User& other) { return strcmp(_name, other._name) == 0; }
};