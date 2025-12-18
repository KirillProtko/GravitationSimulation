//
// Created by user on 28.11.2025.
//

#ifndef GRAVITATIONSIMULATION_PLANET_H
#define GRAVITATIONSIMULATION_PLANET_H
#include "object.h"


class Planet : public Object {
    public:
    Planet(float mass, float dencity, glm::vec3 position);
    Planet(float mass, float dencity, glm::vec3 position, glm::vec3 initVelocity);
};


#endif //GRAVITATIONSIMULATION_PLANET_H