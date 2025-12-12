//
// Created by user on 28.11.2025.
//

#include "star.h"


Star::Star(float mass, float dencity, glm::vec3 position) : Object(mass, dencity, position) {
    this->type = "star";
    SetLuminosity();
}

void Star::SetColor() {

}

void Star::SetLuminosity() {
    float sunMass = 1.98847e30;
    float sunLuminosity = 3.827e26;
    if (this->mass < 0.43f*sunMass) {
        luminosity = 0.23f * sunLuminosity* pow((this->mass/sunMass), 2.3f);
    }
    else if (this->mass >= 0.43f*sunMass && this->mass < 2.0f*sunMass) {
        luminosity = sunLuminosity* pow((this->mass/sunMass), 3.9f);
    }
    else if (this->mass >= 2.0f*sunMass && this->mass < 20.0f*sunMass) {
        luminosity = 1.5f * sunLuminosity* pow((this->mass/sunMass), 3.5f);
    }
    else if (this->mass >= 20.0f*sunMass) {
        luminosity = 32000.0f * sunLuminosity* (this->mass/sunMass);
    }
}

void Star::SetTemperature(float newTemperature) {
    this->temperature = newTemperature;
    SetColor();
}