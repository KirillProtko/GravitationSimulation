#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h> // yea i know there is a .dll file, but for now i will include it like that, bruh
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <memory>
//          files

#include "Classes/object.h"
#include "Classes/star.h"
#include "Classes/planet.h"

#include "Classes/camera.h"
#include "Classes/grid2D.h"

#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_glfw.h"
#include "libs/imgui/backends/imgui_impl_opengl3.h"

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(0,1);

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
uniform vec4 objectColor;
void main() {
    FragColor = objectColor;
})glsl";

const char* vertexTrailShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aAlpha;
uniform mat4 projection;
uniform mat4 view;

out float VertAlpha;
void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    VertAlpha = aAlpha;
})glsl";

const char* fragmentTrailShaderSource = R"glsl(
#version 330 core
in float VertAlpha;
out vec4 FragColor;
uniform vec3 objectColor;
void main() {
    FragColor = vec4(objectColor, VertAlpha);
})glsl";

const char* vertexPreviewShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out float VertAlpha;
void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);

})glsl";

const char* fragmentPreviewShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
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
void inactivateObject(Object& object);
void deleteObject();
void initializeObjects();
void initializeObject(Object& newObject);
GLuint createShaders(const char* vertexShaderSource, const char* fragmentShaderSource);
void setLightSource(Object& object, int index, GLuint defaultShaderProgram);
glm::vec3 screenToWorld(float mouseX, float mouseY);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xPos, double yPos);

void setScenario(int keyNumber);

//
// GLOBAL VARIABLES
//
bool pause = false, mousePressed = false, showTrails = false, cameraChanged = false;
int chaseObject = -1;
enum Stages {POSITION_CHOICE, RADIUS_CHOICE, VELOCITY_CHOICE, NONE};
Stages addingStage = NONE;
float gridOpacity = 1.0f;
//          constant variables
constexpr double G = 6.6743e-11;

float windowWidth, windowHeight;


//          vectors of active Objects (star, planets, black holes, etc.), which must be on screen
std::vector<Object> activeObjects = {
    static_cast<Object>(Star(2.34e22, 1000, glm::vec3{0.0f, 0.0f, 0.0f}, 5772.0f)),
    static_cast<Object>(Planet(3.11e18, 100,glm::vec3 {10.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 4.0f}))
};
//          vectors of inactive objects, in future they're going to deleting
std::vector<Object> inactiveObjects = {};
std::vector<Object> newObject = {};

Grid2D grid = Grid2D(100, 300);

