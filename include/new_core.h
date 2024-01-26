#ifndef _NEW_CORE
#define _NEW_CORE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WITHOUT_ATTRIBUTES 0
#define WITH_ATTRIBUTES 1
#define FOR_REAL 0
#define FOR_DEPTH 1
#define WORLDX 100
#define WORLDY 100
#define WORLDZ 100
#define MAX_DISPLAY_LIST 500000

#define EMPTY 0
#define GREEN 1
#define BLUE 2
#define RED 3
#define BLACK 4
#define WHITE 5
#define PURPLE 6
#define ORANGE 7
#define COAL 8
#define STONE 9
#define BROWN 10
#define YELLOW 11
#define GRAY 9
#define GREY 9
#define DIRT 10

typedef struct {
  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;
  bool forwardPressed;
  bool backwardPressed;
  bool leftPressed;
  bool rightPressed;
  float speed;
  float height;
  bool shouldJump;
  bool jump;
  float jumpStart;
  float jumpSpeed;
  float maxFallSpeed;
  float maxJumpSpeed;
  float jumpTime;
  float fallSpeed;
  float gravity;
  bool canJump;
  float pitch;
  float yaw;
  float lightsUsedControl;
} Camera;

typedef struct {
  int vao;
  int size;
} Mesh;

typedef struct {
  Shader *shader;
  int specularTexture;
  float shininess;
  int ambientTexture; // currently unused
  int diffuseTexture;
  int emissionValues;
  int emissionMap;
  glm::vec3 ambientColor;
  glm::vec3 specularColor;
  glm::vec3 diffuseColor;
} Material;

typedef struct {
  Mesh *mesh; // vertex array object, created with createMesh
  Material *mat; // material created with createMaterial
  glm::vec3 pos;
  glm::vec3 rot;
  glm::vec3 scale;
  float angle;
} GameObject;

typedef struct {
  glm::vec3 specular;
  glm::vec3 diffuse;
  glm::vec3 ambient;
  glm::vec3 dir;
} DirLight;

typedef struct {
  Camera* cam;
  int displayCount; // held here so we can cache it
  int **displayList; // allocate to [MAX_DISPLAY_LIST][3];
  float **frustum; // allocate to [6][4]
  unsigned short world[WORLDX][WORLDY][WORLDZ];
  unsigned short visible[WORLDX][WORLDY][WORLDZ];
} GameContext;

#endif