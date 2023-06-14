#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include <cstring>

#define FACTION_NAME_SIZE 32
#define FACTION_MAX_DEPLOY_COUNT 16

#define NULL_FACTION Faction("", 0)

class Faction
{
private:
    char _name[FACTION_NAME_SIZE];

    // Contains what objects faction spawns, when spawning to the map
    PK_ubyte _deployments[FACTION_MAX_DEPLOY_COUNT];

public:
    Faction(const PK_byte* nameData, size_t nameSize);
    Faction(const PK_byte* data);
    Faction(const Faction& other);

    void setDeployments(PK_ubyte* deployments, size_t count);
    const PK_byte* getNetwData() const;
    static size_t get_netw_size();

    inline const char* getName() const { return _name; }
    inline const PK_ubyte* getDeploymens() { return _deployments; }

    bool operator==(const Faction& other) { return strcmp(_name, other._name) == 0; }
};
