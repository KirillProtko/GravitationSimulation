//
// Created by user on 08.12.2025.
//

#include "camera.h"

Camera::Camera() : distance(10.0f), angleX(45.0f), angleY(20.0f) {
}

Camera::Camera(float distance, float angleX, float angleY) {
    this->distance = distance;
    this->angleX = angleX;
    this->angleY = angleY;
}

glm::mat4 Camera::update() {
    this->position[0] = sin(glm::radians(this->angleY)) * cos(glm::radians(this->angleX)) * this->distance;
    this->position[1] = sin(glm::radians(this->angleX)) * this->distance;
    this->position[2] = cos(glm::radians(this->angleY)) * cos(glm::radians(this->angleX)) * this->distance;

    glm::mat4 view = glm::lookAt(this->position, this->target, this->up);

    return view;
}

void Camera::scaling(float yoffset) {
    distance -= yoffset * 0.5f;
    if (distance <= minimalDistance) { distance = minimalDistance; }
}

void Camera::rotate(double xpos, double ypos) {
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
    angleX += yoffset;
    angleY += xoffset;

    if (angleX >= 89.0f)
        angleX = 89.0f;
    if (angleX <= -89.0f)
        angleX = -89.0f;
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

glm::vec3 Camera::GetPosition() {
    return this->position;
}