/*
*  CPE 471 lab 4 - modern graphics test bed
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

#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "GLSL_helper.h"
#include "MStackHelp.h"

using namespace std;
using namespace glm;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;

//Handles to the shader data
GLint h_aPosition;
GLint h_uColor;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLuint CubeBuffObj, CIndxBuffObj, GrndBuffObj, GIndxBuffObj, NormalBuffObj;
int g_CiboLen, g_GiboLen;

/* globals to control positioning and window size */
static float g_width, g_height;
float g_tx = -2.0;
float g_ty = 0;
float g_tz = 0;
float g_trans = -5.5;
float g_angle = 0;

static const float g_groundY = 0.0;      // y coordinate of the ground
static const float g_groundSize = 10.0;   // half the ground length

int houseSizeX[] = { rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand() };
int houseSizeZ[] = { rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand() };

//declare a matrix stack
RenderingHelper ModelTrans;

/* projection matrix - do not change - sets matrix in shader */
void SetProjectionMatrix() {
   glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);	
   safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change - sets matrix in shader */
void SetView() {
   glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0, g_trans));
   glm::mat4 RotateX = glm::rotate( Trans, g_angle, glm::vec3(0.0f, 1, 0));
   safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
}

/* model transforms - do not change - sets matrix in shader */
void SetModel() {
   safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
}

