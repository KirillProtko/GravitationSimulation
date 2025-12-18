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

    bool firstMouse = true;
    std::string mode = "centered";

    float distance;
    float minimalDistance = 2.5f;
    float angleY;
    float angleX;
    float sensitivity = 0.1f;
    float defaultRotatrionSpeed = 0.2f;

    float lastX;
    float lastY;
public:
    Camera();
    Camera(float distance, float angleX, float angleY);

    glm::mat4 update(); // updating cameras position and angle and returning view matrix
    void scaling(float yoffset);
    void rotate(double xpos, double ypos); // if camera targeted on center

    void SetMinimalDistance(float newMinimalDistance);
    void SetSensitivity(float newSensitivity);
    void SetFirstMouse(bool newValue);
    void SetTarget(glm::vec3 newTarget);
    void SetMode(std::string mode);

    glm::vec3 GetPosition();

};


#endif //GRAVITATIONSIMULATION_CAMERA_H