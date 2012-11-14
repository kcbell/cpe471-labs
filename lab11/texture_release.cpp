//for CSC 471 - texture mapping lab
//ZJ WOOD

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

#include <iostream>
#include <math.h>
#include <assert.h>
#include <vector>

#define MAX_PITCH 0.88f

using namespace std;
using namespace glm;

/*data structure for the image used for  texture mapping */
typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

Image *TextureImage;

typedef struct RGB {
  GLubyte r;
  GLubyte g; 
  GLubyte b;
} RGB;

RGB myimage[64][64];
RGB* g_pixel;

//forward declaration of image loading and texture set-up code
int ImageLoad(char *filename, Image *image);
GLvoid LoadTexture(char* image_file, int tex_id);

//mode to toggle drawing
int cube;

//flag and ID to toggle on and off the shader
int ShadeProg, Shade2;

//Handles to the shader data
GLint h_uTexUnit;
GLint h_uTexUnit2;
GLint h_aPosition;
GLint h_aTexCoord;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uModelMatrix2;
GLint h_uViewMatrix2;
GLint h_uProjMatrix2;
GLuint CubeBuffObj, CIndxBuffObj, TexBuffObj, GTexBuffObj, GrndBuffObj, GIndxBuffObj;
int g_CiboLen, g_GiboLen;
static float  g_width, g_height;
float look_alpha = 0, look_beta = 1.57;
glm::vec3 camera_pos(0.0);
float mouseSensitivity = 5;
float walkSpeed = 0.1;

static const float g_groundY = -1.51;      // y coordinate of the ground
static const float g_groundSize = 10.0;   // half the ground length

/* projection matrix */
void SetProjectionMatrix() {
  glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
  safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
  safe_glUniformMatrix4fv(h_uProjMatrix2, glm::value_ptr(Projection));
}

glm::vec3 getLookPoint()
{
   return glm::vec3(cos(look_alpha)*cos(look_beta),
                  sin(look_alpha),
                  cos(look_alpha)*cos(1.57-look_beta));
}

/* camera controls - do not change */
void SetView() {
   glm::mat4 RotateX = glm::lookAt(camera_pos,
                                   camera_pos+getLookPoint(),
                                   vec3(0.0, 1.0, 0.0));
   //glm::mat4 RotateX = glm::rotate( Trans, g_angle, glm::vec3(0.0f, 1, 0));
   safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
   safe_glUniformMatrix4fv(h_uViewMatrix2, glm::value_ptr(RotateX));
}

/* set the model transform to the identity */
void SetModelI() {
  glm::mat4 tmp = glm::mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(tmp));
  safe_glUniformMatrix4fv(h_uModelMatrix2, glm::value_ptr(tmp));
}

static void initGround() {

  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };

    static GLfloat GrndTex[] = {
       0, 0,
       0, 1,
       1, 1,
       1, 0,
     };

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};


    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &GTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

}


/* intialize the cube data */
static void initCube() {

  float CubePos[] = {
    -1.5, -1.5, -1.5, //back face 5 verts :0 
    -1.5, 1.5, -1.5,
    1.5, 1.5, -1.5,
    1.5, -1.5, -1.5,
    1.5, -1.5, 1.5, //right face 5 verts :4
    1.5, 1.5, 1.5,
    1.5, 1.5, -1.5,
    1.5, -1.5, -1.5,
    -1.5, -1.5, 1.5, //front face 4 verts :8
    -1.5, 1.5, 1.5,
    1.5, 1.5, 1.5,
    1.5, -1.5, 1.5,
    -1.5, -1.5, -1.5, //left face 4 verts :12
    -1.5, 1.5, -1.5,
    -1.5, 1.5, 1.5,
    -1.5, -1.5, 1.5
  };

   static GLfloat CubeTex[] = {
      0, 0, // back 
      0, 1,
      0.25, 1,
      0.25, 0,
      0.5, 0, //right 
      0.5, 1,
      0.25, 1,
      0.25, 0,
      0.75, 0, //front 
      0.75, 1,
      0.5, 1,
      0.5, 0,
      1, 0, // left 
      1, 1,
      .75, 1,
      .75, 0
    }; 
   
    unsigned short idx[] = {0, 1, 2,  2, 3, 0,  4, 5, 6, 6, 7, 4,  8, 9, 10, 10, 11, 8,  12, 13, 14, 14, 15, 12};

    g_CiboLen = 24;
    glGenBuffers(1, &CubeBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

    glGenBuffers(1, &CIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &TexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeTex), CubeTex, GL_STATIC_DRAW);

}

