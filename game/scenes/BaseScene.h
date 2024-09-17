#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "../ui/ui.h"
#include "ecs/components/renderable/TextRenderable.h"


class BaseScene : public pk::Scene
{
protected:
    pk::TextRenderable* _pInfoTxt = nullptr;

    pk::TextRenderable* _pFPSCounterText = nullptr;
    Panel _debugPanel;

public:
    BaseScene();
    virtual ~BaseScene();
    void initBase();
    void updateDebugPanel();
    void setInfoText(const std::string& txt, pk::vec3 color = pk::vec3(1.0f, 1.0f, 1.0f));
};
