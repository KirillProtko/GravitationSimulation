//
// Created by user on 28.11.2025.
//

#ifndef GRAVITATIONSIMULATION_STAR_H
#define GRAVITATIONSIMULATION_STAR_H


#include "object.h"


class Star : public Object{
private:
    // float luminosity;
    // void SetLuminosity();
public:
    Star(float mass, float dencity, glm::vec3 position, float temperature);
    glm::vec3 GetColor(float temperature);
};


#endif //GRAVITATIONSIMULATION_STAR_H