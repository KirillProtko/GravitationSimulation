//
// Created by user on 09.12.2025.
//

#include "grid2D.h"

Grid2D::Grid2D(int divisions, float width) {
    this->divisions = divisions;
    this->width = width/10;
}

void Grid2D::setDivisions(int newDivisions) {
    this->divisions = newDivisions;
}

void Grid2D::setWidth(float newWidth) {
    this->width = newWidth;
}

void Grid2D::setColor(glm::vec3 newColor) {
    this->color = newColor;
}

std::vector<float> Grid2D::getVertices(std::vector<Object> activeObjects) {
    // this code is bullshit
    std::vector<float> vertices;
    float step = width / divisions;
    float half = divisions / 2.0f;
    float ratio = 0.25e-20f;
    float x1, x2, y1 , y2, y3, y4, z1, z2;
    // 4 points
    for (int i = -half; i < half; i++) {
        x1 = i * step;
        x2 = x1 + step;
        for (int j = -half; j < half; j++) {
            z1 = j * step;
            z2 = z1 + step;
            y1 = 0.0f; y2 = 0.0f; y3 = 0.0f; y4 = 0.0f;
            for (auto& object : activeObjects) {
                    float dx1 = x1 - object.position.x;
                    float dx2 = x2 - object.position.x;
                    float dz1 = z1 - object.position.z;
                    float dz2 = z2 - object.position.z;
                    float distance1 = sqrt(dx1 * dx1 + dz1 * dz1);
                    float distance2 = sqrt(dx1 * dx1 + dz2 * dz2);
                    float distance3 = sqrt(dx2 * dx2 + dz2 * dz2);
                    float distance4 = sqrt(dx2 * dx2 + dz1 * dz1);
                    float force1 = object.mass / (distance1 * distance1 * distance1 + 0.1f);
                    float force2 = object.mass / (distance2 * distance2 * distance2 + 0.1f);
                    float force3 = object.mass / (distance3 * distance3 * distance3 + 0.1f);
                    float force4 = object.mass / (distance4 * distance4 * distance4 + 0.1f);

                    y1 -= force1 * ratio;
                    y2 -= force2 * ratio;
                    y3 -= force3 * ratio;
                    y4 -= force4 * ratio;
            }
            glm::vec3 v1(x1, y1, z1);
            glm::vec3 v2(x1, y2, z2);
            glm::vec3 v3(x2, y3, z2);
            glm::vec3 v4(x2, y4, z1);

            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z});
            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
            vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
            vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z});
            vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
        }
    }
    return vertices;
}

void Grid2D::createVBOVAO(GLuint& vao, GLuint& vbo, const float* vertices, size_t amountOfVertex) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * amountOfVertex, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}