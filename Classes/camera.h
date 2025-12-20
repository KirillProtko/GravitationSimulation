//
// Created by user on 08.12.2025.
//

#ifndef GRAVITATIONSIMULATION_CAMERA_H
#define GRAVITATIONSIMULATION_CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Camera {
private:
    glm::mat4 lookAt;
    glm::vec3 position;
    glm::vec3 target = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec2 angles;

    glm::vec3 targetPosition;
    glm::vec3 targetTarget;
    glm::vec2 targetAngles;

    bool firstMouse = true;
    std::string mode = "centered";

    float distance;
    float minimalDistance = 2.5f;
    float sensitivity = 0.1f;
    float defaultRotatrionSpeed = 0.2f;

    float lastX;
    float lastY;
public:
    Camera();
    Camera(float distance, glm::vec2 angles);

    glm::mat4 getViewMatrix(); // updating cameras position and angle and returning view matrix
    void scaling(float yoffset);
    void rotate(double xpos, double ypos); // if camera targeted on center

    void SetMinimalDistance(float newMinimalDistance);
    void SetSensitivity(float newSensitivity);
    void SetFirstMouse(bool newValue);
    void SetTarget(glm::vec3 newTarget);
    void SetMode(std::string mode);
    void SetTargetAngles(glm::vec2 target);
    void SetTargetPosition(glm::vec3 target);
    void SetTargetTarget(glm::vec3 target);

    void ExponentionalChangePosition();
    void ExponentionalChangeAngles();
    void ExponentionalChangeTarget();

    glm::vec3 GetPosition();
    glm::vec3 GetTarget();
    glm::vec2 GetAngles();
    glm::vec3 GetTargetPosition();
    glm::vec3 GetTargetTarget();
    glm::vec2 GetTargetAngles();

};


#endif //GRAVITATIONSIMULATION_CAMERA_H