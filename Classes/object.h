//
// Created by user on 28.11.2025.
//

#ifndef GRAVITATIONSIMULATION_OBJECT_H
#define GRAVITATIONSIMULATION_OBJECT_H
#include <iostream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Object {  //random object with user params
public:
        GLuint VBO, VAO;

        glm::vec3 position, objectColor = {1.0f, 0.5f, 0.25f}, velocity = {0.0f, 0.0f, 0.0f};
        size_t vertexCount;

        float mass, dencity, radius;
        std::string type = "Object";

        bool Initilized = false, Active = true, Selected = false, IsLightSource = false;

        explicit Object(float mass, float dencity, glm::vec3 position);
        // ~Object();
        void init();

        void setMass(float newMass);
        static float getRadius(float newMass, float newDencity); //calculating new sizes
        void updatePosition(); // updating position in space adding object velocity
        void accelerateObject(glm::vec3 acceleration); // increasing velocity by acceleration giving from G-force
        void updateVertices();
        std::vector<float> getVertices(); //getting vertices for drawing spheres
        bool checkCollision(Object *object2) const;

        void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);

        bool operator==(const Object& other) const;
};


#endif //GRAVITATIONSIMULATION_OBJECT_H