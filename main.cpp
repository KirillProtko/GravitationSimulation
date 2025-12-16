#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h> // yea i know there is a .dll file, but for now i will include it like that, bruh
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <random>
//          files

#include <algorithm>
#include <memory>

#include "Classes/object.h"
#include "Classes/star.h"

#include "Classes/camera.h"
#include "Classes/grid2D.h"

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(-20,20);

//
// SHADERS
//
const char* vertexDefaultShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec3 Normal;
out vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = vec3( model * vec4(aPos, 1.0));
})glsl";
// i don't care about light color, so by default it will be just vec3(1.0f)
const char* fragmentDefaultShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 objectColor;

struct Light {
    vec3 position;

    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};
#define MAX_LIGHTS 32

uniform int lengthOfArray;
uniform Light lightArray[MAX_LIGHTS];

vec3 CalculateLight(Light light, vec3 normal, vec3 fragPos) {
    vec3 lightDir = normalize(light.position - fragPos);

    vec3 ambient = light.ambient;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;

    return (ambient + diffuse);
}

void main() {

    vec3 norm = normalize(Normal);

    vec3 result =  vec3(0.0);
    for (int i = 0; i < lengthOfArray; i++) {
        result += CalculateLight(lightArray[i], norm, FragPos);
    }
    FragColor = vec4(objectColor * result, 1.0f);

})glsl";

// light
const char* vertexLightShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})glsl";

const char* fragmentLightShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec3 objectColor;
void main() {
    FragColor = vec4(objectColor, 1.0f);
})glsl";
//
// CAMERA
//
Camera camera = Camera(); //        watch camera.h and camera.cpp

//
// MAIN FUNCTIONS (DECLARATION)
//
void processInput(GLFWwindow *window);
void updateState();
void updateCamera();
void addObject();
void inactivateObject(Object& object);
void deleteObject();
void initializeObjects();
void togglePaths();
GLuint createShaders(const char* vertexShaderSource, const char* fragmentShaderSource);
void setLightSource(Object& object, int index, GLuint defaultShaderProgram);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xPos, double yPos);

void setScenario(int keyNumber);

//
// GLOBAL VARIABLES
//
bool pause = false, mousePressed = false;
bool showPaths = true; // turn on/off paths of objects orbits
int chaseObject = -1;
//          constant variables
constexpr double G = 6.6743e-11;

float windowWidth, windowHeight;


//          vectors of active Objects (star, planets, black holes, etc.), which must be on screen
std::vector<Object> activeObjects = {
    static_cast<Object>(Star(2.34e22, 1000, glm::vec3{0.0f, 0.0f, 0.0f}, 5772.0f)),
    // static_cast<Object>((Star(200000, 1000, glm::vec3{0.0f, 0.0f, -5.0f}, 6800.0f))),
    Object(3.11e18, 100,glm::vec3 {10.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.8f})
};
//          vectors of inactive objects, in future they're going to deleting
std::vector<Object> inactiveObjects = {};

Grid2D grid = Grid2D(100, 300);

//          matrices for vertex shader
glm::mat4 view, projection, model;


