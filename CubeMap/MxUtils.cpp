#include "MxUtils.h"

namespace Mix {
    const glm::vec3 Axis::initX = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 Axis::initY = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 Axis::initZ = glm::vec3(0.0f, 0.0f, 1.0f);

    Axis::Axis() :
        x(initX),
        y(initY),
        z(initZ) {
    }

    void Axis::applyRotate(const glm::mat4 & matrix) {
        glm::mat3 mat = matrix;
        x = mat * x;
        y = mat * y;
        z = mat * z;
    }

    void Axis::applyRotate(const glm::quat& quat) {
        x = quat * x;
        y = quat * y;
        z = quat * z;
    }

    void Axis::rotateFromInit(const glm::mat4 & matrix) {
        glm::mat3 mat = matrix;
        x = mat * initX;
        y = mat * initY;
        z = mat * initZ;
    }

    void Axis::rotateFromInit(const glm::quat & quat) {
        x = quat * initX;
        y = quat * initY;
        z = quat * initZ;
    }

    void Transform::quaternionUpdated() {
        mAxis.rotateFromInit(mQuaternion);
    }

    Transform::Transform() :
        mPosition(0.0f, 0.0f, 0.0f),
        mScale(1.0f, 1.0f, 1.0f),
        mAxis(),
        mQuaternion(1.0f, 0.0f, 0.0f, 0.0f) {
    }

    glm::vec3 Transform::getEulerAngles() const {
        // to avoid negative zero
        return glm::eulerAngles(mQuaternion) + glm::vec3(0.0f, 0.0f, 0.0f);
    }

    void Transform::translate(const glm::vec3 & translation, const Space relativeTo) {
        switch (relativeTo) {
        case Mix::Space::World:
            mPosition += translation;
            break;
        case Mix::Space::Self:
            mPosition += (translation.x*mAxis.x + translation.y*mAxis.y + translation.z*mAxis.z);
            break;
        }
    }

    void Transform::translate(const float x, const float y, const float z, const Space relativeTo) {
        translate(glm::vec3(x, y, z), relativeTo);
    }

    void Transform::translate(const glm::vec3 & translation, const Transform & relativeTo) {
        mPosition = mPosition +
            translation.x * relativeTo.getAxis().x +
            translation.y * relativeTo.getAxis().y +
            translation.z * relativeTo.getAxis().z;
    }

    void Transform::translate(const float x, const float y, const float z, const Transform & relativeTo) {
        translate(glm::vec3(x, y, z), relativeTo);
    }

    void Transform::rotate(const glm::vec3 & eulers, const Space relativeTo) {
        switch (relativeTo) {
        case Mix::Space::World:
            mQuaternion = glm::tquat(eulers) * mQuaternion;
            break;
        case Mix::Space::Self:
            mQuaternion = glm::tquat(glm::eulerAngles(mQuaternion) + eulers);
            break;
        }
        quaternionUpdated();
    }

    void Transform::rotateAround(const glm::vec3 & point, const glm::vec3 & axis, const float angle) {
        glm::vec3 relativePos = mPosition - point;
        glm::mat4 mat = glm::rotate(glm::mat4(1.0f), angle, axis);
        relativePos = mat * glm::vec4(relativePos, 1.0f);
        mPosition = relativePos + point;
    }

    void Transform::scale(const glm::vec3 & scale, const Space relativeTo) {
        switch (relativeTo) {
        case Mix::Space::World:
            mScale = scale;
            break;
        case Mix::Space::Self:
            mScale *= scale;
            break;
        }
    }

    void Transform::lookAt(const Transform & target, const glm::vec3 & worldUp) {
        lookAt(target.getPosition(), worldUp);
    }

    // todo
    void Transform::lookAt(const glm::vec3 & worldPosition, const glm::vec3 & worldUp) {
        // if worldPosition is equal to camera position, do nothing
        auto equal = glm::epsilonEqual(worldPosition, mPosition, MX_EPSILON);
        if (equal.x && equal.y && equal.z)
            return;

        mAxis.x = glm::normalize(worldPosition - mPosition);
        mAxis.y = glm::normalize(glm::cross(worldUp, mAxis.x));
        mAxis.z = glm::normalize(glm::cross(mAxis.x, mAxis.y));

        mQuaternion = rotateBetweenVec(Axis::initX, mAxis.x);

        // rotate when mAxis.y is not perpendicular to worldUp
        glm::vec3 newY = mQuaternion * Axis::initY;
        float theta = glm::dot(worldUp, newY);
        if (glm::epsilonNotEqual(theta, 0.0f, MX_EPSILON)) {
            mQuaternion = rotateBetweenVec(newY, mAxis.y) * mQuaternion;
        }

        // rotate when cos<worldUp, mAxis.z> <= 0.0f
        glm::vec3 newZ = mQuaternion * Axis::initZ;
        theta = glm::dot(worldUp, newZ);
        if (theta <= 0.0f) {
            mQuaternion = glm::angleAxis(glm::radians(180.0f), mAxis.x) * mQuaternion;
        }
    }

    glm::quat rotateBetweenVec(const glm::vec3 & from, const glm::vec3 & to) {
        float cosTheta = glm::dot(from, to);
        glm::vec3 rotationAxis;

        if (cosTheta > 1.0f - MX_EPSILON)
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        else if (cosTheta < -1.0f + MX_EPSILON) {
            rotationAxis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), from);
            if (glm::length(rotationAxis) < MX_EPSILON) // they were parallel
                rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), from);

            rotationAxis = glm::normalize(rotationAxis);
            return glm::angleAxis(glm::radians(180.0f), rotationAxis);
        }

        rotationAxis = glm::normalize(glm::cross(from, to));

        return glm::angleAxis(glm::acos(cosTheta), rotationAxis);
    }

}