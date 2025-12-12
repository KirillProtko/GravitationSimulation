//
// Created by user on 02.12.2025.
//

#ifndef GRAVITATIONSIMULATION_BLACKHOLE_H
#define GRAVITATIONSIMULATION_BLACKHOLE_H
#include "object.h"


class blackHole : Object{
    public:
    blackHole(float mass, float dencity, glm::vec3 position);
};


#endif //GRAVITATIONSIMULATION_BLACKHOLE_H