std::string theme = "dark";
//
// MAIN
//
int main() {
    //              GLFW initialization
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const float screenWidth = (glfwGetVideoMode(monitor)->width - 200.0f);
    const float screenHeight = (glfwGetVideoMode(monitor)->height - 100.0f);
    windowWidth = screenWidth;
    windowHeight = screenHeight;

    //              Creating window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    //              set callbacks
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);


    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, screenWidth, screenHeight);

    //shader program
    GLuint defaultShaderProgram = createShaders(vertexDefaultShaderSource, fragmentDefaultShaderSource);
    GLuint lightShaderProgram = createShaders(vertexLightShaderSource, fragmentLightShaderSource);

    // locations of uniform variables for default shaders
    GLint modelLoc = glGetUniformLocation(defaultShaderProgram, "model");
    GLint projectionLoc = glGetUniformLocation(defaultShaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(defaultShaderProgram, "view");
    GLint objectColorLoc = glGetUniformLocation(defaultShaderProgram, "objectColor");
    GLint amountOfLightsLoc = glGetUniformLocation(defaultShaderProgram, "lengthOfArray");

    // locations of uniform variables for light shaders
    GLint lightModelLoc = glGetUniformLocation(lightShaderProgram, "model");
    GLint lightProjectionLoc = glGetUniformLocation(lightShaderProgram, "projection");
    GLint lightViewLoc = glGetUniformLocation(lightShaderProgram, "view");
    GLint lightObjectColorLoc = glGetUniformLocation(lightShaderProgram, "objectColor");


    int amountOfLightsSources = 0;
    short currentIndex = 0;

    initializeObjects();
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);



    double lastTime = glfwGetTime();
    int frameCount = 0;
    double fpsUpdateInterval = 0.5;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // input processing
        processInput(window);

        // starting render
        if (theme == "dark") {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            grid.setColor({0.9f, 0.9f, 0.9f});
        } else {
            glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
            grid.setColor({0.0f, 0.0f, 0.0f});
        }
        // glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (chaseObject != -1) {
            camera.SetTarget(activeObjects[chaseObject].position);
        }

        updateCamera();
        //          GRID RENDER
        grid.vertices = grid.getVertices(activeObjects);
        grid.vertexCount = grid.vertices.size();
        grid.createVBOVAO(grid.VAO, grid.VBO, grid.vertices.data(), grid.vertices.size());
        model = glm::mat4(1.0f);
        model = glm::translate(model, {0.0f, 0.0f, 0.0f});
        glm::vec3 color = grid.color;
        glUseProgram(lightShaderProgram);

        glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(lightObjectColorLoc, 1, glm::value_ptr(color));

        glBindVertexArray(grid.VAO);
        glDrawArrays(GL_LINES, 0, grid.vertexCount / 3);
        glBindVertexArray(0);
        //          LIGHT
        currentIndex = 0;
        amountOfLightsSources = std::count_if(activeObjects.begin(), activeObjects.end(),
                                           [](Object &object) { return object.IsLightSource; });
        glUseProgram(defaultShaderProgram);
        glUniform1i(amountOfLightsLoc, amountOfLightsSources);
        for (auto& object : activeObjects) {
            if (object.Initilized && object.IsLightSource) {
                setLightSource(object, currentIndex, defaultShaderProgram);
                currentIndex++;
            }
        }

        //          OBJECTS RENDER
        for (auto& object : activeObjects) {
            if (object.Initilized && object.Active) {
                object.updateVertices();
            }
            if (activeObjects.size() > 1) {
                for (auto& object2 : activeObjects) {
                    if (object.VAO !=  object2.VAO && object.Active && object2.Active) {
                        float dx = object2.position.x - object.position.x;
                        float dy = object2.position.y - object.position.y;
                        float dz = object2.position.z - object.position.z;
                        float distance = sqrt(dx * dx + dy * dy + dz * dz); //r

                        if (distance > 0) { //preventing undefined behaviour
                            float gravityForce = (G * object.mass*object2.mass)/((distance*10000) * (distance*10000)); // F=G(m_1*m_2/r^2)
                            if (!pause) {
                                float acceleration = gravityForce / object.mass; //    F/m
                                float x = dx/distance;
                                float y = dy/distance;
                                float z = dz/distance;
                                glm::vec3 accelerationVector = {x*acceleration, y*acceleration, z*acceleration};
                                object.accelerateObject(accelerationVector);
                            }
                        }
                        if (object.checkCollision(&object2)) {
                            inactivateObject(object);
                            inactivateObject(object2);
                        }
                    }
                }
            }

            if (!pause) {
                updateState();
            }
            if (showPaths) {
                togglePaths();
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, object.position);
            color = object.objectColor;
            // if object is a light source, then using light shader program, if not - use normal shader program
            if (object.IsLightSource == true) {
                glUseProgram(lightShaderProgram);

                glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniform3fv(lightObjectColorLoc, 1, glm::value_ptr(color));

                glBindVertexArray(object.VAO);
                glDrawArrays(GL_TRIANGLES, 0, object.vertexCount / 6);
            }
            else {
                glUseProgram(defaultShaderProgram);

                glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniform3fv(objectColorLoc, 1, glm::value_ptr(color));

                glBindVertexArray(object.VAO);
                glDrawArrays(GL_TRIANGLES, 0, object.vertexCount / 6);
            }
            glBindVertexArray(0);
        }
        // for (Object& toDelete : inactiveObjects) {
        //     std::erase(inactiveObjects, toDelete);
        //     delete &toDelete;
        // }
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= fpsUpdateInterval) {
            double fps = frameCount / (currentTime - lastTime);
            std::string title = "Simulation - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        //events and buffer change
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto& obj : activeObjects) {
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
    }
    glDeleteProgram(defaultShaderProgram);

    glfwTerminate(); // this world is cruel and everything is going to end, just like this program
    return 0;
}

//
// main functions (definitions)
//
void addObject() {
    activeObjects.emplace_back(rand() * 1e16f, rand() % 10+1000 , glm::vec3 {distribution(generator), 0, distribution(generator)});
    initializeObjects();
}

void inactivateObject(Object& object) {
    object.Active = false;
    std::cout << "Inactivating object: " << object.VAO << ". Type: " << object.type << std::endl;
    std::erase(activeObjects, object);
    inactiveObjects.emplace_back(object);
}

