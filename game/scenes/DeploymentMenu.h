#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "../ui/ui.h"


class DeploymentMenu : public pk::Scene
{
private:

public:
    DeploymentMenu();
    ~DeploymentMenu();

    virtual void init();
    virtual void update();
};
