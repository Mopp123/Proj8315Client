#include "CameraUtils.h"

#include "Pesukarhu/core/Debug.h"

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
    pController->_enableMouseLook = button == pk::InputMouseButtonName::PK_INPUT_MOUSE_MIDDLE && action == pk::InputAction::PK_INPUT_PRESS;
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

        float& camPitch = pController->_pitch;
        float addPitch = mouseDY * sensitivity;
        if (addPitch < 0.0f && camPitch - addPitch <= pController->_maxPitch)
            camPitch -= addPitch;
        if (addPitch > 0.0f && camPitch - addPitch >= pController->_minPitch)
            camPitch -= addPitch;

        pController->_yaw -= mouseDX * sensitivity;
    }
    _prevMouseX = x;
    _prevMouseY = y;
}


CameraController::CameraScrollEvent::CameraScrollEvent(CameraController* pController) :
    pController(pController)
{}

void CameraController::CameraScrollEvent::func(double dx, double dy)
{
    const float scrollSpeed = 0.01f;
    float addZoom = dy * scrollSpeed;
    float& camZoom = pController->_distToPivotPoint;

    if (addZoom < 0.0f && camZoom + addZoom >= pController->_minDistToPivotPoint)
        camZoom += addZoom;
    else if (addZoom > 0.0f && camZoom + addZoom <= pController->_maxDistToPivotPoint)
        camZoom += addZoom;
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
    pInputManager->addScrollEvent(new CameraScrollEvent(this));
}

CameraController::~CameraController()
{}

void CameraController::update()
{
    Transform* pTransform = getCameraTransform();
    mat4& transformationMatrix = pTransform->accessTransformationMatrix();

    /*
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
    */

    // Move pivot point
    float& xPos = _pivotPoint.x;
    float& zPos = _pivotPoint.z;

    float rotatedAngle = _yaw + M_PI * 0.5f;

    if (_moveForward)
    {
        xPos += std::cos(rotatedAngle) * _speed * Timing::get_delta_time();
        zPos += -std::sin(rotatedAngle) * _speed * Timing::get_delta_time();
    }
    else if (_moveBackwards)
    {
        xPos += std::cos(rotatedAngle) * -_speed * Timing::get_delta_time();
        zPos += -std::sin(rotatedAngle) * -_speed * Timing::get_delta_time();
    }

    if (_moveLeft)
    {
        xPos += std::cos(_yaw) * -_speed * Timing::get_delta_time();
        zPos += -std::sin(_yaw) * -_speed * Timing::get_delta_time();
    }
    else if (_moveRight)
    {
        xPos += std::cos(_yaw) * _speed * Timing::get_delta_time();
        zPos += -std::sin(_yaw) * _speed * Timing::get_delta_time();
    }

    // calc cam transform according to pivot point and angles
    float horizontalDist = std::cos(_pitch) * _distToPivotPoint;
    float verticalDist = std::sin(_pitch) * _distToPivotPoint;

    mat4 translationMat;
    translationMat.setIdentity();
    translationMat[0 + 3 * 4] = _pivotPoint.x + std::sin(_yaw) * horizontalDist;
    translationMat[1 + 3 * 4] = _pivotPoint.y + verticalDist;
    translationMat[2 + 3 * 4] = _pivotPoint.z + std::cos(_yaw) * horizontalDist;

    mat4 rotMatYaw;
    rotMatYaw.setIdentity();
    rotMatYaw[0 + 0 * 4] =	std::cos(_yaw);
    rotMatYaw[0 + 2 * 4] =	std::sin(_yaw);
    rotMatYaw[2 + 0 * 4] = -std::sin(_yaw);
    rotMatYaw[2 + 2 * 4] =	std::cos(_yaw);

    mat4 rotMatPitch;
    rotMatPitch.setIdentity();
    rotMatPitch[1 + 1 * 4] =  std::cos(-_pitch);
    rotMatPitch[1 + 2 * 4] = -std::sin(-_pitch);
    rotMatPitch[2 + 1 * 4] =  std::sin(-_pitch);
    rotMatPitch[2 + 2 * 4] =  std::cos(-_pitch);

    transformationMatrix = translationMat * rotMatYaw * rotMatPitch;
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
