#pragma once

#include "../../PortablePesukarhu/ppk.h"


class MainMenu : public pk::Scene
{
private:
public:
    MainMenu();
    ~MainMenu();

    virtual void init();
    virtual void update();
};
