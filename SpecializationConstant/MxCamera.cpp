#include "MxCamera.h"

namespace Mix {
    Camera::Camera() :
        mTransform(),
        mMoveSpeed(1.0f),
        mMouseSensitivity(1.0f),
        mFovy(45.0f),
        mAspect(1.0f),
        mNear(0.1f),
        mFar(100.0f) {
    }

    Camera::Camera(const float fovy, const float aspect, const float near, const float far) :
        mFovy(fovy),
        mAspect(aspect),
        mNear(near),
        mFar(far) {
    }

    glm::mat4 Camera::getViewMatrix() const {
        return glm::lookAt(mTransform.getPosition(),
                           mTransform.getPosition() + mTransform.getForward(),
                           mTransform.getUp());
    }

    void Camera::moveCamera(const float deltaTime, glm::vec3 direction, bool normalized) {
        if (!normalized)
            direction = glm::normalize(direction);

        mTransform.setPosition(mTransform.getPosition() + direction * deltaTime * mMoveSpeed);
    }

    void Camera::moveCamera(const float deltaTime, const CameraDirection direction) {
        glm::vec3 dir;

        switch (direction) {
        case Mix::Camera::FORWARD:
            dir = mTransform.getForward();
            break;
        case Mix::Camera::BACK:
            dir = -mTransform.getForward();
            break;
        case Mix::Camera::LEFT:
            dir = -mTransform.getRight();
            break;
        case Mix::Camera::RIGHT:
            dir = mTransform.getRight();
            break;
        case Mix::Camera::UP:
            dir = mTransform.getUp();
            break;
        case Mix::Camera::DOWN:
            dir = -mTransform.getUp();
            break;
        case Mix::Camera::WORLD_UP:
            dir = Axis::initZ;
            break;
        case Mix::Camera::WORLD_DOWN:
            dir = -Axis::initZ;
            break;
        }

        moveCamera(deltaTime, dir);
    }

    void Camera::rotateCamera(const float yawOffset, const float pitchOffset, const float rollOffset) {
        mTransform.rotate(rollOffset, pitchOffset, yawOffset);
    }

    void Camera::setPosition(const glm::vec3& pos) {
        mTransform.setPosition(pos);
    }

    void Camera::setMoveSpeed(const float speed) {
        if (speed < 0.0f)
            return;
        mMoveSpeed = speed;
    }

    void Camera::lookAt(const glm::vec3& target) {
        mTransform.lookAt(target);
    }

    void Camera::lookAt(const float x, const float y, const float z) {
        lookAt(glm::vec3(x, y, z));
    }

    void Camera::setMouseSensitivity(const float sensitivity) {
        if (sensitivity < 0.0f)
            return;
        mMouseSensitivity = sensitivity;
    }

    void Camera::setPerspective(const float fovy, const float aspect, const float near, const float far) {
        mFovy = fovy;
        mAspect = aspect;
        mNear = near;
        mFar = far;
    }

    glm::mat4 Camera::getProjMat() const {
        return glm::perspective(mFovy, mAspect, mNear, mFar);
    }

}
