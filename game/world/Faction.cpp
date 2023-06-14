#include "Faction.h"


Faction::Faction(const PK_byte* nameData, size_t nameSize)
{
    if (nameSize > FACTION_NAME_SIZE)
        nameSize = FACTION_NAME_SIZE;
    memset(_name, 0, sizeof(char) * FACTION_NAME_SIZE);
    memcpy(_name, nameData, nameSize);
    if (nameSize > 0)
        memset(_deployments, 0, sizeof(PK_ubyte) * FACTION_MAX_DEPLOY_COUNT);
}

Faction::Faction(const PK_byte* data)
{
    memset(_name, 0, sizeof(char) * FACTION_NAME_SIZE);
    memcpy(_name, data, sizeof(char) * FACTION_NAME_SIZE);

    // NOTE: CURRENTLY WHEN JUST TESTING WE DONT YET GET DEPLOYMENTS FROM SERVER!!!
    memset(_deployments, 0, sizeof(PK_ubyte) * FACTION_MAX_DEPLOY_COUNT);
}

Faction::Faction(const Faction& other)
{
    memset(_name, 0, FACTION_NAME_SIZE);
    memset(_deployments, 0, FACTION_MAX_DEPLOY_COUNT);
    memcpy(_name, other._name, FACTION_NAME_SIZE);
    memcpy(_deployments, other._deployments, FACTION_MAX_DEPLOY_COUNT);
}

void Faction::setDeployments(PK_ubyte* deployments, size_t count)
{
    if (count <= FACTION_MAX_DEPLOY_COUNT)
        memcpy(_deployments, deployments, sizeof(PK_ubyte) * count);
}

const PK_byte* Faction::getNetwData() const
{
    return _name;
}

size_t Faction::get_netw_size()
{
    return FACTION_NAME_SIZE;
}