void InitGeom() {
  initCube();
  initGround();

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
       h_aTexCoord = safe_glGetAttribLocation(ShadeProg,  "aTexCoord");
       h_uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
       h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
       h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
       h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
       printf("sucessfully installed shader %d\n", ShadeProg);
       return 1;

}

/* Some OpenGL initialization */
void Initialize ()                  // Any GL Init Code
{
    // Start Of User Initialization
    glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
    // Black Background
    glClearDepth (1.0f);    // Depth Buffer Setup
    glDepthFunc (GL_LEQUAL);    // The Type Of Depth Testing
    glEnable (GL_DEPTH_TEST);// Enable Depth Testing
    /* texture specific settings */
    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

/* Main display function */
void Draw (void)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (cube == 0) {
      //Start our shader
      glUseProgram(ShadeProg);

      //Set up matrix transforms
      SetProjectionMatrix();
      SetView();
      SetModelI();

      glEnable(GL_TEXTURE_2D);
      
      // Draw Cube
      
      //set up the texture unit
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
      safe_glUniform1i(h_uTexUnit, 0);

      safe_glEnableVertexAttribArray(h_aPosition);
      glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
      safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

      safe_glEnableVertexAttribArray(h_aTexCoord);
      glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
      safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

      // bind ibo
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
      glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);

      // Draw Ground
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 1);

    safe_glUniform1i(h_uTexUnit, 1);

    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, GTexBuffObj);
    safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // bind ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aTexCoord);

      //Disable the shader
      glUseProgram(0);
      glDisable(GL_TEXTURE_2D);
   } 
   glutSwapBuffers();

}

bool mouse_left_down = false;
int drag_begin_x, drag_begin_y;

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

vec2 screenToFractionalCoords(int x, int y)
{
   vec2 result(
      2.0 * x/(float)glutGet(GLUT_WINDOW_WIDTH)  - 1,
      -2.0 * y/(float)glutGet(GLUT_WINDOW_HEIGHT) + 1);
   return result;
}

void mouseMove(int x, int y)
{
   vec2 begin, end;
   float xChange, yChange;


   if (mouse_left_down)
   {
      begin = screenToFractionalCoords(drag_begin_x, drag_begin_y);
      end   = screenToFractionalCoords(x, y);
      xChange = end.x - begin.x;
      yChange = end.y - begin.y;
      
      look_alpha = std::min(MAX_PITCH, std::max(-MAX_PITCH, look_alpha + mouseSensitivity * yChange));
      //look_alpha += mouseSensitivity * yChange;
      look_beta  += mouseSensitivity * xChange;
      
      drag_begin_x = x;
      drag_begin_y = y;
      glutPostRedisplay();

      //printf("%f\n", look_beta);
   }      
}

void keyboard(unsigned char key, int x, int y )
{
   vec3 gaze(getLookPoint());
   vec3 w = normalize(gaze);
   vec3 u = normalize(cross(vec3(0,1,0), w));
   vec3 v = cross(w, u);

   switch( key ) {
      /* WASD keyes effect view/camera transform */
      case 'w':
         camera_pos += vec3(w.x,0,w.z) * walkSpeed;
         break;
      case 's':
         camera_pos -= vec3(w.x,0,w.z) * walkSpeed;
         break;
      case 'a':
         camera_pos += vec3(u.x,0,u.z) * walkSpeed;
         break;
      case 'd':
         camera_pos -= vec3(u.x,0,u.z) * walkSpeed;
         break;
      case 'q': case 'Q' :
         exit( EXIT_SUCCESS );
         break;
   }
   glutPostRedisplay();
}

