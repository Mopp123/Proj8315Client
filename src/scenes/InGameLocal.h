#pragma once

#include "BaseScene.h"
#include "world/World.h"
#include "CameraUtils.h"
#include "ui/InGameUI.h"

#include "world/MousePicker.h"

#include "Pesukarhu/ppk.h"


class InGameLocal : public BaseScene
{
private:
    InGameUI _inGameUI;

    world::World* _pWorld = nullptr;
    CameraController* _pCamController = nullptr;

    const size_t _testMapWidth = 128;
    const size_t _observeAreaRadius = 15;
    const size_t _observeAreaWidth = _observeAreaRadius * 2 + 1;
    std::vector<uint64_t> _testMapFull;
    std::vector<uint64_t> _testMapLocal;

    world::MousePicker _mousePicker;


public:
    InGameLocal();
    ~InGameLocal();

    void init();
    void update();
    void lateUpdate();
};
