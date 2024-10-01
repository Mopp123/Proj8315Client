#include "CameraUtils.h"

#include "../PortablePesukarhu/core/Debug.h"

using namespace pk;


CameraController::CameraKeyEvent::CameraKeyEvent(CameraController* pController) :
    pController(pController)
{}

void CameraController::CameraKeyEvent::func(pk::InputKeyName key, int scancode, pk::InputAction action, int mods)
{
    if (key == pk::InputKeyName::PK_INPUT_KEY_W)
        pController->_moveForward = action == pk::InputAction::PK_INPUT_PRESS;
    if (key == pk::InputKeyName::PK_INPUT_KEY_S)
        pController->_moveBackwards = action == pk::InputAction::PK_INPUT_PRESS;
    if (key == pk::InputKeyName::PK_INPUT_KEY_D)
        pController->_moveRight = action == pk::InputAction::PK_INPUT_PRESS;
    if (key == pk::InputKeyName::PK_INPUT_KEY_A)
        pController->_moveLeft = action == pk::InputAction::PK_INPUT_PRESS;

    if (key == pk::InputKeyName::PK_INPUT_KEY_SPACE)
        pController->_moveUp = action == pk::InputAction::PK_INPUT_PRESS;
    if (key == pk::InputKeyName::PK_INPUT_KEY_LCTRL)
        pController->_moveDown = action == pk::InputAction::PK_INPUT_PRESS;
}


CameraController::CameraMouseButtonEvent::CameraMouseButtonEvent(CameraController* pController) :
    pController(pController)
{}

void CameraController::CameraMouseButtonEvent::func(pk::InputMouseButtonName button, pk::InputAction action, int mods)
{
    pController->_enableMouseLook = button == pk::InputMouseButtonName::PK_INPUT_MOUSE_LEFT && action == pk::InputAction::PK_INPUT_PRESS;
}


CameraController::CameraCursorPosEvent::CameraCursorPosEvent(CameraController* pController) :
    pController(pController)
{}

void CameraController::CameraCursorPosEvent::func(int x, int y)
{
    float mouseDX = (float)(x - _prevMouseX);
    float mouseDY = (float)(y - _prevMouseY);

    if (pController->_enableMouseLook)
    {
        float sensitivity = 0.01f;

        pk::mat4 rotMatX;
        pk::mat4 rotMatY;
        pk::mat4 rotMatZ;

        _pitch -= mouseDY * sensitivity;
        _yaw -= mouseDX * sensitivity;

        pk::mat4 pitchMatrix;
        pitchMatrix.setIdentity();
        pitchMatrix[1 + 1 * 4] =  std::cos(-_pitch);
        pitchMatrix[1 + 2 * 4] = -std::sin(-_pitch);
        pitchMatrix[2 + 1 * 4] =  std::sin(-_pitch);
        pitchMatrix[2 + 2 * 4] =  std::cos(-_pitch);

        pk::mat4 yawMatrix;
        yawMatrix.setIdentity();
        yawMatrix[0 + 0 * 4] = std::cos(_yaw);
        yawMatrix[0 + 2 * 4] = std::sin(_yaw);
        yawMatrix[2 + 0 * 4] = -std::sin(_yaw);
        yawMatrix[2 + 2 * 4] = std::cos(_yaw);

        Transform* pTransform = pController->getCameraTransform();
        pk::mat4& camTMat = pTransform->accessTransformationMatrix();
        pk::mat4 oldTranslation;
        oldTranslation.setIdentity();
        oldTranslation[0 + 3 * 4] = camTMat[0 + 3 * 4];
        oldTranslation[1 + 3 * 4] = camTMat[1 + 3 * 4];
        oldTranslation[2 + 3 * 4] = camTMat[2 + 3 * 4];

        camTMat = oldTranslation * yawMatrix * pitchMatrix;
    }
    _prevMouseX = x;
    _prevMouseY = y;
}


CameraController::CameraController(entityID_t cameraEntityID, float speed) :
    _camEntity(cameraEntityID)
{
    _speed = speed;
    pk::Application* pApp = pk::Application::get();
    _pScene = pApp->accessCurrentScene();
    // Make sure cam is valid (exists and has transform component)
    if (!_pScene->getComponent(_camEntity, pk::ComponentType::PK_TRANSFORM))
    {
        pk::Debug::log(
            "@CameraController::CameraController "
            "Failed to find camera entity's transform component",
            pk::Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }

    pk::InputManager* pInputManager = pApp->accessInputManager();
    pInputManager->addKeyEvent(new CameraKeyEvent(this));
    pInputManager->addMouseButtonEvent(new CameraMouseButtonEvent(this));
    pInputManager->addCursorPosEvent(new CameraCursorPosEvent(this));
}

CameraController::~CameraController()
{}

void CameraController::update()
{
    Transform* pTransform = getCameraTransform();
    mat4& transformationMatrix = pTransform->accessTransformationMatrix();

    float useSpeed = _speed;
    pk::InputManager* pInputManager = Application::get()->accessInputManager();
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_SHIFT))
        useSpeed = _speed * 10.0f;

    if (_moveForward)
        move(pTransform->forward(), transformationMatrix, useSpeed);
    else if (_moveBackwards)
        move(pTransform->forward(), transformationMatrix, -useSpeed);

    if (_moveRight)
        move(pTransform->right(), transformationMatrix, useSpeed);
    else if (_moveLeft)
        move(pTransform->right(), transformationMatrix, -useSpeed);

    if (_moveUp)
        move(pTransform->up(), transformationMatrix, useSpeed);
    else if (_moveDown)
        move(pTransform->up(), transformationMatrix, -useSpeed);
}

pk::Transform* CameraController::getCameraTransform()
{
    // NOTE: danger here..
    return (Transform*)_pScene->getComponent(_camEntity, ComponentType::PK_TRANSFORM);
}

void CameraController::move(const pk::vec3& direction, pk::mat4& targetMatrix, float speed)
{
    targetMatrix[0 + 3 * 4] += direction.x * speed * pk::Timing::get_delta_time();
    targetMatrix[1 + 3 * 4] += direction.y * speed * pk::Timing::get_delta_time();
    targetMatrix[2 + 3 * 4] += direction.z * speed * pk::Timing::get_delta_time();
}
