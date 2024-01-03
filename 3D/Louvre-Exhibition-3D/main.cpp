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
#include "stb_image.h"
#include "camera.h"
#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processCommonInput(GLFWwindow* window);

static unsigned loadImageToTexture(const char* filePath);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 700;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Inicijalizacija GLEW biblioteke
    if (glewInit() != GLEW_OK) //Slicno kao glfwInit. GLEW_OK je predefinisani kod za uspjesnu inicijalizaciju sadrzan unutar biblioteke
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    Shader frameShader("frame.vert", "frame.frag");
    Shader pictureShader("picture.vert", "picture.frag");
    Shader progressShader("progress.vert", "progress.frag");
    Shader basicColorShader("basicColor.vert", "basicColor.frag");
    Shader basicTextureShader("basicTexture.vert", "basicTexture.frag");

    unsigned int VAO[5];
    glGenVertexArrays(5, VAO);
    unsigned int VBO[5];
    glGenBuffers(5, VBO);

    // ------------------------------- SOBA -------------------------------
    float roomVertices[] = {
        //X      Y       Z      NX     NY     NZ
        //BACK
        -1.0f,  -1.0f,  -1.0f,  0.0f,  0.0f, -1.0f,
         1.0f,  -1.0f,  -1.0f,  0.0f,  0.0f, -1.0f,
         1.0f,   1.0f,  -1.0f,  0.0f,  0.0f, -1.0f,
         1.0f,   1.0f,  -1.0f,  0.0f,  0.0f, -1.0f,
        -1.0f,   1.0f,  -1.0f,  0.0f,  0.0f, -1.0f,
        -1.0f,  -1.0f,  -1.0f,  0.0f,  0.0f, -1.0f,

        //FRONT
        -1.0f,  -1.0f,   1.0f,  0.0f,  0.0f,  1.0f,
         1.0f,  -1.0f,   1.0f,  0.0f,  0.0f,  1.0f,
         1.0f,   1.0f,   1.0f,  0.0f,  0.0f,  1.0f,
         1.0f,   1.0f,   1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f,   1.0f,   1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f,  -1.0f,   1.0f,  0.0f,  0.0f,  1.0f,

        //LEFT
        -1.0f,   1.0f,   1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,   1.0f,  -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, - 1.0f,  -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, - 1.0f,  -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, - 1.0f,   1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,   1.0f,   1.0f, -1.0f,  0.0f,  0.0f,

        //RIGHT
         1.0f,   1.0f,   1.0f,  1.0f,  0.0f,  0.0f,
         1.0f,   1.0f,  -1.0f,  1.0f,  0.0f,  0.0f,
         1.0f,  -1.0f,  -1.0f,  1.0f,  0.0f,  0.0f,
         1.0f,  -1.0f,  -1.0f,  1.0f,  0.0f,  0.0f,
         1.0f,  -1.0f,   1.0f,  1.0f,  0.0f,  0.0f,
         1.0f,   1.0f,   1.0f,  1.0f,  0.0f,  0.0f,

        //BOTTOM
        -1.0f,  -1.0f,  -1.0f,  0.0f, -1.0f,  0.0f,
         1.0f,  -1.0f,  -1.0f,  0.0f, -1.0f,  0.0f,
         1.0f,  -1.0f,   1.0f,  0.0f, -1.0f,  0.0f,
         1.0f,  -1.0f,   1.0f,  0.0f, -1.0f,  0.0f,
        -1.0f,  -1.0f,   1.0f,  0.0f, -1.0f,  0.0f,
        -1.0f,  -1.0f,  -1.0f,  0.0f, -1.0f,  0.0f,

        //TOP
        -1.0f,   1.0f,  -1.0f,  0.0f,  1.0f,  0.0f,
         1.0f,   1.0f,  -1.0f,  0.0f,  1.0f,  0.0f,
         1.0f,   1.0f,   1.0f,  0.0f,  1.0f,  0.0f,
         1.0f,   1.0f,   1.0f,  0.0f,  1.0f,  0.0f,
        -1.0f,   1.0f,   1.0f,  0.0f,  1.0f,  0.0f,
        -1.0f,   1.0f,  -1.0f,  0.0f,  1.0f,  0.0f
    };

    // ------------------------------- SLIKE SA OKVIRIMA NA GLAVNOM ZIDU -------------------------------
    float pictureHeight = 0.8f;
    float pictureOffset = 0.1f;

    float mainWallPicturesAspectRatios[] = { 800.0 / 1192.0, 1280.0 / 874.0, 1280.0 / 1027.0, 1920.0 / 1207.0 };
    float mainWallPicturesTotalWidth = 0.0f;
    for (int i = 0; i < 4; i++) {
        mainWallPicturesTotalWidth += (pictureHeight * mainWallPicturesAspectRatios[i]);
    }
    float mainWallPicturesScaler = (mainWallPicturesTotalWidth + 5 * pictureOffset > 2.0f) ? 
        (2.0f - 5 * pictureOffset) / mainWallPicturesTotalWidth : 1.0f;     // kreiranje "skalera" za slike 
                                                                            //(uzima u obzir sirinu slika i offset-e izmedju njih)
    float mainWallPicturesWidths[4];
    for (int i = 0; i < 4; i++) {
        mainWallPicturesWidths[i] = pictureHeight * mainWallPicturesAspectRatios[i] * mainWallPicturesScaler;
    }

    float picturesWithFramesVertices[] = {
        //X                                                                                                                                          Y                     S    T      OKVIR    R    G    B    A
        // Mona Lisa sa okvirom                                                                                                                   
        -1.0f + pictureOffset                                                                                                                     ,  pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Levo
        -1.0f + pictureOffset + mainWallPicturesWidths[0]                                                                                         ,  pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Desno
        -1.0f + pictureOffset                                                                                                                     , -pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Levo
        -1.0f + pictureOffset + mainWallPicturesWidths[0]                                                                                         , -pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Desno
                                                                                                                                                                               
        // The Raft of the Medusa sa okvirom                                                                                                                                                            
        -1.0f + 2 * pictureOffset + mainWallPicturesWidths[0]                                                                                     ,  pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.0f + 2 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1]                                                         ,  pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.0f + 2 * pictureOffset + mainWallPicturesWidths[0]                                                                                     , -pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.0f + 2 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1]                                                         , -pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
                                                                                                                                                                               
        // Liberty Leading the People sa okvirom                                                                                                                                 
        -1.0f + 3 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1]                                                         ,  pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.0f + 3 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1] + mainWallPicturesWidths[2]                             ,  pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.0f + 3 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1]                                                         , -pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.0f + 3 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1] + mainWallPicturesWidths[2]                             , -pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
                                                                                                                                                    
        // The Coronation of Napoleon sa okvirom                                                                                                                      
        -1.0f + 4 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1] + mainWallPicturesWidths[2]                             ,  pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.0f + 4 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1] + mainWallPicturesWidths[2] + mainWallPicturesWidths[3] ,  pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.0f + 4 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1] + mainWallPicturesWidths[2]                             , -pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.0f + 4 * pictureOffset + mainWallPicturesWidths[0] + mainWallPicturesWidths[1] + mainWallPicturesWidths[2] + mainWallPicturesWidths[3] , -pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
    };

    unsigned int mainWallPicturesStride = (2 + 2 + 1 + 4) * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(picturesWithFramesVertices), picturesWithFramesVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mainWallPicturesStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, mainWallPicturesStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Debljina okvira
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, mainWallPicturesStride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Atributi boja (RGBA)
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, mainWallPicturesStride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // ------------------------------- DUGME -------------------------------

    float buttonVertices[CRES * 2 + 4];
    float r = 0.15;
    float buttonAspectRatio = (float)SCR_HEIGHT / (float)SCR_WIDTH;
    buttonVertices[0] = 0.7;
    buttonVertices[1] = 0.75;
    for (int i = 0; i <= CRES; i++)
    {
        buttonVertices[2 + 2 * i] = 0.7 + r * cos((3.141592 / 180) * (i * 360 / CRES)) * buttonAspectRatio; // mnozimo sa aspect ratio-m kako bi bio krug a ne elipsa
        buttonVertices[2 + 2 * i + 1] = 0.75 + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }

    float buttonStride = 2 * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, buttonStride, (void*)0);
    glEnableVertexAttribArray(0);

    // ------------------------------- PROGRESS BAR -------------------------------

    float progressBarVertices[] = {
        // X       Y
           -0.9f,  0.7f, // Gore-Levo
           -0.9f,  0.8f, // Gore-Desno
           0.1f,  0.7f, // Dole-Levo
           0.1f,  0.8f  // Dole-Desno
    };

    float progressBarStride = 2 * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, progressBarStride, (void*)0);
    glEnableVertexAttribArray(0);

    // ------------------------------- POTPIS -------------------------------

    float signatureVertices[] = {
        // X       Y       S    T
           0.45f,  -0.6f,   0.0, 1.0,  // Gore-Levo
           0.9f,  -0.6f,   1.0, 1.0,  // Gore-Desno
           0.45f,  -0.9f,   0.0, 0.0,  // Dole-Levo
           0.9f,  -0.9f,   1.0, 0.0   // Dole-Desno
    };

    float signatureStride = (2 + 2) * sizeof(float);

    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureVertices), signatureVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // CISCENJE
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ------------------------------- TEKSTURE -------------------------------

    // GLAVNI ZID
    unsigned mainWallPicturesTextures[4];

    //Mona Lisa
    mainWallPicturesTextures[0] = loadImageToTexture("res/Mona_Lisa.png");
    glBindTexture(GL_TEXTURE_2D, mainWallPicturesTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //The Raft of the Medusa
    mainWallPicturesTextures[1] = loadImageToTexture("res/The_Raft_of_the_Medusa.png");
    glBindTexture(GL_TEXTURE_2D, mainWallPicturesTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Liberty Leading the People
    mainWallPicturesTextures[2] = loadImageToTexture("res/Liberty_Leading_the_People.png");
    glBindTexture(GL_TEXTURE_2D, mainWallPicturesTextures[2]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //The Coronation of Napoleon
    mainWallPicturesTextures[3] = loadImageToTexture("res/The_Coronation_of_Napoleon.png");
    glBindTexture(GL_TEXTURE_2D, mainWallPicturesTextures[3]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    // POTPIS
    unsigned signatureTexture;
    signatureTexture = loadImageToTexture("res/Signature.png");
    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    // CISCENJE
    glBindTexture(GL_TEXTURE_2D, 0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    // Button
    bool buttonClicked = false;
    // Progress bar
    float progressStep = 0.01f;
    float progressValue = 0.0f;
    // Pictures and frames
    float rotationRadius = 0.1f;
    float baseRotationSpeed = 1.0f;
    float maxRotationSpeed = 10.0f;
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processCommonInput(window);
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        {
            buttonClicked = true;
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            buttonClicked = false;
            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        }        
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            progressValue += progressStep;
            progressValue = progressValue > 1.0f ? 1.0f : progressValue;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            progressValue -= progressStep;
            progressValue = progressValue < 0.0f ? 0.0f : progressValue;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentRotationSpeed = baseRotationSpeed + (maxRotationSpeed - baseRotationSpeed) * progressValue;
        float angle = fmod(currentFrame * currentRotationSpeed, 2.0f * 3.14159265358979323846f);

        //Crtanje okvira
        for (int i = 0; i < 4; i++)
        {
            glPolygonMode(GL_FRONT_AND_BACK, buttonClicked ? GL_LINE : GL_FILL);
            frameShader.use();
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mainWallPicturesTextures[i]);
            frameShader.setFloat("time", currentFrame);
            frameShader.setVec2("circularPosition", buttonClicked ? 0 : (cos(angle) * rotationRadius), buttonClicked ? 0 : (sin(angle) * rotationRadius));
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
        }

        //Crtanje slika
        for (int i = 0; i < 4; i++)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            pictureShader.use();
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mainWallPicturesTextures[i]);
            pictureShader.setInt("uTex", i);
            pictureShader.setVec2("circularPosition", buttonClicked ? 0 : (cos(angle) * rotationRadius), buttonClicked ? 0 : (sin(angle) * rotationRadius));
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
        }

        //Crtanje dugmeta
        basicColorShader.use();
        glBindVertexArray(VAO[2]);
        buttonClicked ? basicColorShader.setVec4("buttonColor", 0.0f, 0.0f, 0.0f, 1.0f) : basicColorShader.setVec4("buttonColor", 1.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        //Crtanje progress bar-a
        progressShader.use();
        progressShader.setFloat("progressValue", progressValue);
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Crtanje potpisa
        basicTextureShader.use();
        glActiveTexture(GL_TEXTURE0 + sizeof(mainWallPicturesTextures));
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        basicTextureShader.setInt("uTex", sizeof(mainWallPicturesTextures));
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glUseProgram(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    //Brisanje bafera i sejdera
    glDeleteBuffers(5, VBO);
    glDeleteVertexArrays(5, VAO);
    delete &frameShader;
    delete &pictureShader;
    delete &progressShader;
    delete &basicColorShader;
    delete &basicTextureShader;
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