#ifndef PLAYGROUND_H
#define PLAYGROUND_H

/** In this example, textures were added to the simple clock. 
 *  All changes required for the textures are marked with “NEW” 
 *  in the comments. Don’t forget to specify the new files in 
 *  CMake if you want to try out the example.
 *
 *  Created by Alfred Franz with AI assistance from Claude Opus 5.5
 */

// Include GLEW
#include <GL/glew.h>

//some global variables for handling the vertex buffer
GLuint vertexbuffer;
GLuint VertexArrayID;
GLuint vertexbuffer_size;

//NEW: texture coordinates (UVs) of the clock hand (one UV per vertex, never change)
GLuint uvbuffer;

//NEW: vertex and UV buffer of the background quad (two triangles covering the whole window)
GLuint backgroundVertexbuffer;
GLuint backgroundUVbuffer;

//NEW: the two textures and the location of the sampler uniform in the fragment shader
GLuint textureHand;
GLuint textureBackground;
GLuint textureSamplerID;

//program ID of the shaders, required for handling the shaders with OpenGL
GLuint programID;

//Storing the seconds for output
int seconds_last;

int main( void ); //<<< main function, called at startup
void updateAnimationLoop(); //<<< updates the animation loop
bool initializeWindow(); //<<< initializes the window using GLFW and GLEW
bool initializeVertexbuffer(); //<<< initializes the vertex buffer
bool initializeUVbuffer(); //<<< NEW: initializes the texture coordinates of the clock hand and the background
bool initializeTextures(); //<<< NEW: loads the two textures from BMP files
void drawTextured(GLuint vbo, GLuint uvbo, GLuint texture, int vertexCount); //<<< NEW: draws triangles with a texture
bool cleanupVertexbuffer(); //<<< frees all recources from the vertex buffer
bool closeWindow(); //<<< Closes the OpenGL window and terminates GLFW

#endif
