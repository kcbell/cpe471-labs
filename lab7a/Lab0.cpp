/*
  Base code for program 3 for CSC 471
  OpenGL, glut and GLSL application
  Starts to loads in a .m mesh file
  ADD: storing data into a VBO and drawing it
  Uses glm for matrix transforms
  I. Dunn and Z. Wood  (original .m loader by H. Hoppe)
*/

#include <iostream>

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif

#ifdef __unix__
#include <GL/glut.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")

#include <GL\glew.h>
#include <GL\glut.h>
#endif

#include "CMeshLoaderSimple.h"
#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

using namespace std;

//position and color data handles
GLuint triBuffObj, colBuffObj;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;
int TriangleCount;
static float g_width, g_height;
float g_Camtrans = -2.5;
float g_Camangle = 0;
glm::vec3 g_trans(0);

//Handles to the shader data
GLint h_aPosition;
GLint h_aColor;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;

/* initialize the geomtry (including color)
   Change file name to load a different mesh file
*/
void InitGeom() {
  CMeshLoader::loadVertexBufferObjectFromMesh(string("../Models/gameguy_color.m"), TriangleCount, triBuffObj, colBuffObj);
}

/* projection matrix */
void SetProjectionMatrix() {
  glm::mat4 Projection = glm::perspective(90.0f, (float)g_width/g_height, 0.1f, 100.f);
  safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
  glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0, g_Camtrans));
  glm::mat4 RotateX = glm::rotate( Trans, g_Camangle, glm::vec3(0.0f, 1, 0));
  safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
}

/* model transforms */
void SetModel() {
  glm::mat4 Trans = glm::translate( glm::mat4(1.0f), g_trans);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Trans));
}

/* set the model transform to the identity */
void SetModelI() {
  glm::mat4 tmp = glm::mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(tmp));
}


/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName)
{
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader

   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);

   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);

   //compile shader and print log
   glCompileShader(VS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);

   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);

   if (!vCompiled || !fCompiled) {
            printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
            return 0;
   }

   //create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);

   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);

   /* get handles to attribute data */
   h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   //h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
   h_aColor = safe_glGetAttribLocation(ShadeProg,  "aColor");
   //h_uLight = safe_glGetUniformLocation(ShadeProg, "uLight");
   h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}


/* Some OpenGL initialization */
void Initialize ()               // Any GL Init Code
{
   // Start Of User Initialization
   glClearColor (0,0,0, 1.0f);
   // Black Background
   //glClearDepth (1.0f);  // Depth Buffer Setup
   //glDepthFunc (GL_LEQUAL); // The Type Of Depth Testing
   glEnable (GL_DEPTH_TEST);// Enable Depth Testing

}


/* Main display function */
void Draw (void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Start our shader
   glUseProgram(ShadeProg);

        /* only set the projection and view matrix once */
        SetProjectionMatrix();
        SetView();

        SetModel();

   //data set up to access the vertices and color
   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, triBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
   safe_glEnableVertexAttribArray(h_aColor);
   glBindBuffer(GL_ARRAY_BUFFER, colBuffObj);
   safe_glVertexAttribPointer(h_aColor, 3, GL_FLOAT, GL_FALSE, 0, 0);

   //actually draw the data
   glDrawArrays(GL_TRIANGLES, 0, TriangleCount*3);
   //clean up
   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aColor);
   //disable the shader
   glUseProgram(0);
   glutSwapBuffers();
}

/* Reshape */
void ReshapeGL (int width, int height)
{
        g_width = (float)width;
        g_height = (float)height;
        glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));

}

//the keyboard callback to change the values to the transforms
void keyboard(unsigned char key, int x, int y ){
  switch( key ) {
    /* WASD keyes effect view/camera transform */
    case 'w':
      g_Camtrans += 0.1;
      break;
    case 's':
      g_Camtrans -= 0.1;
      cout << " Cam trans " << g_Camtrans << endl;
      break;
    case 'a':
      g_Camangle += 1;
      break;
    case 'd':
      g_Camangle -= 1;
      break;
    case 'q': case 'Q' :
      exit( EXIT_SUCCESS );
      break;
  }
  glutPostRedisplay();
}


int main( int argc, char *argv[] )
{
      glutInit( &argc, argv );
      glutInitWindowPosition( 200, 200 );
      glutInitWindowSize( 400, 400 );
      glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
      glutCreateWindow("My first mesh");
      glutReshapeFunc( ReshapeGL );
      glutDisplayFunc( Draw );
        glutKeyboardFunc( keyboard );

   g_width = g_height = 200;

#ifdef _WIN32
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
      std::cerr << "Error initializing glew! " << glewGetErrorString(err) << std::endl;
      return 1;
   }
#endif

      Initialize();

   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"mesh_vert.glsl"),
      textFileRead((char *)"mesh_frag.glsl"))) {
      printf("Error installing shader!\n");
      return 0;
   }

   InitGeom();
   glutMainLoop();
      return 0;
}
