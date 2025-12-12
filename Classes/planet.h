//
// Created by user on 28.11.2025.
//

#ifndef GRAVITATIONSIMULATION_PLANET_H
#define GRAVITATIONSIMULATION_PLANET_H
#include "object.h"


class Planet : Object {
    public:
    Planet(float mass, float dencity, glm::vec3 position);
};


#endif //GRAVITATIONSIMULATION_PLANET_H