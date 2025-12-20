//
// Created by user on 28.11.2025.
//
#include "object.h"

glm::vec3 sphericalToCartesian(float r, float theta, float phi);

Object::Object() : mass(0), density(0), radius(0), position(0), velocity(0),
           VAO(0), VBO(0), TrailVAO(0), TrailVBO(0), vertexCount(0),
           IsLightSource(false), Active(true), Initialized(false),
           type("Object"), objectColor(0.5f, 0.5f, 0.5f) {}
Object::Object(float mass, float density, glm::vec3 position) {
    this->mass = mass;
    this->density = density;
    this->radius = getRadius(mass, density);
    this->position = position;
}
Object::Object(float mass, float density, glm::vec3 position, glm::vec3 initVelocity) {
    this->mass = mass;
    this->density = density;
    this->radius = getRadius(mass, density);
    this->position = position;
    this->velocity = initVelocity;
}

// Object::~Object() {
//     std::cout << "Object " << this->VAO << " destroyed. Type: " << this->type << std::endl;
//     delete this;
// }

void Object::init() {
    // std::cout << "Initializing object at position: "
    //          << position.x << ", " << position.y << ", " << position.z
    //          << std::endl;
    // std::cout << "Radius: " << radius << std::endl;
    // std::cout << "Mass: " << mass << ". Dencity: " << density << std::endl;

    std::vector<float> vertices = getVertices();
    this->vertexCount = vertices.size();

    // std::cout << "Generated " << vertexCount << " vertices ("
    //           << vertexCount / 3 << " points, "
    //           << vertexCount / 9 << " triangles)" << std::endl;

    createVBOVAO(this->VAO, this->VBO, vertices.data(), this->vertexCount);

    // std::cout << "VAO created: " << VAO << ", VBO: " << VBO << std::endl;
    vertices = {0.0f, 0.0f, 0.0f, 0.0f};
    createTrailVBOVAO(this->TrailVAO, this->TrailVBO, vertices.data(), vertices.size());
    this->Initialized = true;
}

void Object::setMass(float newMass) {
    this->mass = newMass;
    this->radius = getRadius(newMass, this->density);
}

float Object::getRadius(float newMass, float newDencity) {
    return std::pow((3.0f * newMass) / (newDencity * 4.0f * glm::pi<float>()), 1.0f/3.0f) / 2000000.0f;
}

void Object::updatePosition() {
    this->position[0] += this->velocity[0]/100;
    this->position[1] += this->velocity[1]/100;
    this->position[2] += this->velocity[2]/100;
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

            glm::vec3 v1Norm = glm::normalize(v1);
            glm::vec3 v2Norm = glm::normalize(v2);
            glm::vec3 v3Norm = glm::normalize(v3);
            glm::vec3 v4Norm = glm::normalize(v4);

            // Triangle 1: v1-v2-v3
            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z});
            vertices.insert(vertices.end(), {v1Norm.x, v1Norm.y, v1Norm.z});

            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
            vertices.insert(vertices.end(), {v2Norm.x, v2Norm.y, v2Norm.z});

            vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
            vertices.insert(vertices.end(), {v3Norm.x, v3Norm.y, v3Norm.z});


            // Triangle 2: v2-v4-v3
            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
            vertices.insert(vertices.end(), {v2Norm.x, v2Norm.y, v2Norm.z});

            vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
            vertices.insert(vertices.end(), {v4Norm.x, v4Norm.y, v4Norm.z});

            vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
            vertices.insert(vertices.end(), {v3Norm.x, v3Norm.y, v3Norm.z});
        }
    }
    return vertices;
}

void Object::updateVertices() {
    std::vector<float> vertices = getVertices();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}

void Object::updateTrail() {
    if (this->trailVertices.size() >= 8000) {
        this->trailVertices.erase(this->trailVertices.begin());
        this->trailVertices.erase(this->trailVertices.begin());
        this->trailVertices.erase(this->trailVertices.begin());
        this->trailVertices.erase(this->trailVertices.begin());
    }
    this->trailVertices.emplace_back(this->position.x);
    this->trailVertices.emplace_back(this->position.y);
    this->trailVertices.emplace_back(this->position.z);
    this->trailVertices.emplace_back(0.5f);

    glBindBuffer(GL_ARRAY_BUFFER, TrailVBO);
    glBufferData(GL_ARRAY_BUFFER, this->trailVertices.size() * sizeof(float), this->trailVertices.data(), GL_STATIC_DRAW);
}

void Object::createVBOVAO(GLuint& vao, GLuint& vbo, const float* vertices, size_t vertexCount) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Object::createTrailVBOVAO(GLuint& TrailVAO, GLuint& TrailVBO, const float* vertices, size_t vertexCount) {
    glGenVertexArrays(1, &TrailVAO);
    glGenBuffers(1, &TrailVBO);

    glBindVertexArray(TrailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TrailVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

bool Object::checkCollision(Object *object2) const {
    float dx = object2->position.x - this->position.x;
    float dy = object2->position.y - this->position.y;
    float dz = object2->position.z - this->position.z;
    float distance = sqrt(dx * dx + dy * dy + dz * dz);
    if (distance <= object2->radius + this->radius) {
        std::cout << "Collide "<< this->VAO << " with "<< object2->VAO << std::endl;
        return true;
    }
    return false;
}

void Object::calculateMass() {
    this->mass = (pow(this->radius * 2000000.0f, 3.0f) * (4.0f *glm::pi<float>()) * this->density) / 3.0f;
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
