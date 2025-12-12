//
// Created by user on 28.11.2025.
//

#include "planet.h"

Planet::Planet(float mass, float dencity, glm::vec3 position) : Object(mass, dencity, position){
    this->type = "Planet";
}
