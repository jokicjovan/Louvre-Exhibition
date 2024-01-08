#define _CRT_SECURE_NO_WARNINGS
#define CRES 30
//Biblioteke za stvari iz C++-a (unos, ispis, fajlovi itd) 
#include <iostream>
#include <fstream>
#include <sstream>

//Biblioteke OpenGL-a
#include <GL/glew.h>   //Omogucava upotrebu OpenGL naredbi
#include <GLFW/glfw3.h>//Olaksava pravljenje i otvaranje prozora (konteksta) sa OpenGL sadrzajem

//stb_image.h je header-only biblioteka za ucitavanje tekstura.
//Potrebno je definisati STB_IMAGE_IMPLEMENTATION prije njenog ukljucivanja
#define STB_IMAGE_IMPLEMENTATION
#include "camera.h"
#include "shader.h"
#include "model.h"

enum LightPosition {
    CEILING,
    FLOOR,
    CAMERA
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processCommonInput(GLFWwindow* window);
void processSceneInput(GLFWwindow* window);
void processStopButtonInput(GLFWwindow* window, bool& stopButtonOn);
void processProgressBarInput(GLFWwindow* window, float& progressValue, float progressStep);
void processCameraSpotLightInput(GLFWwindow*  window, bool& cameraSpotLightOn);
void processLampPointLightInput(GLFWwindow* window, bool& lampPointLightOn);

void setupLight(Shader& shader, LightPosition lightType, float lightIntensity, bool turnOn = true);

static unsigned loadImageToTexture(const char* filePath);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 900;

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

