/*
 *  CPE 474 lab 0 - modern graphics test bed
 *  draws a partial cube using a VBO and IBO 
 *  glut/OpenGL/GLSL application   
 *  Uses glm and local matrix stack
 *  to handle matrix transforms for a view matrix, projection matrix and
 *  model transform matrix
 *
 *  zwood 9/12 
 *  Copyright 2012 Cal Poly. All rights reserved.
 *
 *****************************************************************************/

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif

#ifdef __unix__
#include <GL/glut.h>
#endif

#ifdef _WIN32
#include <GL/glew.h>
#include <GL/glut.h>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "freeglut.lib")
#endif

#include <iostream>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLSL_helper.h"
#include "MStackHelp.h"

#include "GeometryCreator.h"

using namespace std;
using namespace glm;


////////////////
// Globals :( //
////////////////

// Parameters
unsigned int const StepSize = 10;
unsigned int WindowWidth = 1600, WindowHeight = 900;

// Meshes
unsigned int const MeshCount = 14;
Mesh * Meshes[MeshCount];
unsigned int CurrentMesh;

// Variable Handles
GLuint aPosition;
GLuint aNormal;
GLuint uModelMatrix;
GLuint uNormalMatrix;
GLuint uViewMatrix;
GLuint uProjMatrix;
GLuint uColor;

// Shader Handle
GLuint ShadeProg;


// Program Variables
float Accumulator;
float CameraHeight;


RenderingHelper ModelTrans;

void SetProjectionMatrix()
{
    glm::mat4 Projection = glm::perspective(80.0f, ((float) WindowWidth)/ ((float)WindowHeight), 0.1f, 100.f);
    safe_glUniformMatrix4fv(uProjMatrix, glm::value_ptr(Projection));
}

void SetView()
{
    glm::mat4 View = glm::lookAt(vec3(0.f, CameraHeight, 5.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    safe_glUniformMatrix4fv(uViewMatrix, glm::value_ptr(View));
}

void SetModel()
{
    safe_glUniformMatrix4fv(uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
    safe_glUniformMatrix4fv(uNormalMatrix, glm::value_ptr(glm::transpose(glm::inverse(ModelTrans.modelViewMatrix))));
}

bool InstallShader(std::string const & vShaderName, std::string const & fShaderName)
{
    GLuint VS; // handles to shader object
    GLuint FS; // handles to frag shader object
    GLint vCompiled, fCompiled, linked; // status of shader

    VS = glCreateShader(GL_VERTEX_SHADER);
    FS = glCreateShader(GL_FRAGMENT_SHADER);

    // load the source
    char const * vSource = textFileRead(vShaderName);
    char const * fSource = textFileRead(fShaderName);
    glShaderSource(VS, 1, & vSource, NULL);
    glShaderSource(FS, 1, & fSource, NULL);

    // compile shader and print log
    glCompileShader(VS);
    printOpenGLError();
    glGetShaderiv(VS, GL_COMPILE_STATUS, & vCompiled);
    printShaderInfoLog(VS);

    // compile shader and print log
    glCompileShader(FS);
    printOpenGLError();
    glGetShaderiv(FS, GL_COMPILE_STATUS, & fCompiled);
    printShaderInfoLog(FS);

    if (! vCompiled || ! fCompiled)
    {
        std::cerr << "Error compiling either shader " << vShaderName << " or " << fShaderName << std::endl;
        return false;
    }

    // create a program object and attach the compiled shader
    ShadeProg = glCreateProgram();
    glAttachShader(ShadeProg, VS);
    glAttachShader(ShadeProg, FS);

    glLinkProgram(ShadeProg);

    // check shader status requires helper functions
    printOpenGLError();
    glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
    printProgramInfoLog(ShadeProg);

    glUseProgram(ShadeProg);

    // get handles to attribute data
    aPosition   = safe_glGetAttribLocation(ShadeProg, "aPosition");
    aNormal     = safe_glGetAttribLocation(ShadeProg, "aNormal");
    
    uColor          = safe_glGetUniformLocation(ShadeProg, "uColor");
    uProjMatrix     = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
    uViewMatrix     = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
    uModelMatrix    = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
    uNormalMatrix   = safe_glGetUniformLocation(ShadeProg, "uNormalMatrix");

    std::cout << "Sucessfully installed shader " << ShadeProg << std::endl;
    return true;
}

void Initialize()
{
    glClearColor(0.8f, 0.8f, 1.0f, 1.0f);

    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    ModelTrans.useModelViewMatrix();
    ModelTrans.loadIdentity();
}

void Draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ShadeProg);

    SetProjectionMatrix();
    SetView();
    
    ModelTrans.loadIdentity();

    ModelTrans.pushMatrix();

        ModelTrans.rotate(Accumulator * 35.f, vec3(0, 1, 0));
        SetModel();

        safe_glEnableVertexAttribArray(aPosition);
        glBindBuffer(GL_ARRAY_BUFFER, Meshes[CurrentMesh]->PositionHandle);
        safe_glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

        safe_glEnableVertexAttribArray(aNormal);
        glBindBuffer(GL_ARRAY_BUFFER, Meshes[CurrentMesh]->NormalHandle);
        safe_glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Meshes[CurrentMesh]->IndexHandle);

        glUniform3f(uColor, 0.1f, 0.78f, 0.9f);

        glDrawElements(GL_TRIANGLES, Meshes[CurrentMesh]->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

        safe_glDisableVertexAttribArray(aPosition);
        safe_glDisableVertexAttribArray(aNormal);

    ModelTrans.popMatrix();

    glUseProgram(0);
    glutSwapBuffers();
    glutPostRedisplay();
    printOpenGLError();
}

void Reshape(int width, int height)
{
    WindowWidth = width;
    WindowHeight = height;
    glViewport(0, 0, width, height);
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    
    // Camera up/down
    case 'w':
        CameraHeight += 0.1f;
        break;
    case 's':
        CameraHeight -= 0.1f;
        break;

    // Toggle wireframe
    case 'n':
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
        break;
    case 'm':
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
        break;

    // Cycle meshes
    case 'j':
        if (CurrentMesh)
            CurrentMesh --;
        else
            CurrentMesh = MeshCount - 1;
        break;
    case 'k':
        CurrentMesh ++;
        CurrentMesh %= MeshCount;
        break;

    // Quit program
    case 'q': case 'Q' :
        exit( EXIT_SUCCESS );
        break;
    
    }
}

