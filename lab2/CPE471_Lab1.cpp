/*
 *  CPE 471 lab 1 - draw a triangle
 *  glut/OpenGL application which renders a tri.  
 *  works in conjunction with a GLSL vertex shader to do color interpolation 
 *
 *  Created by zwood on 12/30/11 
 *  Copyright 2010 Cal Poly. All rights reserved.
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
#include "GLSL_helper.h"

/* the vertex data */
GLfloat vertexPos[] = {
 -0.5, -0.5, 0.0, 1.0,
 0, 0.5, 0.0, 1.0,
 0.5, -0.5, 0.0, 1.0
};

/* the color data */
static GLfloat vertexCol[] = {
 0.0, 0.5, 0.5, 
 0.0, 0.0, 0.5, 
 0.0, 0.5, 0.0, 
};

//position and color data handles
GLuint triBuffObj, colBuffObj;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;

//Handles to the shader data
GLint h_aPosition;
GLint h_aColor;

/* initialize the geomtry (including color)*/
void InitGeom() {
  glGenBuffers(1, &triBuffObj);
  glGenBuffers(1, &colBuffObj);

  glBindBuffer(GL_ARRAY_BUFFER, triBuffObj);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPos), vertexPos, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, colBuffObj);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCol), vertexCol, GL_STATIC_DRAW);
}


/*function to help load the shader */
int InstallShader(const GLchar *vShaderName) {
	GLuint VS; //handles to shader object
	GLint vCompiled, linked; //status of shader
	
	VS = glCreateShader(GL_VERTEX_SHADER);
	
	//load the source
	glShaderSource(VS, 1, &vShaderName, NULL);
	
	//compile shader and print log
	glCompileShader(VS);
	/* check shader status requires helper functions */
	printOpenGLError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
	printShaderInfoLog(VS);
	
	if (!vCompiled) {
		printf("Error compiling the shader %s", vShaderName);
		return 0;
	}
	 
	//create a program object and attach the compiled shader
	ShadeProg = glCreateProgram();
	glAttachShader(ShadeProg, VS);
	
	glLinkProgram(ShadeProg);
	/* check shader status requires helper functions */
	printOpenGLError();
	glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
	printProgramInfoLog(ShadeProg);

	glUseProgram(ShadeProg);
	
	/* get handles to attribute data */
	h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
	h_aColor = safe_glGetAttribLocation(ShadeProg,	"aColor");

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

}


/* Main display function */
void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
	//Start our shader	
 	glUseProgram(ShadeProg);

	//data set up to access the vertices and color
  	safe_glEnableVertexAttribArray(h_aPosition);
 	glBindBuffer(GL_ARRAY_BUFFER, triBuffObj);
	safe_glVertexAttribPointer(h_aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
  	safe_glEnableVertexAttribArray(h_aColor);
 	glBindBuffer(GL_ARRAY_BUFFER, colBuffObj);
	safe_glVertexAttribPointer(h_aColor, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//actually draw the data
	glDrawArrays(GL_TRIANGLES, 0, 3);	
													
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
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				
}


int main( int argc, char *argv[] )
{
   	glutInit( &argc, argv );
   	glutInitWindowPosition( 20, 20 );
   	glutInitWindowSize( 400, 400 );
   	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   	glutCreateWindow("My first triangle");
   	glutReshapeFunc( ReshapeGL );
   	glutDisplayFunc( Draw );
   	Initialize();
	
	//test the openGL version
	getGLversion();
	//install the shader
	if (!InstallShader(textFileRead((char *)"GLSL_Lab1.glsl"))) {
		printf("Error installing shader!\n");
		return 0;
	}
		
	InitGeom();
  	glutMainLoop();
   	return 0;
}