    // Pokretanje GLFW biblioteke
    // Nju koristimo za stvaranje okvira prozora
    if (!glfwInit()) // !0 == 1  | glfwInit inicijalizuje GLFW i vrati 1 ako je inicijalizovana uspjesno, a 0 ako nije
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    //Odredjivanje OpenGL verzije i profila (3.3, programabilni pajplajn)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Stvaranje prozora
    GLFWwindow* window; //Mjesto u memoriji za prozor
    const char wTitle[] = "Louvre Exhibition 3D";
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, wTitle, NULL, NULL); // Napravi novi prozor
    // glfwCreateWindow( sirina, visina, naslov, monitor na koji ovaj prozor ide preko citavog ekrana (u tom slucaju umjesto NULL ide glfwGetPrimaryMonitor() ), i prozori sa kojima ce dijeliti resurse )
    if (window == NULL) //Ako prozor nije napravljen
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate(); //Gasi GLFW
        return 2; //Vrati kod za gresku
    }
    // Postavljanje novopecenog prozora kao aktivni (sa kojim cemo da radimo)
    glfwMakeContextCurrent(window);

    // Inicijalizacija GLEW biblioteke
    if (glewInit() != GLEW_OK) //Slicno kao glfwInit. GLEW_OK je predefinisani kod za uspjesnu inicijalizaciju sadrzan unutar biblioteke
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
    Shader lightingShader("lighting.vert", "lighting.frag");
    Shader wallPictureShader("wall_picture.vert", "wall_picture.frag");
    Shader progressShader("progress.vert", "progress.frag");
    Shader signatureShader("signature.vert", "signature.frag");

    unsigned int VAO[7];
    glGenVertexArrays(7, VAO);
    unsigned int VBO[7];
    glGenBuffers(7, VBO);

    // ------------------------------- SOBA -------------------------------
    float roomVertices[] = {
        //X      Y       Z       NX     NY     NZ
        //FRONT
        -2.0f,  -1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,
         2.0f,  -1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,
         2.0f,   1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,
         2.0f,   1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,
        -2.0f,   1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,
        -2.0f,  -1.0f,  -2.0f,   0.0f,  0.0f,  1.0f,
                                    
        //BACK                      
         2.0f,   1.0f,   2.0f,   0.0f,  0.0f, -1.0f,
        -2.0f,  -1.0f,   2.0f,   0.0f,  0.0f, -1.0f,
        -2.0f,   1.0f,   2.0f,   0.0f,  0.0f, -1.0f,
         2.0f,   1.0f,   2.0f,   0.0f,  0.0f, -1.0f,
         2.0f,  -1.0f,   2.0f,   0.0f,  0.0f, -1.0f,
        -2.0f,  -1.0f,   2.0f,   0.0f,  0.0f, -1.0f,
                 
        //LEFT   
        -2.0f, - 1.0f,  -2.0f,   1.0f,  0.0f,  0.0f,
        -2.0f,   1.0f,  -2.0f,   1.0f,  0.0f,  0.0f,
        -2.0f,   1.0f,   2.0f,   1.0f,  0.0f,  0.0f,
        -2.0f,   1.0f,   2.0f,   1.0f,  0.0f,  0.0f,
        -2.0f, - 1.0f,   2.0f,   1.0f,  0.0f,  0.0f,
        -2.0f, - 1.0f,  -2.0f,   1.0f,  0.0f,  0.0f,
                       
        //RIGHT            
         2.0f,   1.0f,   2.0f,  -1.0f,  0.0f,  0.0f,
         2.0f,   1.0f,  -2.0f,  -1.0f,  0.0f,  0.0f,
         2.0f,  -1.0f,  -2.0f,  -1.0f,  0.0f,  0.0f,
         2.0f,  -1.0f,  -2.0f,  -1.0f,  0.0f,  0.0f,
         2.0f,  -1.0f,   2.0f,  -1.0f,  0.0f,  0.0f,
         2.0f,   1.0f,   2.0f,  -1.0f,  0.0f,  0.0f,
                       
        //BOTTOM       
        -2.0f,  -1.0f,  -2.0f,   0.0f,  1.0f,  0.0f,
        -2.0f,  -1.0f,   2.0f,   0.0f,  1.0f,  0.0f,
         2.0f,  -1.0f,   2.0f,   0.0f,  1.0f,  0.0f,
         2.0f,  -1.0f,   2.0f,   0.0f,  1.0f,  0.0f,
         2.0f,  -1.0f,  -2.0f,   0.0f,  1.0f,  0.0f,
        -2.0f,  -1.0f,  -2.0f,   0.0f,  1.0f,  0.0f,
                                    
        //TOP                       
        -2.0f,   1.0f,  -2.0f,   0.0f, -1.0f,  0.0f,
         2.0f,   1.0f,  -2.0f,   0.0f, -1.0f,  0.0f,
         2.0f,   1.0f,   2.0f,   0.0f, -1.0f,  0.0f,
         2.0f,   1.0f,   2.0f,   0.0f, -1.0f,  0.0f,
        -2.0f,   1.0f,   2.0f,   0.0f, -1.0f,  0.0f,
        -2.0f,   1.0f,  -2.0f,   0.0f, -1.0f,  0.0f
    };

    unsigned int roomStride = (3 + 3) * sizeof(float);

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
        (3.8f - 5 * backWallPictureOffset) / backWallPicturesTotalWidth : 1.0f;     // kreiranje "skalera" za slike 
                                                                                    //(uzima u obzir sirinu slika i offset-e izmedju njih)
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

    //Podesavanje
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

    // ------------------------------- DUGME -------------------------------

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

    //Podesavanje
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, roomStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, roomStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ------------------------------- PROGRESS BAR -------------------------------

    float progressBarVertices[] = {
        //  X      Y       Z
           -1.9f,  0.7f,  -1.99f,   // Gore-Levo
            0.1f,  0.7f,  -1.99f,   // Dole-Levo
           -1.9f,  0.8f,  -1.99f,   // Gore-Desno
            0.1f,  0.8f,  -1.99f    // Dole-Desno
    };

    float progressBarStride = 3 * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, progressBarStride, (void*)0);
    glEnableVertexAttribArray(0);

    // ------------------------------- POTPIS -------------------------------

    float signatureVertices[] = {
        // X        Y       S    T
           0.45f,  -0.6f,   0.0, 1.0,  // Gore-Levo
           0.45f,  -0.9f,   0.0, 0.0,  // Dole-Levo
           0.9f,   -0.6f,   1.0, 1.0,  // Gore-Desno
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

    // ------------------------------- LAMPA -------------------------------

    Model lamp("res/lamp/model.dae");

    // ------------------------------- DUGME ZA LAMPU -------------------------------

    float lampButtonVertices[] = {
        //  X       Y       Z      NX     NY     NZ
           -1.99f, -0.5f,  -1.0f,  0.0f,  1.0f,  0.0f,  // Dole-Desno
           -1.99f, -0.3f,  -1.0f,  0.0f,  1.0f,  0.0f,  // Gore-Desno
           -1.99f, -0.3f,  -0.9f,  0.0f,  1.0f,  0.0f,  // Gore-Levo
           -1.99f, -0.3f,  -0.9f,  0.0f,  1.0f,  0.0f,  // Gore-Levo
           -1.99f, -0.5f,  -0.9f,  0.0f,  1.0f,  0.0f,  // Dole-Levo
           -1.99f, -0.5f,  -1.0f,  0.0f,  1.0f,  0.0f   // Dole-Desno
    };

    float lampButtonStride = (3 + 3) * sizeof(float);

    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lampButtonVertices), lampButtonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, lampButtonStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, lampButtonStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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


    // CISCENJE
    glBindTexture(GL_TEXTURE_2D, 0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    
    // Postavljanje svetala
    float ceilingSpotLightIntensity = 0.3f;
    float floorPointLightIntensity = 0.3f;
    float cameraSpotLightIntensity = 0.3f;
    setupLight(lightingShader, LightPosition::CEILING, ceilingSpotLightIntensity, true);
    setupLight(lightingShader, LightPosition::FLOOR, floorPointLightIntensity, true);
    setupLight(lightingShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
    setupLight(wallPictureShader, LightPosition::CEILING, ceilingSpotLightIntensity, true);
    setupLight(wallPictureShader, LightPosition::FLOOR, floorPointLightIntensity, true);
    setupLight(wallPictureShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);

    // Button
    bool stopButtonOn = false;
    bool cameraSpotLightOn = true;
    bool floorPointLightOn = true;
    // Progress bar
    float progressStep = 0.01f;
    float progressValue = 0.0f;
    // Pictures and frames
    float rotationRadius = 0.1f;
    float baseRotationSpeed = 1.0f;
    float maxRotationSpeed = 10.0f;
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processCommonInput(window);
        processSceneInput(window);
        processStopButtonInput(window, stopButtonOn);   
        processProgressBarInput(window, progressValue, progressStep);
        processCameraSpotLightInput(window, cameraSpotLightOn);
        processLampPointLightInput(window, floorPointLightOn);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();


        //Priprema ligthing shadera za crtanje sobe
        if (cameraSpotLightOn) {
            setupLight(lightingShader, LightPosition::CAMERA, cameraSpotLightIntensity, true);
        }
        else {
            setupLight(lightingShader, LightPosition::CAMERA, cameraSpotLightIntensity, false);
        }
        if (floorPointLightOn) {
            setupLight(lightingShader, LightPosition::FLOOR, floorPointLightIntensity, true);
        }
        else {
            setupLight(lightingShader, LightPosition::FLOOR, floorPointLightIntensity, false);
        }
        lightingShader.use();
        lightingShader.setMat4("uM", model);
        lightingShader.setMat4("uP", projection);
        lightingShader.setMat4("uV", view);
        if (stopButtonOn) {
            lightingShader.setFloat("uMaterial.shininess", 0.6 * 128); // Dark Stone
            lightingShader.setVec3("uMaterial.diffuse", 0.3, 0.3, 0.3);
            lightingShader.setVec3("uMaterial.specular", 0.5, 0.5, 0.5);
        }
        else
        {
            lightingShader.setFloat("uMaterial.shininess", 0.6 * 128); // Marble
            lightingShader.setVec3("uMaterial.diffuse", 0.8f, 0.8f, 0.8f);
            lightingShader.setVec3("uMaterial.specular", 0.1f, 0.1f, 0.1f);
        }
        glUseProgram(0);

        //Crtanje sobe
        lightingShader.use();
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glUseProgram(0);

        //Priprema wall_picture shadera za crtanje slika prednjeg zida
        float currentRotationSpeed = baseRotationSpeed + (maxRotationSpeed - baseRotationSpeed) * progressValue;
        float angle = fmod(currentFrame * currentRotationSpeed, 2.0f * 3.14159265358979323846f);
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

        //Crtanje slika prednjeg zida sa okvirima
        wallPictureShader.use();
        for (int i = 0; i < 4; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[i]);
            wallPictureShader.setInt("uTex", i);
            wallPictureShader.setVec2("uCircularPosition", stopButtonOn ? 0 : (cos(angle) * rotationRadius), stopButtonOn ? 0 : (sin(angle) * rotationRadius));
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
            wallPictureShader.setVec2("uCircularPosition", 0 ,0);
            glBindVertexArray(VAO[2]);
            glDrawArrays(GL_TRIANGLES, i * 6, 6);
        }
        glUseProgram(0);

        //Crtanje dugmeta
        lightingShader.use();
        if (stopButtonOn) {
            lightingShader.setFloat("uMaterial.shininess", 1.0f);
            lightingShader.setVec3("uMaterial.diffuse", 0.0f, 0.0f, 0.0f);
            lightingShader.setVec3("uMaterial.specular", 0.0f, 0.0f, 0.0f);
        }
        else 
        {
            lightingShader.setFloat("uMaterial.shininess", 1.0f);
            lightingShader.setVec3("uMaterial.diffuse", 1.0f, 1.0f, 0.2f);
            lightingShader.setVec3("uMaterial.specular", 0.3f, 0.3f, 0.1f);
        }
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);
        glUseProgram(0);

        //Crtanje progress bar-a
        progressShader.use();
        progressShader.setMat4("uM", model);
        progressShader.setMat4("uP", projection);
        progressShader.setMat4("uV", view);
        progressShader.setFloat("uProgressValue", progressValue);
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUseProgram(0);

        //Crtanje dugmeta za lampu
        lightingShader.use();
        if (floorPointLightOn) {
            lightingShader.setFloat("uMaterial.shininess", 1.0f);
            lightingShader.setVec3("uMaterial.diffuse", 0.0f, 1.0f, 0.0f);
            lightingShader.setVec3("uMaterial.specular", 0.0f, 1.0f, 0.0f);
        }
        else
        {
            lightingShader.setFloat("uMaterial.shininess", 1.0f);
            lightingShader.setVec3("uMaterial.diffuse", 1.0f, 0.0f, 0.0f);
            lightingShader.setVec3("uMaterial.specular", 1.0f, 0.0f, 0.0f);
        }
        glBindVertexArray(VAO[6]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Crtanje lampe
        lightingShader.use();
        glm::mat4 lampModelMatrix = glm::translate(model, glm::vec3(-1.85f, -1.0f, 0.0f)) * glm::scale(model, glm::vec3(0.005f));
        lightingShader.setMat4("uM", lampModelMatrix);
        lamp.Draw(lightingShader);
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


    //Brisanje bafera i sejdera
    glDeleteBuffers(7, VBO);
    glDeleteVertexArrays(7, VAO);
    //Sve OK - batali program
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

void processSceneInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
        shader.setVec3("uCeilingSpotLight.position", glm::vec3(0.0f, 0.9f, 0.0f));
        shader.setVec3("uCeilingSpotLight.direction", glm::vec3(0.0f, -0.9f, -1.5f));
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
        shader.setVec3("uFloorPointLight.position", glm::vec3(-1.85f, -1.0f, 0.0f));
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
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
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
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
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