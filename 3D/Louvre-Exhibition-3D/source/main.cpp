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

void setupLight(Shader& shader, LightPosition lightType, float lightIntensity, bool turnOn = true);

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
    Shader lightingMaterialShader("shaders/lighting_material.vert", "shaders/lighting_material.frag");
    Shader lightingTextureShader("shaders/lighting_texture.vert", "shaders/lighting_texture.frag");
    Shader wallPictureShader("shaders/wall_picture.vert", "shaders/wall_picture.frag");
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
        // X        Y       S    T
           0.5f,   -0.7f,   0.0, 1.0,  // Gore-Levo
           0.5f,   -0.9f,   0.0, 0.0,  // Dole-Levo
           0.9f,   -0.7f,   1.0, 1.0,  // Gore-Desno
           0.9f,   -0.9f,   1.0, 0.0   // Dole-Desno
    };

    float signatureStride = (2 + 2) * sizeof(float);

    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureVertices), signatureVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)(2 * sizeof(float)));
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
    unsigned floorPointLightButtonTexture[2];

    floorPointLightButtonTexture[0] = loadImageToTexture("res/other/on_toggle.png");
    glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTexture[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    floorPointLightButtonTexture[1] = loadImageToTexture("res/other/off_toggle.png");
    glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTexture[1]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // SOBA
    unsigned roomTexture[3];

    roomTexture[0] = loadImageToTexture("res/room/marble.png");
    glBindTexture(GL_TEXTURE_2D, roomTexture[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    roomTexture[1] = loadImageToTexture("res/room/ceiling_floor_diffuse.png");
    glBindTexture(GL_TEXTURE_2D, roomTexture[1]);
    glGenerateMipmap(GL_TEXTURE_2D);

    roomTexture[2] = loadImageToTexture("res/room/ceiling_floor_specular.png");
    glBindTexture(GL_TEXTURE_2D, roomTexture[2]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // CISCENJE
    glBindTexture(GL_TEXTURE_2D, 0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    
    // Postavljanje svetala
    float ceilingSpotLightIntensity = 0.3f;
    float floorPointLightIntensity = 0.3f;
    float cameraSpotLightIntensity = 0.3f;
    setupLight(lightingMaterialShader, LightPosition::CEILING, ceilingSpotLightIntensity, true);
    setupLight(lightingMaterialShader, LightPosition::FLOOR, floorPointLightIntensity, true);
    setupLight(lightingMaterialShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
    setupLight(lightingTextureShader, LightPosition::CEILING, ceilingSpotLightIntensity, true);
    setupLight(lightingTextureShader, LightPosition::FLOOR, floorPointLightIntensity, true);
    setupLight(lightingTextureShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
    setupLight(wallPictureShader, LightPosition::CEILING, ceilingSpotLightIntensity, true);
    setupLight(wallPictureShader, LightPosition::FLOOR, floorPointLightIntensity, true);
    setupLight(wallPictureShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);

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

        if (useGouraud) {
            lightingMaterialShader.use();
            lightingMaterialShader.setBool("uUseGouraud", true);
            lightingTextureShader.use();
            lightingTextureShader.setBool("uUseGouraud", true);
        }
        else {
            lightingMaterialShader.use();
            lightingMaterialShader.setBool("uUseGouraud", false);
            lightingTextureShader.use();
            lightingTextureShader.setBool("uUseGouraud", false);
        }
        glUseProgram(0);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Priprema ligthing material
        if (cameraSpotLightOn) {
            setupLight(lightingMaterialShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
        }
        else {
            setupLight(lightingMaterialShader, LightPosition::CAMERA, cameraSpotLightIntensity, false);
        }
        if (floorPointLightOn) {
            setupLight(lightingMaterialShader, LightPosition::FLOOR, floorPointLightIntensity, true);
        }
        else {
            setupLight(lightingMaterialShader, LightPosition::FLOOR, floorPointLightIntensity, false);
        }
        lightingMaterialShader.use();
        lightingMaterialShader.setMat4("uM", model);
        lightingMaterialShader.setMat4("uP", projection);
        lightingMaterialShader.setMat4("uV", view);
        glUseProgram(0);

        //Priprema ligthing texture shadera
        if (cameraSpotLightOn) {
            setupLight(lightingTextureShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
        }
        else {
            setupLight(lightingTextureShader, LightPosition::CAMERA, cameraSpotLightIntensity, false);
        }
        if (floorPointLightOn) {
            setupLight(lightingTextureShader, LightPosition::FLOOR, floorPointLightIntensity, true);
        }
        else {
            setupLight(lightingTextureShader, LightPosition::FLOOR, floorPointLightIntensity, false);
        }
        lightingTextureShader.use();
        lightingTextureShader.setMat4("uM", model);
        lightingTextureShader.setMat4("uP", projection);
        lightingTextureShader.setMat4("uV", view);
        glUseProgram(0);

        //Priprema wall_picture shadera
        setupLight(wallPictureShader, LightPosition::CEILING, ceilingSpotLightIntensity, true);
        if (cameraSpotLightOn) {
            setupLight(wallPictureShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
        }
        else {
            setupLight(wallPictureShader, LightPosition::CAMERA, cameraSpotLightIntensity, false);
        }
        if (floorPointLightOn) {
            setupLight(wallPictureShader, LightPosition::FLOOR, floorPointLightIntensity, true);;
        }
        else {
            setupLight(wallPictureShader, LightPosition::FLOOR, floorPointLightIntensity, false);;
        }
        wallPictureShader.use();
        wallPictureShader.setMat4("uM", model);
        wallPictureShader.setMat4("uP", projection);
        wallPictureShader.setMat4("uV", view);
        wallPictureShader.setFloat("uTime", currentFrame);;
        glUseProgram(0);



        //Crtanje sobe
        lightingTextureShader.use();
        lightingTextureShader.setInt("uMaterial.diffuse", 0);
        lightingTextureShader.setInt("uMaterial.specular", 0);
        lightingTextureShader.setFloat("uMaterial.shininess", 64.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, roomTexture[0]);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 24);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, roomTexture[1]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, roomTexture[2]);
        lightingTextureShader.setInt("uMaterial.specular", 1);
        glDrawArrays(GL_TRIANGLES, 24, 36);
        glUseProgram(0);

        //Priprema za crtanje slika prednjeg zida
        float currentRotationSpeed = baseRotationSpeed + (maxRotationSpeed - baseRotationSpeed) * progressBarValue;
        float angle = fmod(currentFrame * currentRotationSpeed, 2.0f * 3.14159265358979323846f);

        //Crtanje slika prednjeg zida sa okvirima
        wallPictureShader.use();
        for (int i = 0; i < 4; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[i]);
            wallPictureShader.setInt("uTex", i);
            wallPictureShader.setVec2("uXYMovement", stopButtonOn ? 0 : (cos(angle) * rotationRadius), stopButtonOn ? 0 : (sin(angle) * rotationRadius));
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_TRIANGLES, i * 6, 6);
        }
        glUseProgram(0);

        //Priprema wall_picture shadera za crtanje slika zadnjeg zida
        setupLight(wallPictureShader, LightPosition::FLOOR, floorPointLightIntensity, false);;
        setupLight(wallPictureShader, LightPosition::CEILING, ceilingSpotLightIntensity, false);;

        //Crtanje slika zadnjeg zida sa okvirima
        wallPictureShader.use();
        for (int i = 0; i < 4; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, backWallPicturesTextures[i]);
            wallPictureShader.setInt("uTex", i);
            wallPictureShader.setVec2("uXYMovement", 0, 0);
            glBindVertexArray(VAO[2]);
            glDrawArrays(GL_TRIANGLES, i * 6, 6);
        }
        glUseProgram(0);


        //Crtanje dugmeta za zaustavljanje slika
        lightingMaterialShader.use();
        if (stopButtonOn) {
            lightingMaterialShader.setFloat("uMaterial.shininess", 1.0f);
            lightingMaterialShader.setVec3("uMaterial.diffuse", 0.0f, 0.0f, 0.0f);
            lightingMaterialShader.setVec3("uMaterial.specular", 0.0f, 0.0f, 0.0f);
        }
        else
        {
            lightingMaterialShader.setFloat("uMaterial.shininess", 1.0f);
            lightingMaterialShader.setVec3("uMaterial.diffuse", 1.0f, 1.0f, 0.2f);
            lightingMaterialShader.setVec3("uMaterial.specular", 0.3f, 0.3f, 0.1f);
        }
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);
        glUseProgram(0);

        //Crtanje progress bar-a
        lightingMaterialShader.use();
        lightingMaterialShader.setMat4("uM", model);
        lightingMaterialShader.setMat4("uP", projection);
        lightingMaterialShader.setMat4("uV", view);
        lightingMaterialShader.setFloat("uMaterial.shininess", 1.0f);
        lightingMaterialShader.setVec3("uMaterial.diffuse", 1.0f, 0.0f, 0.0f);
        lightingMaterialShader.setVec3("uMaterial.specular", 0.5f, 0.0f, 0.0f);
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLES, 0, progressBarValue * progressBarQuadsNum * 2 * 3);
        lightingMaterialShader.setFloat("uMaterial.shininess", 1.0f);
        lightingMaterialShader.setVec3("uMaterial.diffuse", 0.0f, 0.0f, 0.0f);
        lightingMaterialShader.setVec3("uMaterial.specular", 0.0f, 0.0f, 0.0f);
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLES, progressBarValue * progressBarQuadsNum * 2 * 3, progressBarQuadsNum * 2 * 3);
        glUseProgram(0);

        //Crtanje dugmeta za lampu
        lightingTextureShader.use();
        glActiveTexture(GL_TEXTURE0);
        if (floorPointLightOn) {
            glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTexture[0]);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, floorPointLightButtonTexture[1]);
        }
        lightingTextureShader.setInt("uMaterial.diffuse", 0);
        lightingTextureShader.setInt("uMaterial.specular", 0);
        lightingTextureShader.setFloat("uMaterial.shininess", 32.0f);
        glBindVertexArray(VAO[6]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glUseProgram(0);

        //Crtanje podnog svetla
        lightingTextureShader.use();
        glm::mat4 floorLightModelMatrix = glm::translate(model, glm::vec3(-1.8f, -1.0f, 0.0f));
        floorLightModelMatrix = glm::scale(floorLightModelMatrix, glm::vec3(0.005f));
        lightingTextureShader.setMat4("uM", floorLightModelMatrix);
        floorLightModel.Draw(lightingTextureShader);
        glUseProgram(0); 
        
        //Crtanje svetla na plafonu
        lightingTextureShader.use();
        glm::mat4 ceilingLightModelMatrix = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        ceilingLightModelMatrix = glm::rotate(ceilingLightModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ceilingLightModelMatrix = glm::scale(ceilingLightModelMatrix, glm::vec3(0.01f));
        lightingTextureShader.setMat4("uM", ceilingLightModelMatrix);
        ceilingLightModel.Draw(lightingTextureShader);
        glUseProgram(0);

        //Crtanje potpisa
        signatureShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        signatureShader.setInt("uTex", 0);
        glBindVertexArray(VAO[5]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUseProgram(0);

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

void setupLight(Shader& shader, LightPosition lightPosition, float lightIntensity, bool turnOn) {
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