#include "InGame.h"


using namespace pk;


InGame::InGame()
{}

InGame::~InGame()
{
    delete _pCamController;
}

void InGame::init()
{
    initBase();
    _pCamController = new CameraController(activeCamera, 2.5f);

    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);

    _pWorld = new world::World(
        (Scene&)(*this),
        pCamTransform,
        _observeAreaRadius
    );

    _testMapFull.resize(_testMapWidth * _testMapWidth * sizeof(uint64_t), 0);
    _testMapLocal.resize(_observeAreaWidth * _observeAreaWidth * sizeof(uint64_t), 0);

    _pWorld->updateObservedArea(_testMapLocal.data());
}

void InGame::update()
{
    _pCamController->update();

    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);
    mat4& camTMat = pCamTransform->accessTransformationMatrix();
    _pWorld->update(camTMat[0 + 3 * 4], camTMat[2 + 3 * 4]);

    setInfoText(
        "Delta: " + std::to_string(Timing::get_delta_time())
    );
}
