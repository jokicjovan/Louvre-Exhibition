#define _CRT_SECURE_NO_WARNINGS
#define CRES 30

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../headers/camera.h"
#include "../headers/shader.h"
#include "../headers/model.h"

enum LightPosition {
    CEILING,
    FLOOR,
    CAMERA
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processCommonInput(GLFWwindow* window);
void processSceneInput(GLFWwindow* window, bool& useGouraud);
void processStopButtonInput(GLFWwindow* window, bool& stopButtonOn);
void processProgressBarInput(GLFWwindow* window, float& progressValue, float progressStep);
void processCameraSpotLightInput(GLFWwindow*  window, bool& cameraSpotLightOn);
void processLampPointLightInput(GLFWwindow* window, bool& lampPointLightOn);

void setupLight(Shader& shader, LightPosition lightType, bool turnOn = true, float lightIntensity = 0.0f);
void setupSceneLights(Shader& shader, bool cameraSpotLightOn, bool floorPointLightOn);
void setupMVP(Shader& shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);

void drawRoom(int VAO, Shader shader, unsigned roomTextures[]);
void drawFrontWallPictures(int VAO, Shader shader, unsigned frontWallPicturesTextures[], bool stopButtonOn, float angle, float rotationRadius);
void drawBackWallPictures(int VAO, Shader shader, unsigned backWallPicturesTextures[]);
void drawStoppingButton(int VAO, Shader shader, bool stopButtonOn);
void drawProgressBar(int VAO, Shader shader, float progressBarValue, int progressBarQuadsNum);
void drawFloorLightButton(int VAO, Shader shader, unsigned floorPointLightButtonTextures[], bool floorPointLightOn);
void drawFloorLight(Model& floorLightModel, Shader shader);
void drawCeilingLight(Model& ceilingLightModel, Shader shader);
void drawSignature(int VAO, Shader shader, unsigned signatureTexture);

static unsigned loadImageToTexture(const char* filePath);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, -0.3f, 0.0f), glm::vec3(-1.8f, -0.3f, -1.8f), glm::vec3(1.8f, 0.8f, 1.8f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main(void)
{
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ INICIJALIZACIJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    const char wTitle[] = "Louvre Exhibition 3D";
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, wTitle, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // Postavljanje callback-ova
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Gasenje cursora
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    Shader phongMaterialShader("shaders/phong_material.vert", "shaders/phong_material.frag");
    Shader phongTextureShader("shaders/phong_texture.vert", "shaders/phong_texture.frag");
    Shader phongPictureShader("shaders/phong_picture.vert", "shaders/phong_picture.frag");
    Shader gouraudMaterialShader("shaders/gouraud_material.vert", "shaders/gouraud_material.frag");
    Shader signatureShader("shaders/signature.vert", "shaders/signature.frag");

    unsigned int VAO[7];
    glGenVertexArrays(7, VAO);
    unsigned int VBO[7];
    glGenBuffers(7, VBO);

    // ------------------------------- SOBA -------------------------------
    float roomVertices[] = {
        //X      Y       Z       NX     NY     NZ      S     T
        //PREDNJI ZID
        -2.0f,  -1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,  //Dole-Levo
         2.0f,  -1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,  //Dole-Desno
         2.0f,   1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,  //Gore-Desno
         2.0f,   1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,  //Gore-Desno
        -2.0f,   1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,  //Gore-Levo
        -2.0f,  -1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,  //Dole-Levo
                                    
        //ZADNJI ZID                      
        -2.0f,  -1.0f,   2.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,  //Dole-Levo
        -2.0f,   1.0f,   2.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,  //Gore-Levo
         2.0f,   1.0f,   2.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,  //Gore-Desno
         2.0f,   1.0f,   2.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,  //Gore-Desno
         2.0f,  -1.0f,   2.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,  //Dole-Desno
        -2.0f,  -1.0f,   2.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,  //Dole-Levo
                 
        //LEVI ZID   
        -2.0f, - 1.0f,  -2.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,  //Dole-Levo
        -2.0f,   1.0f,  -2.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,  //Gore-Levo
        -2.0f,   1.0f,   2.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,  //Gore-Desno
        -2.0f,   1.0f,   2.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,  //Gore-Desno
        -2.0f, - 1.0f,   2.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,  //Dole-Desno
        -2.0f, - 1.0f,  -2.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,  //Dole-Levo
                       
        //DESNI ZID            
         2.0f,   1.0f,   2.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,  //Gore-Desno
         2.0f,   1.0f,  -2.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,  //Gore-Levo
         2.0f,  -1.0f,  -2.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,  //Dole-Levo
         2.0f,  -1.0f,  -2.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,  //Dole-Levo
         2.0f,  -1.0f,   2.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,  //Dole-Desno
         2.0f,   1.0f,   2.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,  //Gore-Desno
                       
        //POD       
        -2.0f,  -1.0f,  -2.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,  //Gore-Levo
        -2.0f,  -1.0f,   2.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,  //Dole-Levo
         2.0f,  -1.0f,   2.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,  //Dole-Desno
         2.0f,  -1.0f,   2.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,  //Dole-Desno
         2.0f,  -1.0f,  -2.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,  //Gore-Desno
        -2.0f,  -1.0f,  -2.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,  //Gore-Levo
                                    
        //PLAFON                       
        -2.0f,   1.0f,  -2.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,  //Gore-Levo
         2.0f,   1.0f,  -2.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,  //Gore-Desno
         2.0f,   1.0f,   2.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,  //Dole-Desno
         2.0f,   1.0f,   2.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,  //Dole-Desno
        -2.0f,   1.0f,   2.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,  //Dole-Levo
        -2.0f,   1.0f,  -2.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f   //Gore-Levo
    };

    unsigned int roomStride = (3 + 3 + 2) * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, roomStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, roomStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributi za teksture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, roomStride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // ------------------------------- SLIKE SA OKVIRIMA NA PREDNJEM ZIDU -------------------------------

    float frontWallPictureHeight = 0.8f;
    float frontWallPictureOffset = 0.1f;
    float frontWallPicturesAspectRatios[] = { 800.0 / 1192.0, 1280.0 / 874.0, 1280.0 / 1027.0, 1920.0 / 1207.0 };
    float frontWallPicturesTotalWidth = 0.0f;
    for (int i = 0; i < 4; i++) {
        frontWallPicturesTotalWidth += frontWallPictureHeight * frontWallPicturesAspectRatios[i];
    }
    float frontWallPicturesScaler = (frontWallPicturesTotalWidth + 5 * frontWallPictureOffset > 3.8f) ? 
        (3.8f - 5 * frontWallPictureOffset) / frontWallPicturesTotalWidth : 1.0f;     // kreiranje "skalera" za slike 
                                                                                      //(uzima u obzir sirinu slika i offset-e izmedju njih)
    float frontWallPicturesWidths[4];
    for (int i = 0; i < 4; i++) {
        frontWallPicturesWidths[i] = frontWallPictureHeight * frontWallPicturesAspectRatios[i] * frontWallPicturesScaler;
    }

    float frontWallPicturesVertices[] = {
        //X                                                                                                                                                       Y                              Z       NX     NY      NZ      S    T      OKVIR    R    G    B    A
        // Mona Lisa sa okvirom                                                                                                                                            
        -1.9f + frontWallPictureOffset + frontWallPicturesWidths[0]                                                                                            , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Desno
        -1.9f + frontWallPictureOffset + frontWallPicturesWidths[0]                                                                                            ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Desno
        -1.9f + frontWallPictureOffset                                                                                                                         ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Levo
        -1.9f + frontWallPictureOffset                                                                                                                         ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Levo
        -1.9f + frontWallPictureOffset                                                                                                                         , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Levo
        -1.9f + frontWallPictureOffset + frontWallPicturesWidths[0]                                                                                            , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Desno
                                                                                                                                                                                  
        // The Raft of the Medusa sa okvirom                                                                                                                                                              
        -1.9f + 2 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.9f + 2 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.9f + 2 * frontWallPictureOffset + frontWallPicturesWidths[0]                                                                                        ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.9f + 2 * frontWallPictureOffset + frontWallPicturesWidths[0]                                                                                        ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.9f + 2 * frontWallPictureOffset + frontWallPicturesWidths[0]                                                                                        , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.9f + 2 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
                                                                                                                                                                                  
        // Liberty Leading the People sa okvirom                                                                                                                                  
        -1.9f + 3 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.9f + 3 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.9f + 3 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.9f + 3 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.9f + 3 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.9f + 3 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
                                                                                                                                                                           
        // The Coronation of Napoleon sa okvirom                                                                                                                             
        -1.9f + 4 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2] + frontWallPicturesWidths[3] , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.9f + 4 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2] + frontWallPicturesWidths[3] ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.9f + 4 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.9f + 4 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              ,  frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.9f + 4 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   0.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.9f + 4 * frontWallPictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2] + frontWallPicturesWidths[3] , -frontWallPictureHeight / 2 ,  -1.99f,  0.0f,  0.0f,  1.0f,   1.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
    };

    unsigned int frontWallPicturesStride = (3 + 3 + 2 + 1 + 4) * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontWallPicturesVertices), frontWallPicturesVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributi za teksture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Debljina okvira
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Atributi boja (RGBA)
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // ------------------------------- SLIKE SA OKVIRIMA NA ZADNJEM ZIDU -------------------------------

    float backWallPictureHeight = 0.8f;
    float backWallPictureOffset = 0.2f;
    float backWallPicturesAspectRatios[] = { 1014.0 / 1280.0, 1003.0 / 1280.0, 997.0 / 800.0, 1011.0 / 1280.0};
    float backWallPicturesTotalWidth = 0.0f;
    for (int i = 0; i < 4; i++) {
        backWallPicturesTotalWidth += backWallPictureHeight * backWallPicturesAspectRatios[i];
    }
    float backWallPicturesScaler = (backWallPicturesTotalWidth + 5 * backWallPictureOffset > 3.8f) ?
        (3.8f - 5 * backWallPictureOffset) / backWallPicturesTotalWidth : 1.0f;
    float backWallPicturesWidths[4];
    for (int i = 0; i < 4; i++) {
        backWallPicturesWidths[i] = backWallPictureHeight * backWallPicturesAspectRatios[i] * backWallPicturesScaler;
    }

    float backWallPicturesVertices[] = {
        //X                                                                                                                                                 Y                             Z       NX    NY     NZ      S    T      OKVIR    R    G    B    A
        // Starry Night sa okvirom                                                                                                                                         
        -1.9f + backWallPictureOffset                                                                                                                     ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   0.0, 1.0, 1.0, 1.0,  //Gore-Levo
        -1.9f + backWallPictureOffset + backWallPicturesWidths[0]                                                                                         ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 1.0   ,0.05f,   0.0, 1.0, 1.0, 1.0,  //Gore-Desno
        -1.9f + backWallPictureOffset + backWallPicturesWidths[0]                                                                                         , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   0.0, 1.0, 1.0, 1.0,  //Dole-Desno
        -1.9f + backWallPictureOffset + backWallPicturesWidths[0]                                                                                         , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   0.0, 1.0, 1.0, 1.0,  //Dole-Desno
        -1.9f + backWallPictureOffset                                                                                                                     , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 0.0   ,0.05f,   0.0, 1.0, 1.0, 1.0,  //Dole-Levo
        -1.9f + backWallPictureOffset                                                                                                                     ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   0.0, 1.0, 1.0, 1.0,  //Gore-Levo
                                                                                                                                                                                                              
        // Wheat Field with Cypresses sa okvirom                                                                                                                                                              
        -1.9f + 2 * backWallPictureOffset + backWallPicturesWidths[0]                                                                                     ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   1.0, 0.0, 1.0, 1.0,
        -1.9f + 2 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1]                                                         ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 1.0   ,0.05f,   1.0, 0.0, 1.0, 1.0,
        -1.9f + 2 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1]                                                         , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   1.0, 0.0, 1.0, 1.0,
        -1.9f + 2 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1]                                                         , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   1.0, 0.0, 1.0, 1.0,
        -1.9f + 2 * backWallPictureOffset + backWallPicturesWidths[0]                                                                                     , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 0.0   ,0.05f,   1.0, 0.0, 1.0, 1.0,
        -1.9f + 2 * backWallPictureOffset + backWallPicturesWidths[0]                                                                                     ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   1.0, 0.0, 1.0, 1.0,
                                                                                                                                                                                                              
        // Cafe Terrace at Night sa okvirom                                                                                                                                                                   
        -1.9f + 3 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1]                                                         ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   0.0, 0.5, 0.5, 1.0,
        -1.9f + 3 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2]                             ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 1.0   ,0.05f,   0.0, 0.5, 0.5, 1.0,
        -1.9f + 3 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2]                             , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   0.0, 0.5, 0.5, 1.0,
        -1.9f + 3 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2]                             , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   0.0, 0.5, 0.5, 1.0,
        -1.9f + 3 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1]                                                         , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 0.0   ,0.05f,   0.0, 0.5, 0.5, 1.0,
        -1.9f + 3 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1]                                                         ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   0.0, 0.5, 0.5, 1.0,
                                                                                                                                                                                                              
        // The Almond blossom sa okvirom                                                                                                                                                                      
        -1.9f + 4 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2]                             ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   0.0, 0.5, 1.0, 1.0,
        -1.9f + 4 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2] + backWallPicturesWidths[3] ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 1.0   ,0.05f,   0.0, 0.5, 1.0, 1.0,
        -1.9f + 4 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2] + backWallPicturesWidths[3] , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   0.0, 0.5, 1.0, 1.0,
        -1.9f + 4 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2] + backWallPicturesWidths[3] , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   1.0, 0.0   ,0.05f,   0.0, 0.5, 1.0, 1.0,
        -1.9f + 4 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2]                             , -backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 0.0   ,0.05f,   0.0, 0.5, 1.0, 1.0,
        -1.9f + 4 * backWallPictureOffset + backWallPicturesWidths[0] + backWallPicturesWidths[1] + backWallPicturesWidths[2]                             ,  backWallPictureHeight / 2 ,  1.99f,  0.0f, 0.0f, -1.0f,   0.0, 1.0   ,0.05f,   0.0, 0.5, 1.0, 1.0,
    };

    unsigned int backWallPicturesStride = (3 + 3 + 2 + 1 + 4) * sizeof(float);

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backWallPicturesVertices), backWallPicturesVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, backWallPicturesStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, backWallPicturesStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributi za teksture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, backWallPicturesStride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Debljina okvira
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, backWallPicturesStride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Atributi boja (RGBA)
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, backWallPicturesStride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // ------------------------------- DUGME ZA KRETANJE SLIKA -------------------------------

    float buttonVertices[CRES * 6 + 12];
    float r = 0.15;
    buttonVertices[0] = 1.4;
    buttonVertices[1] = 0.75;
    buttonVertices[2] = -1.99f;
    buttonVertices[3] = 0.0f;
    buttonVertices[4] = 0.0f;
    buttonVertices[5] = 1.0f;
    for (int i = 0; i <= CRES; i++)
    {
        buttonVertices[6 + 6 * i] = 1.4 + r * cos((3.141592 / 180) * (i * 360 / CRES));
        buttonVertices[6 + 6 * i + 1] = 0.75 + r * sin((3.141592 / 180) * (i * 360 / CRES));
        buttonVertices[6 + 6 * i + 2] = -1.99f;
        buttonVertices[6 + 6 * i + 3] = 0.0f;
        buttonVertices[6 + 6 * i + 4] = 0.0f;
        buttonVertices[6 + 6 * i + 5] = 1.0f;
    }

    float buttonStride = (3 + 3) * sizeof(float);

    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, buttonStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, buttonStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ------------------------------- PROGRESS BAR -------------------------------

    const int progressBarQuadsNum = 100;
    const int progressBarRowSize = 6 * (3 + 3);
    float progressStep = 0.02f;
    float progressBarVertices[progressBarQuadsNum * progressBarRowSize];
    for (int i = 0; i < progressBarQuadsNum; i++) {
        // Dole-Desno
        progressBarVertices[i * progressBarRowSize + 0] = -1.8f + (i + 1) * progressStep;
        progressBarVertices[i * progressBarRowSize + 1] = 0.7f;
        progressBarVertices[i * progressBarRowSize + 2] = -1.99f;
        progressBarVertices[i * progressBarRowSize + 3] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 4] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 5] = 1.0f;

        // Gore-Desno
        progressBarVertices[i * progressBarRowSize + 6] = -1.8f + (i + 1) * progressStep;
        progressBarVertices[i * progressBarRowSize + 7] = 0.8f;
        progressBarVertices[i * progressBarRowSize + 8] = -1.99f;
        progressBarVertices[i * progressBarRowSize + 9] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 10] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 11] = 1.0f;

        // Gore-Levo
        progressBarVertices[i * progressBarRowSize + 12] = -1.8f + i * progressStep;
        progressBarVertices[i * progressBarRowSize + 13] = 0.8f;
        progressBarVertices[i * progressBarRowSize + 14] = -1.99f;
        progressBarVertices[i * progressBarRowSize + 15] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 16] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 17] = 1.0f;

        // Dole-Levo
        progressBarVertices[i * progressBarRowSize + 18] = -1.8f + i * progressStep;
        progressBarVertices[i * progressBarRowSize + 19] = 0.7f;
        progressBarVertices[i * progressBarRowSize + 20] = -1.99f;
        progressBarVertices[i * progressBarRowSize + 21] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 22] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 23] = 1.0f;

        // Dole-Desno
        progressBarVertices[i * progressBarRowSize + 24] = -1.8f + (i + 1) * progressStep;
        progressBarVertices[i * progressBarRowSize + 25] = 0.7f;
        progressBarVertices[i * progressBarRowSize + 26] = -1.99f;
        progressBarVertices[i * progressBarRowSize + 27] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 28] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 29] = 1.0f;

        // Gore-Levo
        progressBarVertices[i * progressBarRowSize + 30] = -1.8f + i * progressStep;
        progressBarVertices[i * progressBarRowSize + 31] = 0.8f;
        progressBarVertices[i * progressBarRowSize + 32] = -1.99f;
        progressBarVertices[i * progressBarRowSize + 33] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 34] = 0.0f;
        progressBarVertices[i * progressBarRowSize + 35] = 1.0f;
    }
    float progressBarStride = (3 + 3) * sizeof(float);

    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, progressBarStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, buttonStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ------------------------------- POTPIS -------------------------------

    float signatureVertices[] = {
        // X       Y      Z      S    T
           0.9f,  -0.7f,  0.0f,  1.0, 1.0,  // Gore-Desno
           0.5f,  -0.7f,  0.0f,  0.0, 1.0,  // Gore-Levo
           0.5f,  -0.9f,  0.0f,  0.0, 0.0,  // Dole-Levo
           0.5f,  -0.9f,  0.0f,  0.0, 0.0,  // Dole-Levo
           0.9f,  -0.9f,  0.0f,  1.0, 0.0,  // Dole-Desno
           0.9f,  -0.7f,  0.0f,  1.0, 1.0   // Gore-Desno
    };

    float signatureStride = (3 + 2) * sizeof(float);

    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureVertices), signatureVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ------------------------------- PODNO SVETLO -------------------------------

    Model floorLightModel("res/light/floor/FloorLight.obj");
    Model ceilingLightModel("res/light/ceiling/SpotLight.obj");

    // ------------------------------- DUGME ZA PODNO SVETLO -------------------------------

    float lampButtonVertices[] = {
        //  X       Y       Z      NX     NY     NZ     S      T
           -1.99f, -0.5f,  -1.2f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  // Dole-Desno
           -1.99f, -0.3f,  -1.2f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  // Gore-Desno
           -1.99f, -0.3f,  -0.7f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  // Gore-Levo
           -1.99f, -0.3f,  -0.7f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  // Gore-Levo
           -1.99f, -0.5f,  -0.7f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  // Dole-Levo
           -1.99f, -0.5f,  -1.2f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f   // Dole-Desno
    };

    float lampButtonStride = (3 + 3 + 2) * sizeof(float);

    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lampButtonVertices), lampButtonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, lampButtonStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, lampButtonStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributi za teksture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, lampButtonStride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // CISCENJE
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ------------------------------- TEKSTURE -------------------------------

    // PREDNJI ZID
    unsigned frontWallPicturesTextures[4];

    //Mona Lisa
    frontWallPicturesTextures[0] = loadImageToTexture("res/front_wall/Mona_Lisa.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    //The Raft of the Medusa
    frontWallPicturesTextures[1] = loadImageToTexture("res/front_wall/The_Raft_of_the_Medusa.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);

    //Liberty Leading the People
    frontWallPicturesTextures[2] = loadImageToTexture("res/front_wall/Liberty_Leading_the_People.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[2]);
    glGenerateMipmap(GL_TEXTURE_2D);

    //The Coronation of Napoleon
    frontWallPicturesTextures[3] = loadImageToTexture("res/front_wall/The_Coronation_of_Napoleon.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[3]);
    glGenerateMipmap(GL_TEXTURE_2D);

    
    // ZADNJI ZID
    unsigned backWallPicturesTextures[4];

    //Starry Night
    backWallPicturesTextures[0] = loadImageToTexture("res/back_wall/Starry_Night.png");
    glBindTexture(GL_TEXTURE_2D, backWallPicturesTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    //Wheat Field with Cypresses
    backWallPicturesTextures[1] = loadImageToTexture("res/back_wall/Wheat_Field_with_Cypresses.png");
    glBindTexture(GL_TEXTURE_2D, backWallPicturesTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);

    //Cafe Terrace at Night
    backWallPicturesTextures[2] = loadImageToTexture("res/back_wall/Cafe_Terrace_at_Night.png");
    glBindTexture(GL_TEXTURE_2D, backWallPicturesTextures[2]);
    glGenerateMipmap(GL_TEXTURE_2D);    
    
    //Almond blossom
    backWallPicturesTextures[3] = loadImageToTexture("res/back_wall/Almond_blossom.png");
    glBindTexture(GL_TEXTURE_2D, backWallPicturesTextures[3]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // POTPIS
    unsigned signatureTexture;

    signatureTexture = loadImageToTexture("res/other/Signature.png");
    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    // DUGMAD ZA PODNU LAMPU
    unsigned floorPointLightButtonTextures[2];

    floorPointLightButtonTextures[0] = loadImageToTexture("res/other/on_toggle.png");
    glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    floorPointLightButtonTextures[1] = loadImageToTexture("res/other/off_toggle.png");
    glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // SOBA
    unsigned roomTextures[3];

    roomTextures[0] = loadImageToTexture("res/room/marble.png");
    glBindTexture(GL_TEXTURE_2D, roomTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    roomTextures[1] = loadImageToTexture("res/room/ceiling_floor_diffuse.png");
    glBindTexture(GL_TEXTURE_2D, roomTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);

    roomTextures[2] = loadImageToTexture("res/room/ceiling_floor_specular.png");
    glBindTexture(GL_TEXTURE_2D, roomTextures[2]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // CISCENJE
    glBindTexture(GL_TEXTURE_2D, 0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    // 
    // Pictures stopping button
    bool stopButtonOn = false;
    // Lights
    bool cameraSpotLightOn = true;
    bool floorPointLightOn = true;
    bool useGouraud = false;
    // Progress bar
    float progressBarValue = 0.0f;
    // Pictures and frames
    float rotationRadius = 0.1f;
    float baseRotationSpeed = 1.0f;
    float maxRotationSpeed = 10.0f;
    // Postavljanje svetala
    setupSceneLights(phongMaterialShader, cameraSpotLightOn, floorPointLightOn);
    setupSceneLights(phongTextureShader, cameraSpotLightOn, floorPointLightOn);
    setupSceneLights(phongPictureShader, cameraSpotLightOn, floorPointLightOn);
    setupSceneLights(gouraudMaterialShader, cameraSpotLightOn, floorPointLightOn);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processCommonInput(window);
        processSceneInput(window, useGouraud);
        processStopButtonInput(window, stopButtonOn);
        processProgressBarInput(window, progressBarValue, progressStep);
        processCameraSpotLightInput(window, cameraSpotLightOn);
        processLampPointLightInput(window, floorPointLightOn);


        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Priprema phong material shadera
        setupSceneLights(phongMaterialShader, cameraSpotLightOn, floorPointLightOn);
        setupMVP(phongMaterialShader, model, view, projection);

        //Priprema phong texture shadera
        setupSceneLights(phongTextureShader, cameraSpotLightOn, floorPointLightOn);
        setupMVP(phongTextureShader, model, view, projection);

        //Priprema phong picture shadera
        setupSceneLights(phongPictureShader, cameraSpotLightOn, floorPointLightOn);
        setupMVP(phongPictureShader, model, view, projection);        
        
        //Priprema gouraud material shadera
        setupSceneLights(gouraudMaterialShader, cameraSpotLightOn, floorPointLightOn);
        setupMVP(gouraudMaterialShader, model, view, projection);

        phongPictureShader.use();
        phongPictureShader.setFloat("uTime", currentFrame);;
        glUseProgram(0);


        //Crtanje sobe
        drawRoom(VAO[0], phongTextureShader, roomTextures);


        //Crtanje slika prednjeg zida sa okvirima
        float currentRotationSpeed = baseRotationSpeed + (maxRotationSpeed - baseRotationSpeed) * progressBarValue;
        float angle = fmod(currentFrame * currentRotationSpeed, 2.0f * 3.14159265358979323846f);
        drawFrontWallPictures(VAO[1], phongPictureShader, frontWallPicturesTextures, stopButtonOn, angle, rotationRadius);


        //Crtanje slika zadnjeg zida sa okvirima
        drawBackWallPictures(VAO[2], phongPictureShader, backWallPicturesTextures);


        //Crtanje dugmeta za zaustavljanje slika
        drawStoppingButton(VAO[3], useGouraud ? gouraudMaterialShader : phongMaterialShader, stopButtonOn);

        //Crtanje progress bar-a
        drawProgressBar(VAO[4], useGouraud ? gouraudMaterialShader : phongMaterialShader, progressBarValue, progressBarQuadsNum);


        //Crtanje dugmeta za podno svetlo
        drawFloorLightButton(VAO[6], phongTextureShader, floorPointLightButtonTextures, floorPointLightOn);


        //Crtanje modela podnog svetla
        drawFloorLight(floorLightModel, phongTextureShader);
        

        //Crtanje modela svetla na plafonu
        drawCeilingLight(ceilingLightModel, phongTextureShader);


        //Crtanje potpisa
        drawSignature(VAO[5], signatureShader, signatureTexture);


        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    //Brisanje bafera i shader-a
    glDeleteBuffers(7, VBO);
    glDeleteVertexArrays(7, VAO);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processCommonInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

void processSceneInput(GLFWwindow* window, bool& useGouraud) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        useGouraud = false;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        useGouraud = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        useGouraud = false;
    }
}

void processStopButtonInput(GLFWwindow* window, bool& stopButtonOn) {

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        stopButtonOn = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        stopButtonOn = false;
    }
}

void processProgressBarInput(GLFWwindow* window, float& progressValue, float progressStep) {

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        progressValue += progressStep;
        progressValue = progressValue > 1.0f ? 1.0f : progressValue;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        progressValue -= progressStep;
        progressValue = progressValue < 0.0f ? 0.0f : progressValue;
    }
}

void processCameraSpotLightInput(GLFWwindow* window, bool& cameraSpotLightOn) 
{
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        cameraSpotLightOn = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        cameraSpotLightOn = false;
    }
}


void processLampPointLightInput(GLFWwindow* window, bool& lampPointLightOn) 
{
    if (camera.Front.x < -0.1f && -1.5 <= camera.Position.z && camera.Position.z <= -0.5 && camera.Position.x < -0.5) {
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            lampPointLightOn = true;
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            lampPointLightOn = false;
        }
    }
}

void setupLight(Shader& shader, LightPosition lightPosition, bool turnOn, float lightIntensity) {
    shader.use();
    if (lightPosition == LightPosition::CEILING) {
        shader.setVec3("uCeilingSpotLight.position", glm::vec3(0.0f, 0.8f, -0.15f));
        shader.setVec3("uCeilingSpotLight.direction", glm::vec3(0.0f, -0.45f, -0.90f));
        shader.setFloat("uCeilingSpotLight.cutOff", glm::cos(glm::radians(50.0f)));
        shader.setFloat("uCeilingSpotLight.outerCutOff", glm::cos(glm::radians(55.0f)));
        shader.setFloat("uCeilingSpotLight.constant", 1.0f);
        shader.setFloat("uCeilingSpotLight.linear", 0.09f);
        shader.setFloat("uCeilingSpotLight.quadratic", 0.032f);
        if (turnOn) {
            shader.setVec3("uCeilingSpotLight.ambient", lightIntensity * 1.0f, lightIntensity * 1.0f, lightIntensity * 1.0f);
            shader.setVec3("uCeilingSpotLight.diffuse", lightIntensity * 1.0f, lightIntensity * 1.0f, lightIntensity * 1.0f);
            shader.setVec3("uCeilingSpotLight.specular", lightIntensity * 1.0f, lightIntensity * 1.0f, lightIntensity * 1.0f);
        }
        else {
            shader.setVec3("uCeilingSpotLight.ambient", 0.0f, 0.0f, 0.0f);
            shader.setVec3("uCeilingSpotLight.diffuse", 0.0f, 0.0f, 0.0f);
            shader.setVec3("uCeilingSpotLight.specular", 0.0f, 0.0f, 0.0f);
        }
    }
    else if (lightPosition == LightPosition::FLOOR) {
        shader.setVec3("uFloorPointLight.position", glm::vec3(-1.8f, -0.85f, 0.0f));
        shader.setFloat("uFloorPointLight.constant", 1.0f);
        shader.setFloat("uFloorPointLight.linear", 0.09f);
        shader.setFloat("uFloorPointLight.quadratic", 0.032f);
        if (turnOn) {
            shader.setVec3("uFloorPointLight.ambient", lightIntensity * 0.7f, lightIntensity * 0.4f, lightIntensity * 0.0f);
            shader.setVec3("uFloorPointLight.diffuse", lightIntensity * 0.9f, lightIntensity * 0.5f, lightIntensity * 0.2f);
            shader.setVec3("uFloorPointLight.specular", lightIntensity * 1.0f, lightIntensity * 0.7f, lightIntensity * 0.5f);
        }
        else {
            shader.setVec3("uFloorPointLight.ambient", 0.0f, 0.0f, 0.0f);
            shader.setVec3("uFloorPointLight.diffuse", 0.0f, 0.0f, 0.0f);
            shader.setVec3("uFloorPointLight.specular", 0.0f, 0.0f, 0.0f);
        }
    }
    else if (lightPosition == LightPosition::CAMERA) {
        shader.setVec3("uCameraSpotLight.position", camera.Position);
        shader.setVec3("uCameraSpotLight.direction", camera.Front);
        shader.setVec3("uViewPos", camera.Position);
        shader.setFloat("uCameraSpotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shader.setFloat("uCameraSpotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        shader.setFloat("uCameraSpotLight.constant", 1.0f);
        shader.setFloat("uCameraSpotLight.linear", 0.09f);
        shader.setFloat("uCameraSpotLight.quadratic", 0.032f);
        if (turnOn) {
            shader.setVec3("uCameraSpotLight.ambient", lightIntensity * 0.2f, lightIntensity * 0.0f, lightIntensity * 0.8f);
            shader.setVec3("uCameraSpotLight.diffuse", lightIntensity * 0.4f, lightIntensity * 0.0f, lightIntensity * 0.8f);
            shader.setVec3("uCameraSpotLight.specular", lightIntensity * 0.6f, lightIntensity * 0.2f, lightIntensity * 1.0f);
        }
        else {
            shader.setVec3("uCameraSpotLight.ambient", 0.0f, 0.0f, 0.0f);
            shader.setVec3("uCameraSpotLight.diffuse", 0.0f, 0.0f, 0.0f);
            shader.setVec3("uCameraSpotLight.specular", 0.0f, 0.0f, 0.0f);
        }
    }
    glUseProgram(0);
}

void setupSceneLights(Shader& shader, bool cameraSpotLightOn, bool floorPointLightOn) 
{
    float ceilingSpotLightIntensity = 0.3f;
    float floorPointLightIntensity = 0.3f;
    float cameraSpotLightIntensity = 0.3f;
    shader.use();
    if (cameraSpotLightOn) {
        setupLight(shader, LightPosition::CAMERA, true, cameraSpotLightIntensity);
    }
    else {
        setupLight(shader, LightPosition::CAMERA, false, cameraSpotLightIntensity);
    }
    if (floorPointLightOn) {
        setupLight(shader, LightPosition::FLOOR, true, floorPointLightIntensity);
    }
    else {
        setupLight(shader, LightPosition::FLOOR, false, floorPointLightIntensity);
    }
    setupLight(shader, LightPosition::CEILING, true, ceilingSpotLightIntensity);
    glUseProgram(0);
}

void setupMVP(Shader& shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    shader.use();
    shader.setMat4("uM", model);
    shader.setMat4("uV", view);
    shader.setMat4("uP", projection);
    glUseProgram(0);
}

void drawRoom(int VAO, Shader shader, unsigned roomTextures[]) {
    shader.use();
    shader.setInt("uMaterial.diffuse", 0);
    shader.setInt("uMaterial.specular", 0);
    shader.setFloat("uMaterial.shininess", 64.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, roomTextures[0]);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, roomTextures[1]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, roomTextures[2]);
    shader.setInt("uMaterial.specular", 1);
    glDrawArrays(GL_TRIANGLES, 24, 36);
    glUseProgram(0);
}


void drawFrontWallPictures(int VAO, Shader shader, unsigned frontWallPicturesTextures[], bool stopButtonOn, float angle, float rotationRadius) {
    shader.use();
    for (int i = 0; i < 4; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[i]);
        shader.setInt("uTex", i);
        shader.setVec2("uXYMovement", stopButtonOn ? 0 : (cos(angle) * rotationRadius), stopButtonOn ? 0 : (sin(angle) * rotationRadius));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }
    glUseProgram(0);
}

void drawBackWallPictures(int VAO, Shader shader, unsigned backWallPicturesTextures[])
{
    setupLight(shader, LightPosition::FLOOR, false, 0.0f);
    setupLight(shader, LightPosition::CEILING, false, 0.0f);
    shader.use();
    for (int i = 0; i < 4; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, backWallPicturesTextures[i]);
        shader.setInt("uTex", i);
        shader.setVec2("uXYMovement", 0, 0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }
    glUseProgram(0);
}

void drawStoppingButton(int VAO, Shader shader, bool stopButtonOn) {
    shader.use();
    if (stopButtonOn) {
        shader.setFloat("uMaterial.shininess", 0.25 * 128.0f);
        shader.setVec3("uMaterial.ambient", 0.0f, 0.0f, 0.0f);
        shader.setVec3("uMaterial.diffuse", 0.01f, 0.01f, 0.01);
        shader.setVec3("uMaterial.specular", 0.50f, 0.50f, 0.50f);
    }
    else
    {
        shader.setFloat("uMaterial.shininess", 0.4 * 128.0f);
        shader.setVec3("uMaterial.ambient", 0.24725f, 0.1995f, 0.0745f);
        shader.setVec3("uMaterial.diffuse", 0.75164f, 0.60648f, 0.22648f);
        shader.setVec3("uMaterial.specular", 0.628281f, 0.555802f, 0.366065f);
    }
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);
    glUseProgram(0);
}


void drawProgressBar(int VAO, Shader shader, float progressBarValue, int progressBarQuadsNum) {
    shader.use();
    shader.setFloat("uMaterial.shininess", 0.6 * 128.0f);
    shader.setVec3("uMaterial.diffuse", 0.1745, 0.01175, 0.01175);
    shader.setVec3("uMaterial.diffuse", 0.61424f, 0.04136f, 0.04136f);
    shader.setVec3("uMaterial.specular", 0.727811f, 0.626959f, 0.626959f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, progressBarValue * progressBarQuadsNum * 2 * 3);
    shader.setFloat("uMaterial.shininess", 0.25 * 128.0f);
    shader.setVec3("uMaterial.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("uMaterial.diffuse", 0.01f, 0.01f, 0.01);
    shader.setVec3("uMaterial.specular", 0.50f, 0.50f, 0.50f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, progressBarValue * progressBarQuadsNum * 2 * 3, progressBarQuadsNum * 2 * 3);
    glUseProgram(0);
}


void drawFloorLightButton(int VAO, Shader shader, unsigned floorPointLightButtonTextures[], bool floorPointLightOn) {
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    if (floorPointLightOn) {
        glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTextures[0]);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTextures[1]);
    }
    shader.setInt("uMaterial.diffuse", 0);
    shader.setInt("uMaterial.specular", 0);
    shader.setFloat("uMaterial.shininess", 32.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
}

void drawFloorLight(Model& floorLightModel, Shader shader) {
    shader.use();
    glm::mat4 floorLightModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.8f, -1.0f, 0.0f));
    floorLightModelMatrix = glm::scale(floorLightModelMatrix, glm::vec3(0.005f));
    shader.setMat4("uM", floorLightModelMatrix);
    floorLightModel.Draw(shader);
    glUseProgram(0);
}

void drawCeilingLight(Model& ceilingLightModel, Shader shader) {
    shader.use();
    glm::mat4 ceilingLightModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ceilingLightModelMatrix = glm::rotate(ceilingLightModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ceilingLightModelMatrix = glm::scale(ceilingLightModelMatrix, glm::vec3(0.01f));
    shader.setMat4("uM", ceilingLightModelMatrix);
    ceilingLightModel.Draw(shader);
    glUseProgram(0);
}

void drawSignature(int VAO, Shader shader, unsigned signatureTexture) {
    shader.use();
    shader.setMat4("uM", glm::mat4(1.0f));
    shader.setMat4("uP", glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));
    shader.setMat4("uV", glm::mat4(1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    shader.setInt("uTex", 0);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
}


static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}