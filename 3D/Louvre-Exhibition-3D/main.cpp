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

unsigned int compileShader(GLenum type, const char* source); //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
unsigned int createShader(const char* vsSource, const char* fsSource); //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource i Fragment sejdera na putanji fsSource
static unsigned loadImageToTexture(const char* filePath);

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
    unsigned int wWidth = 1920;
    unsigned int wHeight = 700;
    const char wTitle[] = "Louvre Exhibition";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); // Napravi novi prozor
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

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    unsigned int frameShader = createShader("frame.vert", "frame.frag");
    unsigned int pictureShader = createShader("picture.vert", "picture.frag");
    unsigned int buttonShader = createShader("button.vert", "button.frag");
    unsigned int progressShader = createShader("progress.vert", "progress.frag");
    unsigned int signatureShader = createShader("signature.vert", "signature.frag");

    unsigned int VAO[4];
    glGenVertexArrays(4, VAO);
    unsigned int VBO[4];
    glGenBuffers(4, VBO);

    // ------------------------------- SLIKE SA OKVIRIMA -------------------------------

    float aspectRatios[] = { 800.0 / 1192.0, 1280.0 / 874.0, 1280.0 / 1027.0, 1920.0 / 1207.0 };
    float pictureHeight = 0.8f;
    float pictureOffset = 0.1f;

    float totalPicturesWidth = 0.0f;
    for (int i = 0; i < 4; i++) {
        totalPicturesWidth += (pictureHeight * aspectRatios[i]);
    }
    float scale = (totalPicturesWidth + 5 * pictureOffset > 2.0f) ? (2.0f - 5 * pictureOffset) / totalPicturesWidth : 1.0f;     // kreiranje "skalera" za slike 
                                                                                                                                //(uzima u obzir sirinu slika i offset-e izmedju njih)
    float picturesWidths[4];
    for (int i = 0; i < 4; i++) {
        picturesWidths[i] = pictureHeight * aspectRatios[i] * scale;
    }

    float picturesWithFramesVertices[] = {
        //X                                                                                                         Y                     S    T      OKVIR    R    G    B    A
        // Mona Lisa sa okvirom
        -1.0f + pictureOffset                                                                                     , pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Levo
        -1.0f + pictureOffset + picturesWidths[0]                                                                 , pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Gore-Desno
        -1.0f + pictureOffset                                                                                     ,-pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Levo
        -1.0f + pictureOffset + picturesWidths[0]                                                                 ,-pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   1.0, 1.0, 0.0, 1.0,  //Dole-Desno
                                                                                                                                              
        // The Raft of the Medusa sa okvirom                                                                                                                           
        -1.0f + 2 * pictureOffset + picturesWidths[0]                                                             , pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.0f + 2 * pictureOffset + picturesWidths[0] + picturesWidths[1]                                         , pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.0f + 2 * pictureOffset + picturesWidths[0]                                                             ,-pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
        -1.0f + 2 * pictureOffset + picturesWidths[0] + picturesWidths[1]                                         ,-pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   1.0, 0.0, 0.0, 1.0,
                                                                                                                                              
        // Liberty Leading the People sa okvirom                                                                                                
        -1.0f + 3 * pictureOffset + picturesWidths[0] + picturesWidths[1]                                         , pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.0f + 3 * pictureOffset + picturesWidths[0] + picturesWidths[1] + picturesWidths[2]                     , pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.0f + 3 * pictureOffset + picturesWidths[0] + picturesWidths[1]                                         ,-pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
        -1.0f + 3 * pictureOffset + picturesWidths[0] + picturesWidths[1] + picturesWidths[2]                     ,-pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   0.5, 0.0, 0.5, 1.0,
                                                                                                                                              
        // The Coronation of Napoleon sa okvirom                                                                                                                     
        -1.0f + 4 * pictureOffset + picturesWidths[0] + picturesWidths[1] + picturesWidths[2]                     , pictureHeight / 2 ,   0.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.0f + 4 * pictureOffset + picturesWidths[0] + picturesWidths[1] + picturesWidths[2] + picturesWidths[3] , pictureHeight / 2 ,   1.0, 1.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.0f + 4 * pictureOffset + picturesWidths[0] + picturesWidths[1] + picturesWidths[2]                     ,-pictureHeight / 2 ,   0.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
        -1.0f + 4 * pictureOffset + picturesWidths[0] + picturesWidths[1] + picturesWidths[2] + picturesWidths[3] ,-pictureHeight / 2 ,   1.0, 0.0   ,0.05f,   0.0, 0.0, 1.0, 1.0,
    };

    unsigned int picturesStride = (2 + 2 + 1 + 4) * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(picturesWithFramesVertices), picturesWithFramesVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, picturesStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, picturesStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Debljina okvira
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, picturesStride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Atributi boja (RGBA)
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, picturesStride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Postavili smo sta treba, pa te stvari iskljucujemo, da se naknadna podesavanja ne bi odnosila na njih i nesto poremetila
    // To radimo tako sto bindujemo 0, pa kada treba da nacrtamo nase stvari, samo ih ponovo bindujemo
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ------------------------------- DUGME -------------------------------

    float buttonVertices[CRES * 2 + 4];
    float r = 0.15;
    float buttonAspectRatio = (float)wHeight / (float)wWidth;
    buttonVertices[0] = 0.7;
    buttonVertices[1] = 0.75;
    for (int i = 0; i <= CRES; i++)
    {
        buttonVertices[2 + 2 * i] = 0.7 + r * cos((3.141592 / 180) * (i * 360 / CRES)) * buttonAspectRatio; // mnozimo sa aspect ratio-m kako bi bio krug a ne elipsa
        buttonVertices[2 + 2 * i + 1] = 0.75 + r * sin((3.141592 / 180) * (i * 360 / CRES));
    }

    float buttonStride = 2 * sizeof(float);

    //Podesavanje
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, buttonStride, (void*)0);
    glEnableVertexAttribArray(0);

    //Ciscenje
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, progressBarStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Ciscenje
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ------------------------------- POTPIS -------------------------------

    float signatureVertices[] = {
        // X       Y       S    T
           0.45f,  -0.6f,   0.0, 1.0,  // Gore-Levo
           0.9f,  -0.6f,   1.0, 1.0,  // Gore-Desno
           0.45f,  -0.9f,   0.0, 0.0,  // Dole-Levo
           0.9f,  -0.9f,   1.0, 0.0   // Dole-Desno
    };

    float signatureStride = (2 + 2) * sizeof(float);

    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureVertices), signatureVertices, GL_STATIC_DRAW);

    // Atributi za tacke
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributi za teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Ciscenje
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TEKSTURE

    unsigned picturesTextures[4];
    unsigned signatureTexture;

    //Mona Lisa
    picturesTextures[0] = loadImageToTexture("res/Mona_Lisa.png");
    glBindTexture(GL_TEXTURE_2D, picturesTextures[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //The Raft of the Medusa
    picturesTextures[1] = loadImageToTexture("res/The_Raft_of_the_Medusa.png");
    glBindTexture(GL_TEXTURE_2D, picturesTextures[1]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Liberty Leading the People
    picturesTextures[2] = loadImageToTexture("res/Liberty_Leading_the_People.png");
    glBindTexture(GL_TEXTURE_2D, picturesTextures[2]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //The Coronation of Napoleon
    picturesTextures[3] = loadImageToTexture("res/The_Coronation_of_Napoleon.png");
    glBindTexture(GL_TEXTURE_2D, picturesTextures[3]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Potpis
    signatureTexture = loadImageToTexture("res/Signature.png");
    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    // Uniforms locations
    unsigned uTimeLoc = glGetUniformLocation(frameShader, "time");
    unsigned uFrameCircularPositionLoc = glGetUniformLocation(frameShader, "circularPosition");

    unsigned uPictureTexLoc = glGetUniformLocation(pictureShader, "uTex");
    unsigned uPictureCircularPositionLoc = glGetUniformLocation(pictureShader, "circularPosition");

    unsigned uButtonColorLoc = glGetUniformLocation(buttonShader, "buttonColor");

    unsigned uProgressValueLoc = glGetUniformLocation(progressShader, "progressValue");

    unsigned uSignatureTexLoc = glGetUniformLocation(signatureShader, "uTex");

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
        float currentTime = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            buttonClicked = true;
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            buttonClicked = false;
            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        }        
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            progressValue += progressStep;
            progressValue = progressValue > 1.0f ? 1.0f : progressValue;
        }
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        {
            progressValue -= progressStep;
            progressValue = progressValue < 0.0f ? 0.0f : progressValue;
        }

        float currentRotationSpeed = baseRotationSpeed + (maxRotationSpeed - baseRotationSpeed) * progressValue;
        float angle = fmod(currentTime * currentRotationSpeed, 2.0f * 3.14159265358979323846f);

        //Crtanje okvira
        for (int i = 0; i < 4; i++)
        {
            glPolygonMode(GL_FRONT_AND_BACK, buttonClicked ? GL_LINE : GL_FILL);
            glUseProgram(frameShader);
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, picturesTextures[i]);
            glUniform1f(uTimeLoc, currentTime);
            glUniform2f(uFrameCircularPositionLoc, buttonClicked ? 0 : (cos(angle) * rotationRadius), buttonClicked ? 0 : (sin(angle) * rotationRadius));
            glBindVertexArray(VAO[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
        }

        //Crtanje slika
        for (int i = 0; i < 4; i++)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUseProgram(pictureShader);
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, picturesTextures[i]);
            glUniform1i(uPictureTexLoc, i);
            glUniform2f(uPictureCircularPositionLoc, buttonClicked ? 0 : (cos(angle) * rotationRadius), buttonClicked ? 0 : (sin(angle) * rotationRadius));
            glBindVertexArray(VAO[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
        }

        //Crtanje dugmeta
        glUseProgram(buttonShader);
        glBindVertexArray(VAO[1]);
        buttonClicked ? glUniform4f(uButtonColorLoc, 0.0f, 0.0f, 0.0f, 1.0f) : glUniform4f(uButtonColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES + 2);

        //Crtanje progress bar-a
        glUseProgram(progressShader);
        glUniform1f(uProgressValueLoc, progressValue);
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Crtanje potpisa
        glUseProgram(signatureShader);
        glActiveTexture(GL_TEXTURE0 + sizeof(picturesTextures));
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        glUniform1i(uSignatureTexLoc, sizeof(picturesTextures));
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    //Brisanje bafera i sejdera
    glDeleteBuffers(4, VBO);
    glDeleteVertexArrays(4, VAO);
    glDeleteProgram(frameShader);
    glDeleteProgram(pictureShader);
    glDeleteProgram(buttonShader);
    glDeleteProgram(progressShader);
    glDeleteProgram(signatureShader);
    //Sve OK - batali program
    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)

    int success; //Da li je kompajliranje bilo uspjesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
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