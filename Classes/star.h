//
// Created by user on 28.11.2025.
//

#ifndef GRAVITATIONSIMULATION_STAR_H
#define GRAVITATIONSIMULATION_STAR_H


#include "object.h"


class Star : public Object{
private:
    float temperature;
    float luminosity;
    void SetColor();
    void SetLuminosity();
public:
    explicit Star(float mass, float dencity, glm::vec3 position);
    void SetTemperature(float newTemperature);
};


#endif //GRAVITATIONSIMULATION_STAR_H