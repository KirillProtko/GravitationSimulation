//
// Created by user on 28.11.2025.
//

#ifndef GRAVITATIONSIMULATION_STAR_H
#define GRAVITATIONSIMULATION_STAR_H


#include "object.h"
#include <stdio.h>


class Star : public Object{
private:
    float temperature;
    float luminosity;
    void SetLuminosity();
public:
    Star(float mass, float dencity, glm::vec3 position, float temperature);
    void SetTemperature(float newTemperature);
    glm::vec3 GetColor();
};


#endif //GRAVITATIONSIMULATION_STAR_H