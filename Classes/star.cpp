//
// Created by user on 28.11.2025.
//

#include "star.h"


Star::Star(float mass, float dencity, glm::vec3 position, float temperature) : Object(mass, dencity, position) {
    this->type = "star";
    this->IsLightSource = true;
    this->objectColor = GetColor(temperature);
    // SetLuminosity();
}

Star::Star(float mass, float dencity, glm::vec3 position, glm::vec3 initVelocity,float temperature) : Object(mass, dencity, position, initVelocity) {
    this->type = "star";
    this->IsLightSource = true;
    this->objectColor = GetColor(temperature);
    // SetLuminosity();
}

glm::vec3 Star::GetColor(float temperature) {
    if (temperature <= 2500.0f)
        return {1.0f, 0.3f, 0.1f};  // Dark red
    else if (temperature <= 3500.0f)
        return {1.0f, 0.3f, 0.2f};  // Red
    else if (temperature <= 4500.0f)
        return {1.0f, 0.7f, 0.3f};  // Orange
    else if (temperature <= 5500.0f)
        return {1.0f, 0.8f, 0.2f};  // Yellow-orange
    else if (temperature <= 6500.0f)
        return {1.0f, 1.0f, 0.0f};  // Yellow
    else if (temperature <= 8000.0f)
        return {0.9f, 0.95f, 0.7f}; // White-yellow
    else if (temperature <= 10000.0f)
        return {1.0f, 1.0f, 1.0f};  // White
    else if (temperature <= 15000.0f)
        return {0.7f, 0.8f, 1.0f};  // Blue-white
     else if (temperature <= 25000.0f)
        return {0.6f, 0.7f, 1.0f};  // Light blue
    else
        return {0.4f, 0.6f, 1.0f}; // Blue
}

// void Star::SetLuminosity() {
//     float sunMass = 1.98847e30;
//     float sunLuminosity = 3.827e26;
//     if (this->mass < 0.43f*sunMass) {
//         luminosity = 0.23f * sunLuminosity* pow((this->mass/sunMass), 2.3f);
//     }
//     else if (this->mass >= 0.43f*sunMass && this->mass < 2.0f*sunMass) {
//         luminosity = sunLuminosity* pow((this->mass/sunMass), 3.9f);
//     }
//     else if (this->mass >= 2.0f*sunMass && this->mass < 20.0f*sunMass) {
//         luminosity = 1.5f * sunLuminosity* pow((this->mass/sunMass), 3.5f);
//     }
//     else if (this->mass >= 20.0f*sunMass) {
//         luminosity = 32000.0f * sunLuminosity* (this->mass/sunMass);
//     }
// }