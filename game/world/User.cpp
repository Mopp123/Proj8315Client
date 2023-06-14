#include "User.h"

User::User(const char* name, size_t nameLen)
{
    if (nameLen > USER_NAME_LEN)
        nameLen = USER_NAME_LEN;
    memset(_name, 0, sizeof(char) * USER_NAME_LEN);
    memcpy(_name, name, sizeof(char) * nameLen);
}


User::User(const User& other)
{

}

