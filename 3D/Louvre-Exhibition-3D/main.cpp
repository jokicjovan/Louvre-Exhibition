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
void processSceneInput(GLFWwindow* window);

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
    Shader roomShader("room.vert", "room.frag");
    Shader frontWallPictureShader("front_wall_picture.vert", "front_wall_picture.frag");
    Shader progressShader("progress.vert", "progress.frag");
    Shader buttonShader("button.vert", "button.frag");
    Shader signatureShader("signature.vert", "signature.frag");

    unsigned int VAO[5];
    glGenVertexArrays(5, VAO);
    unsigned int VBO[5];
    glGenBuffers(5, VBO);

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


    // ------------------------------- SLIKE SA OKVIRIMA NA GLAVNOM ZIDU -------------------------------
    float pictureHeight = 0.8f;
    float pictureOffset = 0.1f;

    float frontWallPicturesAspectRatios[] = { 800.0 / 1192.0, 1280.0 / 874.0, 1280.0 / 1027.0, 1920.0 / 1207.0 };
    float frontWallPicturesTotalWidth = 0.0f;
    for (int i = 0; i < 4; i++) {
        frontWallPicturesTotalWidth += (pictureHeight * frontWallPicturesAspectRatios[i]);
    }
    float frontWallPicturesScaler = (frontWallPicturesTotalWidth + 5 * pictureOffset > 4.0f) ? 
        (4.0f - 5 * pictureOffset) / frontWallPicturesTotalWidth : 1.0f;     // kreiranje "skalera" za slike 
                                                                            //(uzima u obzir sirinu slika i offset-e izmedju njih)
    float frontWallPicturesWidths[4];
    for (int i = 0; i < 4; i++) {
        frontWallPicturesWidths[i] = pictureHeight * frontWallPicturesAspectRatios[i] * frontWallPicturesScaler;
    }

    float picturesWithFramesVertices[] = {
        //X                                                                                                                                              Y                     Z         S    T      OKVIR    R    G    B    A
        // Mona Lisa sa okvirom                                                                                                                                            
        -2.0f + pictureOffset                                                                                                                         ,  pictureHeight / 2 ,  -1.99f,    0.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Levo
        -2.0f + pictureOffset                                                                                                                         , -pictureHeight / 2 ,  -1.99f,    0.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Levo
        -2.0f + pictureOffset + frontWallPicturesWidths[0]                                                                                            ,  pictureHeight / 2 ,  -1.99f,    1.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Desno
        -2.0f + pictureOffset + frontWallPicturesWidths[0]                                                                                            , -pictureHeight / 2 ,  -1.99f,    1.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Desno
                                                                                                                                                                                  
        // The Raft of the Medusa sa okvirom                                                                                                                                                              
        -2.0f + 2 * pictureOffset + frontWallPicturesWidths[0]                                                                                        ,  pictureHeight / 2 ,  -1.99f,    0.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -2.0f + 2 * pictureOffset + frontWallPicturesWidths[0]                                                                                        , -pictureHeight / 2 ,  -1.99f,    0.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -2.0f + 2 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           ,  pictureHeight / 2 ,  -1.99f,    1.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -2.0f + 2 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           , -pictureHeight / 2 ,  -1.99f,    1.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
                                                                                                                                                                                  
        // Liberty Leading the People sa okvirom                                                                                                                                  
        -2.0f + 3 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           ,  pictureHeight / 2 ,  -1.99f,    0.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -2.0f + 3 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1]                                                           , -pictureHeight / 2 ,  -1.99f,    0.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -2.0f + 3 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              ,  pictureHeight / 2 ,  -1.99f,    1.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -2.0f + 3 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              , -pictureHeight / 2 ,  -1.99f,    1.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
                                                                                                                                                                           
        // The Coronation of Napoleon sa okvirom                                                                                                                             
        -2.0f + 4 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              ,  pictureHeight / 2 ,  -1.99f,    0.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -2.0f + 4 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2]                              , -pictureHeight / 2 ,  -1.99f,    0.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -2.0f + 4 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2] + frontWallPicturesWidths[3] ,  pictureHeight / 2 ,  -1.99f,    1.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -2.0f + 4 * pictureOffset + frontWallPicturesWidths[0] + frontWallPicturesWidths[1] + frontWallPicturesWidths[2] + frontWallPicturesWidths[3] , -pictureHeight / 2 ,  -1.99f,    1.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
    };

    unsigned int frontWallPicturesStride = (3 + 2 + 1 + 4) * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(picturesWithFramesVertices), picturesWithFramesVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Debljina okvira
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Atributi boja (RGBA)
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, frontWallPicturesStride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // ------------------------------- DUGME -------------------------------

    float buttonVertices[CRES * 3 + 6];
    float r = 0.15;
    buttonVertices[0] = 1.4;
    buttonVertices[1] = 0.75;
    buttonVertices[2] = -1.99f;
    for (int i = 0; i <= CRES; i++)
    {
        buttonVertices[3 + 3 * i] = 1.4 + r * cos((3.141592 / 180) * (i * 360 / CRES));
        buttonVertices[3 + 3 * i + 1] = 0.75 + r * sin((3.141592 / 180) * (i * 360 / CRES));
        buttonVertices[3 + 3 * i + 2] = -1.99f;
    }

    float buttonStride = 3 * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, buttonStride, (void*)0);
    glEnableVertexAttribArray(0);

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
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
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
    unsigned frontWallPicturesTextures[4];

    //Mona Lisa
    frontWallPicturesTextures[0] = loadImageToTexture("res/Mona_Lisa.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //The Raft of the Medusa
    frontWallPicturesTextures[1] = loadImageToTexture("res/The_Raft_of_the_Medusa.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Liberty Leading the People
    frontWallPicturesTextures[2] = loadImageToTexture("res/Liberty_Leading_the_People.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[2]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //The Coronation of Napoleon
    frontWallPicturesTextures[3] = loadImageToTexture("res/The_Coronation_of_Napoleon.png");
    glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[3]);
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
    
    // Postavljanje jacine svetla i materiala zida
    roomShader.use();
    roomShader.setVec3("uLight.position", glm::vec3(0.0f, 0.9f, 0.0f));
    roomShader.setVec3("uLight.direction", glm::vec3(0.0f, -0.9f, -1.5f));       
    roomShader.setFloat("uLight.cutOff", glm::cos(glm::radians(60.0f)));
    roomShader.setFloat("uLight.outerCutOff", glm::cos(glm::radians(65.0f)));

    roomShader.setVec3("uLight.ambient", 0.3f, 0.3f, 0.3f);
    roomShader.setVec3("uLight.diffuse", 0.8f, 0.8f, 0.8f);
    roomShader.setVec3("uLight.specular", 1.0f, 1.0f, 1.0f);

    roomShader.setFloat("uLight.constant", 1.0f);
    roomShader.setFloat("uLight.linear", 0.09f);
    roomShader.setFloat("uLight.quadratic", 0.032f);

    roomShader.setFloat("uMaterial.shininess", 0.6 * 128); // Marble
    roomShader.setVec3("uMaterial.diffuse", 0.8f, 0.8f, 0.8f);
    roomShader.setVec3("uMaterial.specular", 0.1f, 0.1f, 0.1f);
    glUseProgram(0);

    // Button
    bool buttonClicked = false;
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
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            buttonClicked = true;
            roomShader.use();
            roomShader.setFloat("uMaterial.shininess", 0.6 * 128); //Materijal: Dark Stone
            roomShader.setVec3("uMaterial.diffuse", 0.3, 0.3, 0.3);
            roomShader.setVec3("uMaterial.specular", 0.5, 0.5, 0.5);
            glUseProgram(0);
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            buttonClicked = false;
            roomShader.use();
            roomShader.setFloat("uMaterial.shininess", 0.6 * 128); //Materijal: Marble
            roomShader.setVec3("uMaterial.diffuse", 0.8, 0.8, 0.8);
            roomShader.setVec3("uMaterial.specular", 0.1, 0.1, 0.1);
            glUseProgram(0);
        }        
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

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        //Crtanje sobe
        roomShader.use();
        roomShader.setMat4("uM", model);
        roomShader.setMat4("uP", projection);
        roomShader.setMat4("uV", view);
        roomShader.setVec3("uViewPos", camera.Position);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Priprema za crtanje slika prednjeg zida
        float currentRotationSpeed = baseRotationSpeed + (maxRotationSpeed - baseRotationSpeed) * progressValue;
        float angle = fmod(currentFrame * currentRotationSpeed, 2.0f * 3.14159265358979323846f);

        //Crtanje slika sa okvirima
        for (int i = 0; i < 4; i++)
        {
            frontWallPictureShader.use();
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, frontWallPicturesTextures[i]);
            frontWallPictureShader.setMat4("uM", model);
            frontWallPictureShader.setMat4("uP", projection);
            frontWallPictureShader.setMat4("uV", view);
            frontWallPictureShader.setFloat("uTime", currentFrame);
            frontWallPictureShader.setInt("uTex", i);
            frontWallPictureShader.setVec2("uCircularPosition", buttonClicked ? 0 : (cos(angle) * rotationRadius), buttonClicked ? 0 : (sin(angle) * rotationRadius));
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
        }

        //Crtanje dugmeta
        buttonShader.use();
        glBindVertexArray(VAO[2]);
        buttonShader.setMat4("uM", model);
        buttonShader.setMat4("uP", projection);
        buttonShader.setMat4("uV", view);
        buttonClicked ? buttonShader.setVec4("uButtonColor", 0.0f, 0.0f, 0.0f, 1.0f) : buttonShader.setVec4("uButtonColor", 1.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        //Crtanje progress bar-a
        progressShader.use();
        progressShader.setMat4("uM", model);
        progressShader.setMat4("uP", projection);
        progressShader.setMat4("uV", view);
        progressShader.setFloat("uProgressValue", progressValue);
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Crtanje potpisa
        signatureShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        signatureShader.setInt("uTex", 0);
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    //Brisanje bafera i sejdera
    glDeleteBuffers(5, VBO);
    glDeleteVertexArrays(5, VAO);
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