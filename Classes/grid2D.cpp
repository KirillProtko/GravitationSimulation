//
// Created by user on 09.12.2025.
//

#include "grid2D.h"

Grid2D::Grid2D(int divisions, float width) {
    this->divisions = divisions;
    this->width = width/10;
    // this->vertices = getVertices();
    // this->vertexCount = vertices.size();

    // createVBOVAO(this->VAO, this->VBO, vertices.data(), this->vertexCount);
}
std::vector<float> Grid2D::getVertices(std::vector<Object> activeObjects) {
    std::vector<float> vertices;
    float step = width / divisions;
    float half = divisions / 2.0f;
    float x1, x2, y1 , y2, y3, y4, z1, z2;
    // 4 points
    for (int i = -half; i < half; i++) {
        x1 = i * step;
        x2 = x1 + step;
        for (int j = -half; j < half; j++) {
            z1 = j * step;
            z2 = z1 + step;
            y1 = -2.5f; y2 = -2.5f; y3 = -2.5f; y4 = -2.5f;
            for (auto& object : activeObjects) {
                    float dx1 = x1 - object.position.x;
                    float dx2 = x2 - object.position.x;
                    float dz1 = z1 - object.position.z;
                    float dz2 = z2 - object.position.z;
                    float distance1 = sqrt(dx1 * dx1 + dz1 * dz1);
                    float distance2 = sqrt(dx1 * dx1 + dz2 * dz2);
                    float distance3 = sqrt(dx2 * dx2 + dz2 * dz2);
                    float distance4 = sqrt(dx2 * dx2 + dz1 * dz1);
                    y1 += distance1 * object.mass / 10000;
                    y2 += distance2 * object.mass / 10000;
                    y3 += distance3 * object.mass / 10000;
                    y4 += distance4 * object.mass / 10000;
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
    // //xyz
    // glm::vec3 pos;
    // for (int i = -half; i < half; i++) {
    //     x1 = i * step;
    //     x2 = x1 + step;
    //     for (int j = -half; j < half; j++) {
    //         z1 = j * step;
    //         z2 = z1 + step;
    //
    //         for (auto& object : activeObjects) {
    //             for (int k = 0; k < 4; k++) {
    //
    //             }
    //             pos = {x1,y1,z1};
    //             float dx = x1 - object.position.x;
    //             float dy = 0 - object.position.y;
    //             float dz = z1 - object.position.z;
    //             float distance = sqrt(dx * dx + dy * dy + dz * dz);
    //             y2 -= distance * object.mass / 100;
    //         }
    //     }
    // }
    //
    // //x
    // for (int i = -half; i < half; i++) {
    //     x1 = i * step;
    //     x2 = x1 + step;
    //     for (int j = -half; j < half; j++) {
    //         z1 = j * step;
    //         verticesX.push_back(x1);
    //         // vertices.push_back(y1);
    //         // vertices.push_back(z1);
    //
    //         verticesX.push_back(x2);
    //         // vertices.push_back(y1);
    //         // vertices.push_back(z1);
    //     }
    // }
    // //y
    // glm::vec3 pos;
    // for (auto& object : activeObjects) {
    //     pos = {x1,y1,z1};
    //     float dx = x1 - object.position.x;
    //     float dy = y1 - object.position.y;
    //     float dz = z1 - object.position.z;
    //     float distance = sqrt(dx * dx + dy * dy + dz * dz);
    //     y2 -= distance * object.mass / 100;
    // }
    // //z
    // for (int i = -half; i < half; i++) {
    //     x1 = i * step;
    //     for (int j = -half; j < half; j++) {
    //         z1 = j * step;
    //         z2 = z1 + step;
    //         vertices.push_back(x1);
    //         vertices.push_back(y1);
    //         vertices.push_back(z1);
    //
    //         vertices.push_back(x1);
    //         vertices.push_back(y1);
    //         vertices.push_back(z2);
    //     }
    // }
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