//
// Created by user on 08.12.2025.
//

#include "camera.h"

Camera::Camera() : distance(10.0f), angles(45.0f, 20.0f) {
    targetAngles = angles;
}
Camera::Camera(float distance, glm::vec2 angles) {
    this->distance = distance;
    this->angles = angles;
    targetAngles = angles;
}

glm::mat4 Camera::getViewMatrix() {
    this->targetPosition[0] = target.x + sin(glm::radians(this->angles[1])) * cos(glm::radians(this->angles[0])) * this->distance;
    this->targetPosition[1] = target.y + sin(glm::radians(this->angles[0])) * this->distance;
    this->targetPosition[2] = target.z + cos(glm::radians(this->angles[1])) * cos(glm::radians(this->angles[0])) * this->distance;

    glm::mat4 view = glm::lookAt(this->position, this->target, this->up);

    return view;
}
void Camera::scaling(float yoffset) {
    distance -= yoffset * 0.5f;
    if (distance <= minimalDistance) { distance = minimalDistance; }
}
void Camera::rotate(double xpos, double ypos) {
    if (mode == "locked") return;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        SetFirstMouse(false);
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;
    targetAngles[0] += yoffset;
    targetAngles[1] += xoffset;

    if (targetAngles[0] >= 89.9f)
        targetAngles[0] = 89.9f;
    if (targetAngles[0] <= -89.9f)
        targetAngles[0] = -89.9f;
}

void Camera::SetMinimalDistance(float newMinimalDistance) {
    minimalDistance = newMinimalDistance;
}
void Camera::SetSensitivity(float newSensitivity) {
    sensitivity = newSensitivity;
}
void Camera::SetFirstMouse(bool newValue) {
    firstMouse = newValue;
}
void Camera::SetTarget(glm::vec3 newTarget) {
    target = newTarget;
}
void Camera::SetMode(std::string mode) {
    this->mode = mode;
}

void Camera::SetTargetAngles(glm::vec2 target) {
    targetAngles = target;
}
void Camera::SetTargetPosition(glm::vec3 target) {
    targetPosition = target;
}
void Camera::SetTargetTarget(glm::vec3 target) {
    targetTarget = target;
}


void Camera::ExponentionalChangePosition() {
    float alpha = 1.0f - powf(1.0f - 0.05f, 0.1f * 60.0f);
    position = glm::mix(position, targetPosition, alpha);
}
void Camera::ExponentionalChangeAngles() {
    float alpha = 1.0f - powf(1.0f - 0.05f, 0.1f * 60.0f);
    angles = glm::mix(angles, targetAngles, alpha);
}
void Camera::ExponentionalChangeTarget() {
    float alpha = 1.0f - powf(1.0f - 0.05f, 0.1f * 60.0f);
    target = glm::mix(target, targetTarget, alpha);
}

glm::vec3 Camera::GetPosition() {
    return this->position;
}
glm::vec3 Camera::GetTarget() {
    return this->target;
}
glm::vec2 Camera::GetAngles() {
    return angles;
}
glm::vec3 Camera::GetTargetPosition() {
    return targetPosition;
}
glm::vec3 Camera::GetTargetTarget() {
    return targetTarget;
}
glm::vec2 Camera::GetTargetAngles() {
    return targetAngles;
}