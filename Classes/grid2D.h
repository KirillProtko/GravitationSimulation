//
// Created by user on 09.12.2025.
//

#ifndef GRAVITATIONSIMULATION_GRID2D_H
#define GRAVITATIONSIMULATION_GRID2D_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "object.h"

class Grid2D {
private:
    int divisions;
    float width;
public:
    GLuint VBO, VAO;
    std::vector<float> vertices;
    int vertexCount;
    Grid2D(int divisions, float width);
    std::vector<float> getVertices(std::vector<Object> activeObjects);

    void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
};


#endif //GRAVITATIONSIMULATION_GRID2D_H