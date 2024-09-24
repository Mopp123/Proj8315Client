#pragma once

#include "BaseScene.h"
#include "world/World.h"
#include "CameraUtils.h"


class InGame : public BaseScene
{
private:
    world::World* _pWorld = nullptr;
    CameraController* _pCamController = nullptr;

    const size_t _testMapWidth = 20;
    const size_t _observeAreaRadius = 15;
    const size_t _observeAreaWidth = _observeAreaRadius * 2 + 1;
    std::vector<uint64_t> _testMapFull;
    std::vector<uint64_t> _testMapLocal;

public:
    InGame();
    ~InGame();

    void init();
    void update();
    virtual void lateUpdate();
};
