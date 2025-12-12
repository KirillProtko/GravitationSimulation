//
// Created by user on 28.11.2025.
//
#include "object.h"

glm::vec3 sphericalToCartesian(float r, float theta, float phi);

Object::Object(float mass, float dencity, glm::vec3 position) {
    this->mass = mass;
    this->dencity = dencity;
    this->radius = getRadius(mass, dencity);
    this->position = position;
}

// Object::~Object() {
//     std::cout << "Object " << this->VAO << " destroyed. Type: " << this->type << std::endl;
//     delete this;
// }

void Object::init() {
    std::cout << "Initializing object at position: "
             << position.x << ", " << position.y << ", " << position.z
             << std::endl;
    std::cout << "Radius: " << radius << std::endl;
    std::cout << "Mass: " << mass << ". Dencity: " << dencity << std::endl;

    std::vector<float> vertices = getVertices();
    this->vertexCount = vertices.size();

    std::cout << "Generated " << vertexCount << " vertices ("
              << vertexCount / 3 << " points, "
              << vertexCount / 9 << " triangles)" << std::endl;

    if (vertices.size() >= 6) {
        std::cout << "First vertex: " << vertices[0] << ", " << vertices[1] << ", " << vertices[2] << std::endl;
        std::cout << "Second vertex: " << vertices[3] << ", " << vertices[4] << ", " << vertices[5] << std::endl;
    }

    createVBOVAO(this->VAO, this->VBO, vertices.data(), this->vertexCount);

    std::cout << "VAO created: " << VAO << ", VBO: " << VBO << std::endl;
    std::cout << std::endl;
    this->Initilized = true;
}

void Object::setMass(float newMass) {
    this->mass = newMass;
    getRadius(newMass, this->dencity);
}

float Object::getRadius(float newMass, float newDencity) {
    return pow((3*newMass/newDencity)/4*glm::pi<float>(), 1.0f/3.0f)/50.0f;
}

void Object::updatePosition() {
    this->position[0] += this->velocity[0];
    this->position[1] += this->velocity[1];
    this->position[2] += this->velocity[2];
}

void Object::accelerateObject(glm::vec3 acceleration) {
    this->velocity[0] += acceleration[0]/10000;
    this->velocity[1] += acceleration[1]/10000;
    this->velocity[2] += acceleration[2]/10000;
}

std::vector<float> Object::getVertices() {
    std::vector<float> vertices;
    float stacks = 10;
    float sectors = 10;

    // generate circumference points using integer steps
    for(float i = 0.0f; i <= stacks; ++i){
        float theta1 = (i / stacks) * glm::pi<float>();
        float theta2 = (i+1) / stacks * glm::pi<float>();
        for (float j = 0.0f; j < sectors; ++j){
            float phi1 = j / sectors * 2 * glm::pi<float>();
            float phi2 = (j+1) / sectors * 2 * glm::pi<float>();
            glm::vec3 v1 = sphericalToCartesian(this->radius, theta1, phi1);
            glm::vec3 v2 = sphericalToCartesian(this->radius, theta1, phi2);
            glm::vec3 v3 = sphericalToCartesian(this->radius, theta2, phi1);
            glm::vec3 v4 = sphericalToCartesian(this->radius, theta2, phi2);

            // Triangle 1: v1-v2-v3
            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z}); //        /|
            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z}); //     /   |
            vertices.insert(vertices.end(), {v3.x, v3.y, v3.z}); //  /__|

            // Triangle 2: v2-v4-v3
            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
            vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
            vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
        }
    }
    return vertices;
}

void Object::updateVertices() {
    std::vector<float> vertices = getVertices();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}


void Object::createVBOVAO(GLuint& vao, GLuint& vbo, const float* vertices, size_t amountOfVertex) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * amountOfVertex, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

bool Object::checkCollision(Object *object2) {
    float dx = object2->position.x - this->position.x;
    float dy = object2->position.y - this->position.y;
    float dz = object2->position.z - this->position.z;
    float distance = sqrt(dx * dx + dy * dy + dz * dz);
    if (distance <= object2->radius + this->radius) {
        std::cout << "Collide!" << std::endl;
        return true;
    }
    return false;
}

glm::vec3 sphericalToCartesian(float r, float theta, float phi){
    float x = r * sin(theta) * cos(phi);
    float y = r * cos(theta);
    float z = r * sin(theta) * sin(phi);
    return glm::vec3 {x, y, z};
}

bool Object::operator==(const Object& other) const {
    return this->VAO == other.VAO;
}