//code to create a checker board texture...
void makeCheckerBoard ( int nRows, int nCols )
{
  g_pixel = new RGB[nRows * nCols];
  int c;
  
  long count = 0;
  for ( int i=0; i < nRows; i++ ) {
    for ( int j=0; j < nCols; j++ ) {
      count = j*nCols +i;
      c = (((i/8) + (j/8)) %2) * 255;
      g_pixel[count].r = c;
      g_pixel[count].g = c;
      g_pixel[count].b = c;
      
    }
  }
  /* set up the checker board texture as well */ 
  glBindTexture(GL_TEXTURE_2D, 2);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pixel);
}

/* Reshape */
void ReshapeGL (int width, int height)
{
    g_width = (float)width;
    g_height = (float)height;
    glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));

}

int main(int argc, char** argv) {

   //initialize the window
   glutInit(&argc, argv);
   glutInitWindowPosition( 20, 20 );
   glutInitWindowSize(400, 400);
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("My First texture maps");
   //set up the opengl call backs
   glutDisplayFunc(Draw);
   glutReshapeFunc(ReshapeGL);
   
   glutKeyboardFunc(keyboard);
   glutMouseFunc(mouse);
   glutMotionFunc(mouseMove);

   cube = 0;

   Initialize();
   //load in the other image textures
   LoadTexture((char *)"earth1.bmp", 0);
   LoadTexture((char *)"crate.bmp", 1);
   //make the checker board image
   makeCheckerBoard(64, 64);

   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"tex_vert.glsl"), textFileRead((char *)"tex_frag.glsl"))) {
         printf("Error installing shader!\n");
         return 0;
   }
   InitGeom();

   glutMainLoop();
  
}

//routines to load in a bmp files - must be 2^nx2^m and a 24bit bmp
GLvoid LoadTexture(char* image_file, int texID) { 
  
  TextureImage = (Image *) malloc(sizeof(Image));
  if (TextureImage == NULL) {
    printf("Error allocating space for image");
    exit(1);
  }
  cout << "trying to load " << image_file << endl;
  if (!ImageLoad(image_file, TextureImage)) {
    exit(1);
  }  
  /*  2d texture, level of detail 0 (normal), 3 components (red, green, blue),            */
  /*  x size from image, y size from image,                                              */    
  /*  border 0 (normal), rgb color data, unsigned byte data, data  */ 
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
    TextureImage->sizeX, TextureImage->sizeY,
    0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); /*  cheap scaling when image bigger than texture */    
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); /*  cheap scaling when image smalled than texture*/
  
}


/* BMP file loader loads a 24-bit bmp file only */

/*
* getint and getshort are help functions to load the bitmap byte by byte
*/
static unsigned int getint(FILE *fp) {
  int c, c1, c2, c3;
  
  /*  get 4 bytes */ 
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
  int c, c1;
  
  /* get 2 bytes*/
  c = getc(fp);  
  c1 = getc(fp);
  
  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/*  quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  */

int ImageLoad(char *filename, Image *image) {
  FILE *file;
  unsigned long size;                 /*  size of the image in bytes. */
  unsigned long i;                    /*  standard counter. */
  unsigned short int planes;          /*  number of planes in image (must be 1)  */
  unsigned short int bpp;             /*  number of bits per pixel (must be 24) */
  char temp;                          /*  used to convert bgr to rgb color. */
  
  /*  make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }
  
  /*  seek through the bmp header, up to the width height: */
  fseek(file, 18, SEEK_CUR);
  
  /*  No 100% errorchecking anymore!!! */
  
  /*  read the width */    image->sizeX = getint (file);
  
  /*  read the height */ 
  image->sizeY = getint (file);
  
  /*  calculate the size (assuming 24 bits or 3 bytes per pixel). */
  size = image->sizeX * image->sizeY * 3;
  
  /*  read the planes */    
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }
  
  /*  read the bpp */    
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }
  
  /*  seek past the rest of the bitmap header. */
  fseek(file, 24, SEEK_CUR);
  
  /*  read the data.  */
  image->data = (char *) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0; 
  }
  
  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }
  
  for (i=0;i<size;i+=3) { /*  reverse all of the colors. (bgr -> rgb) */
    temp = image->data[i];
    image->data[i] = image->data[i+2];
    image->data[i+2] = temp;
  }
  
  fclose(file); /* Close the file and release the filedes */
  
  /*  we're done. */
  return 1;
}