/* intialize ground data */
static void initGround() {

   // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
   float GrndPos[] = { 
      -g_groundSize, g_groundY, -g_groundSize, 
      -g_groundSize, g_groundY,  g_groundSize, 
      g_groundSize, g_groundY,  g_groundSize, 
      g_groundSize, g_groundY, -g_groundSize
   };
   unsigned short idx[] = {0, 1, 2, 0, 2, 3};

   g_GiboLen = 6;
   glGenBuffers(1, &GrndBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

   glGenBuffers(1, &GIndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

/* intialize the cube data */
static void initCube() {

   float CubePos[] = {
      -0.5, -0.5, -0.5, /*back face 5 verts :0 */
      -0.5, 0.5, -0.5,
      0.5, 0.5, -0.5,
      0.5, -0.5, -0.5,
      0.0, 0.75, -0.5,
      -0.5, -0.5, 0.5, /*front face 5 verts :5*/
      -0.5, 0.5, 0.5,
      0.5, 0.5, 0.5,
      0.5, -0.5, 0.5,
      0.0, 0.75, 0.5,
      -0.5, -0.5, 0.5, /*left face 4 verts :10*/
      -0.5, -0.5, -.5,
      -0.5, 0.5, -0.5,
      -0.5, 0.5, 0.5,
      0.5, -0.5, 0.5, /*right face 4 verts :14*/
      0.5, -0.5, -.5,
      0.5, 0.5, -0.5,
      0.5, 0.5, 0.5
   };

   unsigned short idx[] = { 0, 1, 2,  2, 3, 0,  5, 6, 7,  7, 8, 5,  10, 11, 12,  12, 13, 10,  /* 14, 15, 16,  16, 17, 14 */ };

   g_CiboLen = 18;
   glGenBuffers(1, &CubeBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

   glGenBuffers(1, &CIndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

   /* normal data - note it is empty right now 
   glGenBuffers(1, &NormalBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, NormalBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubeNormal), CubeNormal, GL_STATIC_DRAW);
   */
}

void InitGeom() {
   initGround();
   initCube();
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
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
   /* add a handle for the normal */
   h_uColor = safe_glGetUniformLocation(ShadeProg,  "uColor");
   h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}


/* Some OpenGL initialization */
void Initialize ()					// Any GL Init Code 
{
   // Start Of User Initialization
   glClearColor (1.0f, 1.0f, 1.0f, 1.0f);								
   // Black Background
   glClearDepth (1.0f);	// Depth Buffer Setup
   glDepthFunc (GL_LEQUAL);	// The Type Of Depth Testing
   glEnable (GL_DEPTH_TEST);// Enable Depth Testing

   /* some matrix stack init */
   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();

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


   /******************* set up to draw the ground plane */
   /*
   safe_glEnableVertexAttribArray(h_aPosition);
   // bind vbo
   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
   // bind ibo
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);

   // set the color in the shader 
   glUniform3f(h_uColor, 0.7, 0.98, 0.9);

   // set the matrix stack to the identity - no transforms on the ground
   ModelTrans.loadIdentity();
   SetModel();
   // draw!
   glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

   // Disable the attributes used by our shader
   safe_glDisableVertexAttribArray(h_aPosition);
   */

   /*********************** set up to draw the cube */
   safe_glEnableVertexAttribArray(h_aPosition);
   // bind vbo
   glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
   // bind ibo
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);

   ModelTrans.pushMatrix();

   // draw: set the color in the shader
   glUniform3f(h_uColor, abs(sin((y+x) * 3.14159 / 8.0)), 0.78, abs(cos((y+x) * 3.14159 / 16.0)));
   glDrawElements(GL_TRIANGLES, g_CiboLen - 3*6, GL_UNSIGNED_SHORT, 0);
   glUniform3f(h_uColor, 0.3, 0.3, 0.3);
   glDrawElements(GL_TRIANGLES, 3*6, GL_UNSIGNED_SHORT,
      (GLvoid*)(sizeof(short)*(g_CiboLen - 3*6)));

   ModelTrans.popMatrix();
   
   /*
   for (int x = 0; x < 2; x++)
   {
      spacing_sum = 0;

      for (int y = 0; y < 10; y++)
      {
         ModelTrans.pushMatrix();

         if (x == 0)
         {
            // Row across scene
            float sizeX = ((houseSizeX[y] % 1000) / 500.0) + .2;
            float sizeY = ((houseSizeX[y] % 2000) / 750.0) + .2;
            spacing_sum += sizeX / 2.0;
            ModelTrans.translate(vec3(g_tx + spacing_sum + xBlockXOffset, g_ty - 0.5, g_tz));
            ModelTrans.scale(sizeX, sizeY, 1.0);
            ModelTrans.translate(vec3(0, .5, 0));
            spacing_sum += sizeX / 2.0;
         }
         else if (x == 1)
         {
            // Row coming towards camera
            float sizeX = ((houseSizeZ[y] % 1000) / 500.0) + .2;
            float sizeY = ((houseSizeZ[y] % 2000) / 750.0) + .2;
            spacing_sum += sizeX / 2.0;
            ModelTrans.translate(vec3(g_tx, g_ty - 0.5, g_tz + spacing_sum + zBlockZOffset));
            ModelTrans.rotate(90, vec3(0, 1, 0));
            ModelTrans.scale(sizeX, sizeY, 1.0);
            ModelTrans.translate(vec3(0, .5, 0));
            spacing_sum += sizeX / 2.0;
         }

         // set up where to draw the box 
         SetModel();
         // draw!
         // set the color in the shader
         glUniform3f(h_uColor, abs(sin((y+x) * 3.14159 / 8.0)), 0.78, abs(cos((y+x) * 3.14159 / 16.0)));
         glDrawElements(GL_TRIANGLES, g_CiboLen - 3*6, GL_UNSIGNED_SHORT, 0);
         glUniform3f(h_uColor, 0.3, 0.3, 0.3);
         glDrawElements(GL_TRIANGLES, 3*6, GL_UNSIGNED_SHORT,
            (GLvoid*)(sizeof(short)*(g_CiboLen - 3*6)));

         ModelTrans.popMatrix();
      }
   }
   */

   // Disable the attributes used by our shader

   safe_glDisableVertexAttribArray(h_aPosition);

   //Disable the shader
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

void keyboard(unsigned char key, int x, int y ){
   switch( key ) {
      /* M and N key move the object in x */
   case 'd':
      g_tx -= .1;
      break;
   case 'a':
      g_tx += .1;
      break;
   case 'w':
      g_ty += .1;
      break;
   case 's':
      g_ty -= .1;
      break;
   case 'z':
      g_tz -= .1;
      break;
   case 'x':
      g_tz += .1;
      break;
   case 'q': case 'Q' :
      exit( EXIT_SUCCESS );
      break;
   }
   glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
   if (button == GLUT_LEFT_BUTTON)
   {
      if (state == GLUT_DOWN)
      {
         printf("mouse clicked at %d %d\n", x, y);
      }
   }
}

void mouseMove(int x, int y)
{
   printf("mouse moved %d %d", x, y);
}

int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowPosition( 20, 20 );
   glutInitWindowSize( 400, 400 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow("Cube and Transforms");
   glutReshapeFunc( ReshapeGL );
   glutDisplayFunc( Draw );
   glutKeyboardFunc( keyboard );
   glutMouseFunc(mouse);
   glutMotionFunc(mouseMove);
   Initialize();

   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"Lab0_vert.glsl"), textFileRead((char *)"Lab0_frag.glsl"))) {
      printf("Error installing shader!\n");
      return 0;
   }

   InitGeom();
   glutMainLoop();
   return 0;
}