void initializeObjects() {
    for (auto& obj : activeObjects) {
        if (!obj.Initilized) {
            obj.init();
        }
    }
}

void updateState() {
    for (auto& obj : activeObjects) {
        obj.updatePosition();
    }
}

bool spacePressed = false;
bool numberKeyPressed = false;
bool themeChanging = false;
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {

        if (!spacePressed) {
            if (pause == true) {
                pause = false;
            } else {
                pause = true;
            }
        }
        spacePressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        spacePressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {

        if (!themeChanging) {
            if (theme == "dark") {
                theme = "white";
            } else {
                theme = "dark";
            }
        }
        themeChanging = true;
    } else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        themeChanging = false;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        addObject();
    }


    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {

        if (!numberKeyPressed) {
            setScenario(1);
        }
        numberKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        numberKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {

        if (!numberKeyPressed) {
            setScenario(2);
        }
        numberKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
        numberKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {

        if (!numberKeyPressed) {
            setScenario(3);
        }
        numberKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        numberKeyPressed = false;
    }
}

GLuint createShaders(const char* vertexShaderSource,const  char* fragmentShaderSource) {
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    // shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (glfwGetCurrentContext() != NULL) {
        glViewport(0, 0, width, height);
        projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f, 100000.0f);
    }
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.scaling(yOffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            camera.SetFirstMouse(true);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            mousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
     if (!mousePressed) {
         return;
     }
    camera.rotate(xPos, yPos);
}

void updateCamera() {
    view = camera.update();
    projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f, 100000.0f);
}

void togglePaths() {
    //bruh
}

void setLightSource(Object& object, int index, GLuint defaultShaderProgram) {
    float constant = 1.0f;
    float linear = 0.009f;
    float quadratic = 0.0032f;
    glm::vec3 position = object.position;
    glm::vec3 ambient {0.2f};
    glm::vec3 diffuse {1.0f};

    std::string name = "lightArray[" + std::to_string(index) + "]";
    glUseProgram(defaultShaderProgram);
    GLint location = glGetUniformLocation(defaultShaderProgram, (name + ".position").c_str());
    glUniform3fv(location, 1, glm::value_ptr(position));
    location = glGetUniformLocation(defaultShaderProgram, (name + ".ambient").c_str());
    glUniform3fv(location, 1, glm::value_ptr(ambient));
    location = glGetUniformLocation(defaultShaderProgram, (name + ".diffuse").c_str());
    glUniform3fv(location, 1, glm::value_ptr(diffuse));
    location = glGetUniformLocation(defaultShaderProgram, (name + ".constant").c_str());
    glUniform1f(location, constant);
    location = glGetUniformLocation(defaultShaderProgram, (name + ".linear").c_str());
    glUniform1f(location, linear);
    location = glGetUniformLocation(defaultShaderProgram, (name + ".quadratic").c_str());
    glUniform1f(location, quadratic);
}

void setScenario(int keyNumber) {
    chaseObject = -1;
    switch (keyNumber) {
        case 1:
            activeObjects = {
            static_cast<Object>(Star(2.34e22, 1000, glm::vec3{0.0f, 0.0f, 0.0f}, 5772.0f)),
            Object(3.11e18, 100,glm::vec3 {10.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.8f})
        };
            camera.SetTarget({0.0f, 0.0f , 0.0f});
            break;
        case 2:
            activeObjects = {
            static_cast<Object>(Star(2.34e23, 1409, glm::vec3{10.0f, 0.0f, 0.0f}, 5000.0f)),
            static_cast<Object>(Star(2.34e23, 1409, glm::vec3{-10.0f, 0.0f, 0.0f}, 6800.0f))
        };
            activeObjects[1].velocity[2] = 4;
            activeObjects[0].velocity[2] = -4;
            camera.SetTarget({0.0f, 0.0f , 0.0f});
            break;
        case 3:
            activeObjects = {
            static_cast<Object>(Star(2.34e22, 1409, glm::vec3{0.0f, 0.0f, 0.0f}, 5772.0f)),
            Object(6.11e17, 123,glm::vec3 {5.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.8f}),
            Object(3.33022e19, 336.4,glm::vec3 {-10.0f, 0.0f, -0.5f}, {0.0f, 0.0f, 2.2f}),
            Object(1.223e20, 480,glm::vec3 {0.0f, 0.0f, -15.5f}, {1.5f, 0.0f, 0.0f})
        };
            activeObjects[3].objectColor = {0.5f, 0.5f, 1.0f};
            camera.SetTarget(activeObjects[3].position);
            chaseObject = 3;
            break;
        default:
            break;
    }
    initializeObjects();
}

