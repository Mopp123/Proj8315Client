#pragma once

#include "../../PortablePesukarhu/ppk.h"


class BaseScene : public pk::Scene
{
protected:
    pk::Font* _pDefaultFont = nullptr;
    pk::TextRenderable* _pInfoTxt = nullptr;

public:
    BaseScene();
    virtual ~BaseScene();
    void initBase();

    void setInfoText(const std::string& txt, pk::vec3 color = pk::vec3(1.0f, 1.0f, 1.0f));
};
