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

    public:
        CameraController* pController;
        CameraCursorPosEvent(CameraController* pController);
	    virtual void func(int x, int y);
    };

    class CameraScrollEvent : public pk::ScrollEvent
    {
    private:
        friend class CameraController;

    public:
        CameraController* pController;
        CameraScrollEvent(CameraController* pController);
	    virtual void func(double dx, double dy);
    };

    pk::vec3 _pivotPoint;
    float _distToPivotPoint = 20.0f;
    const float _minDistToPivotPoint = 2.0f;
    const float _maxDistToPivotPoint = 100.0f;
    float _yaw = 0.0f;
    const float _minPitch = M_PI * 0.1f;
    const float _maxPitch = M_PI * 0.5f;
    float _pitch = _maxPitch * 0.5f;

public:
    CameraController(entityID_t cameraEntityID, float speed);
    ~CameraController();
    void update();

    pk::Transform* getCameraTransform();

    inline void setPivotPoint(const pk::vec3& pos) { _pivotPoint = pos; }
    inline void setPivotPointHeight(float height) { _pivotPoint.y = height;}
    inline const pk::vec3& getPivotPoint() const { return _pivotPoint; }

private:
    void move(const pk::vec3& direction, pk::mat4& targetMatrix, float speed);
};