void Timer(int param)
{
    Accumulator += StepSize * 0.001f;
    glutTimerFunc(StepSize, Timer, 1);
}

int main(int argc, char *argv[])
{
    // Initialize Global Variables
    Accumulator = 0.f;
    CameraHeight = 0.f;
    CurrentMesh = 0;

    // Glut Setup
    glutInit(& argc, argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WindowWidth, WindowHeight);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Geometry Creator Demo");
    glutDisplayFunc(Draw);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(StepSize, Timer, 1);

    // GLEW Setup (Windows only)
#ifdef _WIN32
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error initializing glew! " << glewGetErrorString(err) << std::endl;
        return 1;
    }
#endif

    // OpenGL Setup
    Initialize();
    getGLversion();

    // Shader Setup
    if (! InstallShader("Diffuse.vert", "Diffuse.frag"))
    {
        printf("Error installing shader!\n");
        return 1;
    }
    
    Meshes[0] = GeometryCreator::CreateCube();
    Meshes[1]= GeometryCreator::CreateCube(vec3(0.5f, 2.f, 4.f));
    Meshes[2] = GeometryCreator::CreateCylinder(1.f, 1.f, 1.5f, 8, 8);
    Meshes[3] = GeometryCreator::CreateCylinder(1.5f, 0.5f, 2.0f, 16, 5);
    Meshes[4] = GeometryCreator::CreateCylinder(0.8f, 0.f, 1.75f, 32, 1);
    Meshes[5] = GeometryCreator::CreateDisc(0.5f, 1.5f, 1.75f, 32, 3);
    Meshes[6] = GeometryCreator::CreateDisc(0.1f, 1.f, 0.f, 8, 0);
    Meshes[7] = GeometryCreator::CreateSphere(glm::vec3(1.5f));
    Meshes[8] = GeometryCreator::CreateSphere(glm::vec3(1.5f, 2.5f, 0.5f), 128, 32);
    Meshes[9] = GeometryCreator::CreateSphere(glm::vec3(2.0f), 4, 2);
    Meshes[10] = GeometryCreator::CreateSphere(glm::vec3(2.0f), 6, 3);
    Meshes[11] = GeometryCreator::CreateTorus(1.f, 2.f, 12, 24);
    Meshes[12] = GeometryCreator::CreateTorus(0.5f, 2.5f, 8, 12);
    Meshes[13] = GeometryCreator::CreateTorus(2.f, 2.5f, 48, 64);
    glutMainLoop();

    return 0;
}