//          matrices for vertex shader
glm::mat4 view, projection, model;
GLuint vaoPreview, vbopreview;

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
    const float screenWidth = (glfwGetVideoMode(monitor)->width - 400.0f);
    const float screenHeight = (glfwGetVideoMode(monitor)->height - 200.0f);
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
    GLuint trailShaderProgram = createShaders(vertexTrailShaderSource, fragmentTrailShaderSource);
    GLuint previewShaderProgram = createShaders(vertexPreviewShaderSource, fragmentPreviewShaderSource);

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

    //locations of uniform variables for trail shaders
    GLint trailProjectionLoc = glGetUniformLocation(trailShaderProgram, "projection");
    GLint trailViewLoc = glGetUniformLocation(trailShaderProgram, "view");
    GLint trailObjectColorLoc = glGetUniformLocation(trailShaderProgram, "objectColor");

    //locations of uniform variables for preview shaders
    GLint previewProjectionLoc = glGetUniformLocation(previewShaderProgram, "projection");
    GLint previewViewLoc = glGetUniformLocation(previewShaderProgram, "view");
    GLint previewColorLoc = glGetUniformLocation(previewShaderProgram, "color");

    int amountOfLightsSources = 0;
    short currentIndex = 0;

    initializeObjects();
    grid.vertices = grid.getVertices(activeObjects);
    grid.vertexCount = grid.vertices.size();
    grid.createVBOVAO(grid.VAO, grid.VBO, grid.vertices.data(), grid.vertices.size());

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double lastTime = glfwGetTime();
    int frameCount = 0;
    double fpsUpdateInterval = 0.5;
    glm::vec4 color;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiIO& io = ImGui::GetIO();
    ImFont* russian_font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // input processing
        processInput(window);

        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // starting render
        if (theme == "dark") {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            grid.setColor({0.9f, 0.9f, 0.9f});
        } else {
            // glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            grid.setColor({0.0f, 0.0f, 0.0f});
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (chaseObject != -1) {
            camera.SetTarget(activeObjects[chaseObject].position);
        }

        updateCamera();

        //          LIGHT
        currentIndex = 0;
        amountOfLightsSources = std::count_if(activeObjects.begin(), activeObjects.end(),
                                           [](Object &object) { return object.IsLightSource; });
        glUseProgram(defaultShaderProgram);
        glUniform1i(amountOfLightsLoc, amountOfLightsSources);
        for (auto& object : activeObjects) {
            if (object.Initialized && object.IsLightSource) {
                setLightSource(object, currentIndex, defaultShaderProgram);
                currentIndex++;
            }
        }

        //          OBJECTS RENDER
        for (auto& object : activeObjects) {
            if (!pause) {
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
                                if (object.mass > object2.mass * 10) {
                                    inactivateObject(object2);
                                    // object.setMass(object.mass + object.mass);
                                }
                                else if (object.mass * 10 < object2.mass) {
                                    inactivateObject(object);
                                    // object2.setMass(object2.mass + object.mass);
                                }
                                else {
                                    inactivateObject(object);
                                    inactivateObject(object2);
                                }
                            }
                        }
                    }
                }
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, object.position);
            color = glm::vec4(object.objectColor, 1.0f);
            // if object is a light source, then using light shader program, if not - use normal shader program
            if (object.IsLightSource == true) {
                glUseProgram(lightShaderProgram);

                glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniform4fv(lightObjectColorLoc, 1, glm::value_ptr(color));

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
            if (showTrails) {
                if (!pause) {
                    object.updateTrail();
                }
                color = (theme == "dark") ? glm::vec4(1.0f) : glm::vec4(0.0f,0.0f,0.0f, 1.0f);
                glUseProgram(trailShaderProgram);

                glUniformMatrix4fv(trailProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(trailViewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniform3fv(trailObjectColorLoc, 1, glm::value_ptr(color));

                glBindVertexArray(object.TrailVAO);
                glLineWidth(2.0f);
                glDrawArrays(GL_LINES, 0, object.trailVertices.size() / 4);
            }
        }
        if (!pause) {
            updateState();
        }
        //          GRID RENDER
        // after all because it can be semi-transparent
        glLineWidth(1.0f);
        if (gridOpacity != 0.0f) {
            if (!pause) {
                grid.updateVertices(activeObjects);
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, {0.0f, 0.0f, 0.0f});
            glUseProgram(lightShaderProgram);
            color = glm::vec4(grid.color, gridOpacity);

            glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniform4fv(lightObjectColorLoc, 1, glm::value_ptr(color));

            glBindVertexArray(grid.VAO);
            glDrawArrays(GL_LINES, 0, grid.vertexCount / 3);
        }
        //      PREVIEW OF CREATING
        if (addingStage == RADIUS_CHOICE) {
            glGenVertexArrays(1, &vaoPreview);
            glGenBuffers(1, &vbopreview);

            glBindVertexArray(vaoPreview);
            glBindBuffer(GL_ARRAY_BUFFER, vbopreview);
            float xObj = newObject[0].position.x;
            float zObj = newObject[0].position.z;
            glm::vec3 lastPos = screenToWorld(xPos, yPos);

            std::vector<float> vertPrev = {};
            float radius = glm::distance(newObject[0].position, {lastPos.x, 0.0f, lastPos.z});
            vertPrev.push_back(xObj);
            vertPrev.push_back(0.0f);
            vertPrev.push_back(zObj);
            for (int i = 0; i <= 20; ++i) {
                vertPrev.push_back(xObj + radius * cos( 2*glm::pi<float>() * (float) i / 20));
                vertPrev.push_back(0.0f);
                vertPrev.push_back(zObj + radius * sin(2*glm::pi<float>() * (float) i / 20));
            }
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertPrev.size(), vertPrev.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glUseProgram(previewShaderProgram);

            color = (theme == "dark") ? glm::vec4(1.0f, 1.0f, 1.0f, 0.5f) : glm::vec4(0.0f,0.0f,0.0f, 0.5f);
            glUniformMatrix4fv(previewProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(previewViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniform4fv(previewColorLoc, 1, glm::value_ptr(color));

            glBindVertexArray(vaoPreview);
            glDrawArrays(GL_TRIANGLE_FAN, 0, vertPrev.size() / 3);

        } else if (addingStage == VELOCITY_CHOICE) {
            glGenVertexArrays(1, &vaoPreview);
            glGenBuffers(1, &vbopreview);

            glBindVertexArray(vaoPreview);
            glBindBuffer(GL_ARRAY_BUFFER, vbopreview);
            float xObj = newObject[0].position.x;
            float zObj = newObject[0].position.z;
            glm::vec3 lastPos = screenToWorld(xPos, yPos);

            std::vector<float> vertPrev = {
                xObj, 0.0f, zObj,
                lastPos.x, 0.0f, lastPos.z
            };
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, vertPrev.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glUseProgram(previewShaderProgram);

            color = (theme == "dark") ? glm::vec4(1.0f, 1.0f, 1.0f, 0.5f) : glm::vec4(0.0f,0.0f,0.0f, 0.5f);
            glUniformMatrix4fv(previewProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(previewViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniform4fv(previewColorLoc, 1, glm::value_ptr(color));

            glBindVertexArray(vaoPreview);
            glDrawArrays(GL_LINES, 0, 2);
        }

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        ImGui::Begin("Object Creation", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        if (addingStage == NONE)
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Справка:");
        else
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Добавление объекта");
        ImGui::Separator();

        switch (addingStage) {
            case POSITION_CHOICE:
                ImGui::Text("1 Этап: Выбор позиции");
                ImGui::BulletText("Нажмите ЛКМ на место, где \nхотите разместить объект");
                break;
            case RADIUS_CHOICE:
                ImGui::Text("2 Этап: Выбор радиуса");
                ImGui::BulletText("Нажмите ЛКМ, чтобы установить \nрадиус");
                ImGui::BulletText("Текущий радиус: %.2f",
                                glm::distance(newObject[0].position, screenToWorld(xPos, yPos)));
                break;
            case VELOCITY_CHOICE:
                ImGui::Text("3 Этап: Выбор ускорения");
                ImGui::BulletText("Нажмите ЛКМ, чтобы установить \nначальное ускорение");
                ImGui::BulletText("Текущее ускорение: (%.2f, %.2f)",
                                screenToWorld(xPos, yPos).x - newObject[0].position.x,
                                screenToWorld(xPos, yPos).z - newObject[0].position.z);
                break;
            default:
                ImGui::BulletText("Е - переключить тему");
                ImGui::BulletText("Пробел - остановить симуляцию");
                ImGui::BulletText("Ф - перейти в режим добавления \nобъекта");
                ImGui::BulletText("1, 2, 3 - загрузить сценарий");
                ImGui::BulletText("Esc - выйти из программы");
                ImGui::BulletText("↑ и ↓ - изменить прозрачность сетки");
                ImGui::BulletText("Ч - включить/выключить траектории");
                break;
        }

        ImGui::End();

        glBindVertexArray(0);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //      FPS COUNTER in windows title
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
    glDeleteVertexArrays(1, &vaoPreview);
    glDeleteBuffers(1, &vbopreview);
    glDeleteProgram(defaultShaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate(); // this world is cruel and everything is going to end, just like this program
    return 0;
}

//
// main functions (definitions)
//

void inactivateObject(Object& object) {
    object.Active = false;
    std::cout << "Inactivating object: " << object.VAO << ". Type: " << object.type << std::endl;
    std::erase(activeObjects, object);
    inactiveObjects.emplace_back(object);
}

void initializeObjects() {
    for (auto& obj : activeObjects) {
        if (!obj.Initialized) {
            obj.init();
        }
    }
}

void initializeObject(Object& newObj) {
    activeObjects.emplace_back(newObj);
    activeObjects.back().init();
}

void updateState() {
    for (auto& obj : activeObjects) {
        obj.updatePosition();
    }
}

bool spacePressed = false;
bool numberKeyPressed = false;
bool themeChanging = false;
bool gridOpacityKey = false;
bool toggleTrailsKey = false;
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

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {

        if (!toggleTrailsKey) {
            if (showTrails == true) {
                showTrails = false;
            } else {
                showTrails = true;
            }
        }
        toggleTrailsKey = true;
    } else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE) {
        toggleTrailsKey = false;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {

        if (!gridOpacityKey) {
            if (gridOpacity < 1.0f)
                gridOpacity += 0.1f;
        }
        gridOpacityKey = true;
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        gridOpacityKey = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {

        if (!gridOpacityKey) {
            if (gridOpacity > 0.0f)
                gridOpacity -= 0.1f;
        }
        gridOpacityKey = true;
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        gridOpacityKey = false;
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
        if (addingStage == NONE)
        {
            addingStage = POSITION_CHOICE;
            pause = true;

            camera.SetMode("locked");
            camera.SetTargetPosition({0.0f, 10.0f, 0.0f});
            camera.SetTargetAngles({90.0f, 0.0f});
            camera.SetTargetTarget({0.0f, 0.0f, 0.0f});

            Object obj;
            newObject.emplace_back(obj);
        }
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
            if (addingStage != NONE) {
                double xPos, zPos;
                glfwGetCursorPos(window, &xPos, &zPos);
                glm::vec3 cursorPosition = screenToWorld(xPos, zPos);
                switch (addingStage) {
                    case POSITION_CHOICE:
                        newObject[0].position = cursorPosition * glm::vec3{1.0f, 0.0f, 1.0f};
                        camera.SetTargetPosition(cursorPosition + glm::vec3{0.0f, 5.0f, 0.0f});
                        camera.SetTargetTarget(newObject[0].position);
                        addingStage = RADIUS_CHOICE;
                        break;
                    case RADIUS_CHOICE:
                        newObject[0].radius = glm::distance(newObject[0].position, cursorPosition);
                        newObject[0].density = 500.0f;
                        newObject[0].calculateMass();
                        newObject[0].type = "Planet";
                        newObject[0].IsLightSource = false;
                        newObject[0].objectColor = glm::vec3(distribution(generator), distribution(generator), distribution(generator));
                        initializeObject(newObject[0]);
                        addingStage = VELOCITY_CHOICE;
                        break;
                    case VELOCITY_CHOICE:
                        activeObjects.back().velocity = cursorPosition - newObject[0].position;
                        activeObjects.back().velocity.y = 0;
                        addingStage = NONE;
                        pause = false;
                        camera.SetTargetTarget(glm::vec3{0.0f, 0.0f, 0.0f});
                        camera.SetTargetAngles({45.0f, 20.0f});
                        camera.SetMode("centered");
                        newObject.clear();
                        break;
                    default:
                        break;
                }
            } else {
                camera.SetFirstMouse(true);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
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
    view = camera.getViewMatrix();
    if (camera.GetPosition() != camera.GetTargetPosition() || camera.GetAngles() != camera.GetTargetAngles() || camera.GetTarget() != camera.GetTargetTarget()) {
        camera.ExponentionalChangePosition();
        camera.ExponentionalChangeTarget();
        camera.ExponentionalChangeAngles();
    }
    projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f, 100000.0f);
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
            static_cast<Object>(Planet(3.11e18, 100,glm::vec3 {10.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 4.0f}))
        };
            break;
        case 2:
            activeObjects = {
            static_cast<Object>(Star(2.34e23, 1409, glm::vec3{10.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 6.0f}, 5000.0f)),
            static_cast<Object>(Star(2.34e23, 1409, glm::vec3{-10.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -6.0f},6800.0f))
        };
            break;
        case 3:
            activeObjects = {
            static_cast<Object>(Star(2.34e22, 1409, glm::vec3{0.0f, 0.0f, 0.0f}, 5772.0f)),
            static_cast<Object>(Planet(6.11e17, 123,glm::vec3 {5.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 5.0f})),
            static_cast<Object>(Planet(3.33022e19, 336.4,glm::vec3 {-10.0f, 0.0f, -0.5f}, {0.0f, 0.0f, 3.7f})),
            static_cast<Object>(Planet(1.223e20, 480,glm::vec3 {0.0f, 0.0f, -15.5f}, {3.2f, 0.0f, 0.0f}))
        };
            activeObjects[3].objectColor = {0.5f, 0.5f, 1.0f};
            // chaseObject = 3;
            break;
        default:
            break;
    }
    initializeObjects();
}

glm::vec3 screenToWorld(float mouseX, float mouseY) {
    float x = (2.0f * mouseX) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / windowHeight;

    glm::vec4 rayClip(x, y, -1.0f, 1.0f);
    glm::mat4 invProjection = glm::inverse(projection);
    glm::vec4 rayEye = invProjection * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::mat4 invView = glm::inverse(view);
    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));

    float t = -camera.GetPosition().y / rayDir.y;
    return camera.GetPosition() + rayDir * t;
}
