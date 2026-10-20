#include "playground.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <time.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp> //NEW: provides loadBMP_custom()

int main( void )
{
  //Initialize window
  bool windowInitialized = initializeWindow();
  if (!windowInitialized) return -1;

  //Initialize vertex buffer
  bool vertexbufferInitialized = initializeVertexbuffer();
  if (!vertexbufferInitialized) return -1;

  //NEW: Initialize the buffers that never change (UVs of the hand, background quad)
  initializeUVbuffer();
  initializeBackground();

  // Create and compile our GLSL program from the shaders
  // NEW: shaders with texture support (the old Simple*Shader files are no longer used)
  programID = LoadShaders("TextureVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

  //NEW: Load the textures and get the location of the sampler uniform in the fragment shader
  bool texturesInitialized = initializeTextures();
  if (!texturesInitialized) return -1;
  textureSamplerID = glGetUniformLocation(programID, "myTextureSampler");

	//start animation loop until escape key is pressed
	do{
    initializeVertexbuffer(); //hack: reinitialize vertex buffer every iteration
    updateAnimationLoop();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );


  //Cleanup and close window
  cleanupVertexbuffer();
  glDeleteProgram(programID);
	closeWindow();

	return 0;
}

void updateAnimationLoop()
{
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT);

  // Use our shader
  glUseProgram(programID);

  //NEW: Draw the background first (clock face), then the hand on top of it.
  //     Without depth test, whatever is drawn later covers what was drawn before.
  drawTextured(backgroundVertexbuffer, backgroundUVbuffer, textureBackground, 6); // 6 vertices -> 2 triangles
  drawTextured(vertexbuffer, uvbuffer, textureHand, 3);                            // 3 vertices -> 1 triangle

  // Swap buffers
  glfwSwapBuffers(window);
  glfwPollEvents();
}

//NEW: Draws triangles from a vertex buffer and a UV buffer, using the given texture
void drawTextured(GLuint vbo, GLuint uvbo, GLuint texture, int vertexCount)
{
  // Bind the texture to texture unit 0 and tell the sampler in the shader to use unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(textureSamplerID, 0);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size : x,y,z
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // NEW: 2nd attribute buffer : UVs
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvbo);
  glVertexAttribPointer(
    1,                  // attribute 1, must match "layout(location = 1)" in the vertex shader
    2,                  // size : u,v
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // Draw the triangles ! (the count is the number of vertices, not the buffer size in bytes)
  glDrawArrays(GL_TRIANGLES, 0, vertexCount);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

bool initializeWindow()
{
  // Initialise GLFW
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    char c = getchar();
    return false;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(600, 600, "Tutorial 02 - Red triangle", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    char c = getchar();
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    char c = getchar();
    glfwTerminate();
    return false;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  return true;
}

bool initializeVertexbuffer()
{
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  glm::vec2 triangleVertice1 = glm::vec2(-0.05f, -0.05f);
  glm::vec2 triangleVertice2 = glm::vec2(0.05f, -0.05f);
  glm::vec2 triangleVertice3 = glm::vec2(0.0f, 0.8f);

  time_t seconds;
  seconds = time(NULL); //Gives seconds since Epoch (e.g., 1 Jan 1970)
  if (seconds_last != (seconds % 60)) {
    std::cout << "Seconds: " << (seconds % 60) << std::endl; //(seconds % 60) gives the second of current minute
    seconds_last = (seconds % 60);
  }
  float secondRad = (seconds % 60) / 60.0 * (3.14 * 2.0); //conversion to radiants

  glm::mat2 myR = glm::mat2(cos(secondRad), -sin(secondRad),
                          sin(secondRad), cos(secondRad));

  glm::vec2 myT = glm::vec2(0.0f, 0.0f);

  triangleVertice1 = myR * triangleVertice1 + myT;
  triangleVertice2 = myR * triangleVertice2 + myT;
  triangleVertice3 = myR * triangleVertice3 + myT;

  std::vector< glm::vec3 > vertices = std::vector< glm::vec3 >();

  vertices.push_back({ triangleVertice1[0], triangleVertice1[1], 0.0f });
  vertices.push_back({ triangleVertice2[0], triangleVertice2[1], 0.0f });
  vertices.push_back({ triangleVertice3[0], triangleVertice3[1], 0.0f });
  vertexbuffer_size = (GLuint)vertices.size() * (GLuint)sizeof(glm::vec3);

  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexbuffer_size, &vertices[0], GL_STATIC_DRAW);

  return true;
}

//NEW: UV coordinates of the hand. One (u,v) per vertex, in the same order as the vertices.
//     (0,0) is the lower left corner of the image, (1,1) the upper right corner.
//     The UVs belong to the triangle, not to the screen: the texture rotates with the hand.
bool initializeUVbuffer()
{
  std::vector< glm::vec2 > uvs = std::vector< glm::vec2 >();
  uvs.push_back({ 0.0f, 0.0f }); // vertex 1: lower left  -> lower left of the image
  uvs.push_back({ 1.0f, 0.0f }); // vertex 2: lower right -> lower right of the image
  uvs.push_back({ 0.5f, 1.0f }); // vertex 3: tip         -> upper middle of the image

  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
  return true;
}

//NEW: Background quad covering the whole window (x and y from -1 to 1), built from two triangles.
//     Its UVs map the full image (0..1) onto the window.
bool initializeBackground()
{
  std::vector< glm::vec3 > vertices = std::vector< glm::vec3 >();
  std::vector< glm::vec2 > uvs = std::vector< glm::vec2 >();

  // triangle 1: lower left, lower right, upper right
  vertices.push_back({ -1.0f, -1.0f, 0.0f }); uvs.push_back({ 0.0f, 0.0f });
  vertices.push_back({  1.0f, -1.0f, 0.0f }); uvs.push_back({ 1.0f, 0.0f });
  vertices.push_back({  1.0f,  1.0f, 0.0f }); uvs.push_back({ 1.0f, 1.0f });
  // triangle 2: lower left, upper right, upper left
  vertices.push_back({ -1.0f, -1.0f, 0.0f }); uvs.push_back({ 0.0f, 0.0f });
  vertices.push_back({  1.0f,  1.0f, 0.0f }); uvs.push_back({ 1.0f, 1.0f });
  vertices.push_back({ -1.0f,  1.0f, 0.0f }); uvs.push_back({ 0.0f, 1.0f });

  glGenBuffers(1, &backgroundVertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, backgroundVertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &backgroundUVbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, backgroundUVbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
  return true;
}

//NEW: Loads the two textures. The BMP files must be 24 bit, uncompressed,
//     with a width that is a multiple of 4 (limitation of loadBMP_custom).
bool initializeTextures()
{
  textureBackground = loadBMP_custom("clockface.bmp");
  textureHand = loadBMP_custom("hand.bmp");
  return (textureBackground != 0 && textureHand != 0);
}

bool cleanupVertexbuffer()
{
  // Cleanup VBO
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);               //NEW
  glDeleteBuffers(1, &backgroundVertexbuffer); //NEW
  glDeleteBuffers(1, &backgroundUVbuffer);     //NEW
  glDeleteTextures(1, &textureHand);           //NEW
  glDeleteTextures(1, &textureBackground);     //NEW
  glDeleteVertexArrays(1, &VertexArrayID);
  return true;
}

bool closeWindow()
{
  glfwTerminate();
  return true;
}
