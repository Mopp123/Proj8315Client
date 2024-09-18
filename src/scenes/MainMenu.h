#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "ui/Panel.h"


class MainMenu : public BaseScene
{
private:
    Panel _mainPanel;

public:
    MainMenu();
    ~MainMenu();

    void init();
    void update();
};
