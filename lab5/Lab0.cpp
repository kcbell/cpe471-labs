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

static float g_width, g_height;
float camera_trans = -5.5; // Camera controls
float camera_angle = 0;

vec3 cube_translate;
mat4 cube_rotate;
float cube_scale;

RenderingHelper ModelTrans;

vec2 screenToFractionalCoords(int x, int y);

/* projection matrix - do not change - sets matrix in shader */
void SetProjectionMatrix() {
   glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);	
   safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change - sets matrix in shader */
void SetView() {
   glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0, camera_trans));
   glm::mat4 RotateX = glm::rotate( Trans, camera_angle, glm::vec3(0.0f, 1, 0));
   safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
}

/* model transforms - do not change - sets matrix in shader */
void SetModel() {
   safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
}

/* intialize the cube data */
static void initCube()
{
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

   unsigned short idx[] = {0, 1, 2,  2, 3, 0,  1, 4, 2,  5, 6, 7,  7, 8, 5,  6, 9, 7,  10, 11, 12,  12, 13, 10,  14, 15, 16,  16, 17, 14};

   g_CiboLen = 30;
   glGenBuffers(1, &CubeBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

   glGenBuffers(1, &CIndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

void InitGeom() {
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
   glClearColor (0.1, 0.1, 0.1, 1.0f);
   // Black Background
   glClearDepth (1.0f);	// Depth Buffer Setup
   glDepthFunc (GL_LEQUAL);	// The Type Of Depth Testing
   glEnable (GL_DEPTH_TEST);// Enable Depth Testing

   /* some matrix stack init */
   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();

   cube_translate = vec3(0.0);
   cube_scale = 1.0;
   cube_rotate = mat4(1.0); //The identity matrix
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

   /*********************** set up to draw the cube */
   safe_glEnableVertexAttribArray(h_aPosition);
   // bind vbo
   glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
   // bind ibo
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);

   glUniform3f(h_uColor, 0.5, 0.05, 0.4); //Cube color

   ModelTrans.pushMatrix();

   ModelTrans.translate(cube_translate);
   ModelTrans.scale(cube_scale, cube_scale, cube_scale);
   ModelTrans.multMatrix(cube_rotate);
   
   SetModel();
   glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);

   ModelTrans.popMatrix();
   
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

bool mouse_left_down;
int drag_begin_x, drag_begin_y;
int nav_mode = 0;
const int MODE_ROTATE = 1, MODE_SCALE = 2, MODE_PAN = 3;

void mouse(int button, int state, int x, int y)
{
   if (button == GLUT_LEFT_BUTTON)
   {
      if (state == GLUT_DOWN)
      {
         mouse_left_down = true;
         drag_begin_x = x;
         drag_begin_y = y;
      }
      else if (state == GLUT_UP)
      {
         mouse_left_down = false;
      }
   }
}

void mouseMove(int x, int y)
{
   vec2 begin, end;
   float beginDist, endDist;
   
   begin = screenToFractionalCoords(drag_begin_x, drag_begin_y);
   end   = screenToFractionalCoords(x, y);
   beginDist = sqrt((float)pow(begin.x, 2) + pow(begin.y, 2));
   endDist   = sqrt((float)pow(end.x, 2) + pow(end.y, 2));

   if (nav_mode == MODE_SCALE)
   {
      float scaleFactor = endDist / beginDist;
      if (scaleFactor != 0)
         cube_scale *= scaleFactor;
   }
   else if (nav_mode == MODE_ROTATE)
   {
      float alpha;
      vec3 u, v, axis;

      // Constrain begin and end to a radius of .99
      float RADIUS = 0.99;
      if (beginDist > RADIUS)
      {
         begin.x /= beginDist / RADIUS;
         begin.y /= beginDist / RADIUS;
         beginDist = RADIUS;
      }
      if (endDist > RADIUS)
      {
         end.x /= endDist / RADIUS;
         end.y /= endDist / RADIUS;
         endDist = RADIUS;
      }

      // 1. Compute two vec3's for mousedown and mouseup u and v
      u = vec3(begin.x, begin.y, sqrt(1.0 - pow(beginDist,2)));
      v = vec3(end.x, end.y, sqrt(1.0 - pow(endDist,2)));

      // 2. Compute axis of rotation u cross v
      axis = vec3(
         u.y*v.z - u.z*v.y,
         u.z*v.x - u.x*v.z,
         u.x*v.y - u.y*v.x);

      // 3. Compute angle of rotation alpha = acos(u dot v)
      float udotv = u.x * v.x + u.y * v.y + u.z * v.z;
      if (udotv < -1.0 || udotv > 1.0)
         return;
      alpha = acos(udotv);
      // 3a. convert rad to deg
      alpha *= 180/3.14159265;

      // 4. rotate(alpha, axis)
      cube_rotate = rotate(mat4(1.0), alpha, axis) * cube_rotate;
   }
   else if (nav_mode == MODE_PAN)
   {
      cube_translate = vec3(
         cube_translate.x + (x - drag_begin_x) / 100.0,
         cube_translate.y - (y - drag_begin_y) / 100.0,
         cube_translate.z);

      cube_translate = vec3(
         cube_translate.x + 1.5 * (end.x - begin.x),
         cube_translate.y + 1.5 * (end.y - begin.y),
         cube_translate.z);
   }

   if (mouse_left_down)
   {
      drag_begin_x = x;
      drag_begin_y = y;
      glutPostRedisplay();
   }
}

vec2 screenToFractionalCoords(int x, int y)
{
   vec2 result(
      2.0 * x/(float)glutGet(GLUT_WINDOW_WIDTH)  - 1,
      -2.0 * y/(float)glutGet(GLUT_WINDOW_HEIGHT) + 1);
   return result;
}

void keyboard(unsigned char key, int x, int y ){
   switch( key ) {
   case 'r':
      // Reset all transformations
      cube_translate = vec3(0.0);
      cube_scale = 1.0;
      cube_rotate = mat4(1.0); //The identity matrix
      printf("View Reset\n");
      break;
   case 'q':
      exit(0);
      break;
   case 'a':
      nav_mode = MODE_PAN;
      printf("Mode: Pan\n");
      break;
   case 's':
      nav_mode = MODE_ROTATE;
      printf("Mode: Rotate\n");
      break;
   case 'd':
      nav_mode = MODE_SCALE;
      printf("Mode: Scale\n");
      break;
      
   }
   glutPostRedisplay();
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

   printf("\nA - Pan mode\nS - Rotate mode\nD - Scale mode\nR - Reset\nQ - Quit\n\n");
   
   glutMainLoop();
   return 0;
}
