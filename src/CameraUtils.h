#pragma once

#include "../PortablePesukarhu/ppk.h"
#include "core/input/InputEvent.h"

#include "ecs/components/Transform.h"



class CameraController
{
private:
    entityID_t _camEntity;
    pk::Scene* _pScene = nullptr;

    float _speed = 10.0f;
    bool _enableMouseLook = false;
    bool _moveForward = false;
    bool _moveBackwards = false;
    bool _moveRight = false;
    bool _moveLeft = false;
    bool _moveUp = false;
    bool _moveDown = false;


    class CameraKeyEvent : public pk::KeyEvent
    {
    private:
        friend class CameraController;
    public:
        CameraController* pController;
        CameraKeyEvent(CameraController* pController);
        virtual void func(pk::InputKeyName key, int scancode, pk::InputAction action, int mods);
    };

    class CameraMouseButtonEvent : public pk::MouseButtonEvent
    {
    private:
        friend class CameraController;
    public:
        CameraController* pController;
        CameraMouseButtonEvent(CameraController* pController);
	virtual void func(pk::InputMouseButtonName button, pk::InputAction action, int mods);
    };

    class CameraCursorPosEvent : public pk::CursorPosEvent
    {
    private:
        friend class CameraController;
        int _prevMouseX = 0;
        int _prevMouseY = 0;
        float _yaw = 0.0f;
        float _pitch = 0.0f;

    public:
        CameraController* pController;
        CameraCursorPosEvent(CameraController* pController);
	virtual void func(int x, int y);
    };

public:
    CameraController(entityID_t cameraEntityID, float speed);
    ~CameraController();
    void update();

    pk::Transform* getCameraTransform();

private:
    void move(const pk::vec3& direction, pk::mat4& targetMatrix, float speed);
};
