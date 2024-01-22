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
} GameContext;

Material* createMaterial(Shader *shader, int specularTexture, float shininess, int diffuseTexture, glm::vec3 ambientColor, int emissionValues, int emissionMap)
{
  Material* mat = (Material *)malloc(sizeof(Material));

  mat->shader = shader;
  mat->specularTexture = specularTexture;
  mat->shininess = 16.0f;
  mat->diffuseTexture = diffuseTexture;
  mat->ambientColor = ambientColor;
  mat->emissionValues = emissionValues;
  mat->emissionMap = emissionMap;
  // only used by the light cubes; default vals for now
  mat->specularColor = glm::vec3(1.0f);
  mat->diffuseColor = glm::vec3(1.0f);
  // unused for now:
  mat->ambientTexture = -1;

  return mat;
};

void destroyMaterial(Material *mat)
{
  free(mat);
}

GameObject* createGameObject(Mesh *mesh, Material *mat, glm::vec3 pos)
{
  GameObject *gameObject = (GameObject *)malloc(sizeof(GameObject));
  gameObject->mesh = mesh;
  gameObject->mat = mat;
  gameObject->pos = pos;
  gameObject->rot = glm::vec3(1.0f);
  gameObject->scale = glm::vec3(1.0f);
  gameObject->angle = 0.0f;
  return gameObject;
}

void destroyGameObject(GameObject *gameObject)
{
  free(gameObject);
}

void renderGameObject(GameObject *gameObject, glm::mat4 view, glm::mat4 projection)
{
  Material *mat = gameObject->mat;
  Shader *shader = mat->shader;
  shader->use();

  unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
  unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
  unsigned int projLoc = glGetUniformLocation(shader->ID, "projection");

  shader->setInt("material.specular", mat->specularTexture);
  shader->setFloat("material.shininess", mat->shininess);
  shader->setInt("material.diffuse", mat->diffuseTexture);
  shader->setVec3f("material.ambient", mat->ambientColor.r, mat->ambientColor.g, mat->ambientColor.b);
  shader->setInt("material.emission", mat->emissionValues);
  shader->setInt("material.emission_map", mat->emissionMap);

  glm::mat4 model = glm::translate(glm::mat4(1.0f), gameObject->pos);
  model = glm::rotate(model, gameObject->angle, gameObject->rot);
  model = glm::scale(model, gameObject->scale);

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  glBindVertexArray(gameObject->mesh->vao);
  glDrawArrays(GL_TRIANGLES, 0, gameObject->mesh->size);
}

void renderSkybox(GameObject *skybox, glm::mat4 view, glm::mat4 projection)
{
  Shader *shader = skybox->mat->shader;
  shader->use();

  glDepthMask(GL_FALSE);
  glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
  unsigned int skyboxViewLoc = glGetUniformLocation(shader->ID, "view");
  unsigned int skyboxProjLoc = glGetUniformLocation(shader->ID, "projection");
  glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(skyboxView));
  glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
  glBindVertexArray(skybox->mesh->vao);
  glDrawArrays(GL_TRIANGLES, 0, skybox->mesh->size);
  glDepthMask(GL_TRUE);
}

void setupCam(Camera* cam)
{
  cam->pos = glm::vec3(0.0f, 0.0f,  3.0f);
  cam->front = glm::vec3(0.0f, 0.0f, -1.0f);
  cam->up = glm::vec3(0.0f, 1.0f,  0.0f);
  cam->forwardPressed = false;
  cam->backwardPressed = false;
  cam->leftPressed = false;
  cam->rightPressed = false;
  cam->speed = 2.5f;
  cam->height = 0;
  cam->jump = false;
  cam->shouldJump = false;
  cam->jumpStart = 0.0f;
  cam->jumpSpeed = 0.0f;
  cam->maxJumpSpeed = 4.5f;
  cam->jumpTime = 1.29f;
  cam->fallSpeed = 0.0f;
  cam->maxFallSpeed = 7.0f;
  cam->gravity = 3.5f;
  cam->canJump = true;
  cam->pitch = 0.0f;
  cam->yaw = -90.0f;
  cam->lightsUsedControl = 1.0f;
}

void processCamera(Camera* cam, float deltaTime, float currentFrame)
{
  float deltaSpeed = cam->speed * deltaTime;

  if (!cam->jump && cam->shouldJump) {
    cam->shouldJump = false;
    cam->canJump = false;
    cam->jump = true;
    cam->jumpStart = currentFrame;
    cam->jumpSpeed = cam->maxJumpSpeed;
  }

  if (cam->forwardPressed) {
    cam->forwardPressed = false;
    cam->pos += deltaSpeed * cam->front;
  }

  if (cam->backwardPressed) {
    cam->backwardPressed = false;
    cam->pos -= deltaSpeed * cam->front;
  }

  if (cam->leftPressed) {
    cam->leftPressed = false;
    cam->pos -= glm::normalize(glm::cross(cam->front, cam->up)) * deltaSpeed;
  }

  if (cam->rightPressed) {
    cam->rightPressed = false;
    cam->pos += glm::normalize(glm::cross(cam->front, cam->up)) * deltaSpeed;
  }

  if (cam->jump == true) {
    float jumpStartDelta = currentFrame - cam->jumpStart;

    // jump runs for "2 seconds", or something.
    if (jumpStartDelta < cam->jumpTime) {
      cam->height += cam->jumpSpeed * deltaTime;
      cam->jumpSpeed -= cam->gravity * deltaTime;

      if (cam->jumpSpeed < 0.0f) {
        //printf("redundant jump speed call: %f\n", cam->jumpSpeed);
        cam->jumpSpeed = 0;
      }
    } else {
      cam->jump = false;
      cam->jumpStart = 0;
      cam->fallSpeed = cam->jumpSpeed;
    }
  } else if (cam->height > 0.1f) {
    cam->height -= cam->fallSpeed * deltaTime;
    cam->fallSpeed += cam->gravity * deltaTime;

    if (cam->fallSpeed > cam->maxFallSpeed) {
      cam->fallSpeed = cam->maxFallSpeed;
    }
  } else if (cam->height < 0.1f) {
    cam->canJump = true;
    cam->height = 0.0f;
    cam->fallSpeed = 0.0f;
  }

  cam->pos.y = cam->height;

  glm::vec3 direction;
  direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
  direction.y = sin(glm::radians(cam->pitch));
  direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
  cam->front = glm::normalize(direction);
}

void changeCameraAngles(Camera *cam, float xoffset, float yoffset)
{
  cam->yaw   += xoffset;
  cam->pitch += yoffset;

  // keeps us locked to sane angles
  if (cam->pitch > 89.0f) {
    cam->pitch =  89.0f;
  }

  if (cam->pitch < -89.0f) {
    cam->pitch = -89.0f;
  }
}

void setupDirLightDefaults(DirLight *light)
{
  light->dir = glm::vec3(0.2f,  -0.4f,  1.0f);
  light->specular = glm::vec3(1.0f);
  light->diffuse = light->specular * 0.65f;
  light->ambient = light->specular * 0.3f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  printf("famebuffer_size changed: %d, %d\n", width, height);
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  static float lastX = 0, lastY = 0;
  static bool firstMouse = true;

  if (firstMouse) { // initially set to true; prevents a big jump at first load
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
  lastX = xpos;
  lastY = ypos;

  // controls how much the mouse actually moves the camera
  const float sensitivity = 0.4f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  GameContext* game = (GameContext *) glfwGetWindowUserPointer(window);
  changeCameraAngles(game->cam, xoffset, yoffset);
}

void processInput(GLFWwindow *window, Camera* cam)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  cam->shouldJump = false;

  if (cam->canJump && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    cam->shouldJump = true;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cam->forwardPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cam->backwardPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cam->leftPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cam->rightPressed = true;
  }
}

int loadTexture(int tex_number, const char *path)
{
  int width, height, nrChannels;
  unsigned int texture;
  glGenTextures(1, &texture);
  std::cout << "loadTexture texture id set to: " << texture << " for path " << path << std::endl;
  glActiveTexture(GL_TEXTURE0 + tex_number);
  glBindTexture(GL_TEXTURE_2D, texture);

  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

  if (data) {
    int pathLen = strlen(path);

    if (strcmp(&path[pathLen - 4], ".jpg") == 0) {
      printf("got jpg for path: %s\n", path);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
      printf("got non-jpg for path: %s\n", path);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);
  return texture;
}

Mesh *createMesh(float *vertices, unsigned int numVertices, unsigned int array_size, int with_attributes)
{
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  //unsigned int EBO;
  //glGenBuffers(1, &EBO);

  // bind the vertex array
  glBindVertexArray(VAO);

  // bind the array buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, array_size, vertices, GL_STATIC_DRAW);

  // bind the element buffer
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  /* end declare vertices */

  /* set up attribute arrays */
  if (with_attributes == WITH_ATTRIBUTES) {
    // vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture attributes
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
  } else {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
  }

  Mesh *mesh = (Mesh *)malloc(sizeof(Mesh));
  mesh->vao = VAO;
  mesh->size = numVertices;
  return mesh;
}

void destroyMesh(Mesh *mesh)
{
  free(mesh);
}

unsigned int loadCubemap(int tex_number, std::vector<std::string> faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  std::cout << "cubemap textureID set to: " << textureID << std::endl;
  glActiveTexture(GL_TEXTURE0 + textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;

  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

    if (data) {
      if (nrChannels == 3) {
        std::cout << "3 channels and w,h " << width << ", " << height << " at path: " << faces[i] << std::endl;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                    );
      } else if (nrChannels == 4) {
        if (width != height) {
          height = width;
        }

        std::cout << "4 channels and w,h " << width << ", " << height << " at path: " << faces[i] << std::endl;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
                    );
      }

      stbi_image_free(data);
    } else {
      std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

void renderScene(GameObject *skybox, GameObject *plane, GameObject *cube, glm::mat4 view, glm::mat4 projection, int mode)
{
  renderSkybox(skybox, view, projection);
  renderGameObject(plane, view, projection);
  renderGameObject(cube, view, projection);
}

int main(int argc, char** argv)
{
  const int WINDOW_WIDTH = 1280;
  const int WINDOW_HEIGHT = 720;
  GLFWwindow* window;
  GameContext game;
  Camera cam;
  setupCam(&cam);
  game.cam = &cam;
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame
  DirLight dirLight;
  setupDirLightDefaults(&dirLight);

  /* Initialize the library */
  if (!glfwInit()) {
    return -1;
  }

#ifdef __APPLE__
  /* We need to explicitly ask for a 4.1 context on OS X */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Triangle", NULL, NULL);

  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return -1;
  }

  glfwSetWindowUserPointer(window, (void *)&game);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  /* texture loading */
  std::vector<std::string> vfaces = {
    "images/skybox/tutorial/right.jpg",
    "images/skybox/tutorial/left.jpg",
    "images/skybox/tutorial/top.jpg",
    "images/skybox/tutorial/bottom.jpg",
    "images/skybox/tutorial/front.jpg",
    "images/skybox/tutorial/back.jpg"
  };

  // flip the rest of the images around vertically
  stbi_set_flip_vertically_on_load(true);
  unsigned int container = loadTexture(1, "images/container.jpg");
  unsigned int container2 = loadTexture(2, "images/container2.png");
  unsigned int container2_specular = loadTexture(3, "images/container2_specular.png");
  unsigned int container2_emission_map = loadTexture(4, "images/container2_emission_map.png");
  unsigned int generic01 = loadTexture(5, "images/altdev/generic-07.png");
  unsigned int generic02 = loadTexture(6, "images/altdev/generic-12.png");
  unsigned int awesomeface = loadTexture(7, "images/awesomeface.png");
  unsigned int matrixTexture = loadTexture(8, "images/matrix.jpg");
  unsigned int blankTexture = loadTexture(9, "images/1x1.png");
  stbi_set_flip_vertically_on_load(false);
  unsigned int skyboxTexture = loadCubemap(10, vfaces);

  /* end texture loading */
  Shader lightingShader("src/shaders/lighting_shader.vs", "src/shaders/lighting_shader.fs");
  //Shader lightCubeShader("shaders/light_cube_shader.vs", "shaders/light_cube_shader.fs");
  Shader skyboxShader("src/shaders/skybox_shader.vs", "src/shaders/skybox_shader.fs");
  //Shader debugDepthShader("shaders/lighting_shader.vs", "shaders/debug_quad.fs");

  glm::vec3 defaultAmbientColor = glm::vec3(0.2f);
  //                                            (shader,           specular,            shininess,  diffuse,       ambient,             emissionVals,  emissionMap);
  Material *containerMaterial   = createMaterial(&lightingShader,  blankTexture,        16.0f,      container,     defaultAmbientColor, blankTexture,  blankTexture);
  Material *container2Material  = createMaterial(&lightingShader,  container2_specular, 16.0f,      container2,    defaultAmbientColor, matrixTexture, container2_emission_map);
  Material *awesomefaceMaterial = createMaterial(&lightingShader,  blankTexture,        64.0f,      awesomeface,   defaultAmbientColor, awesomeface,   awesomeface);
  Material *generic01Material   = createMaterial(&lightingShader,  blankTexture,        16.0f,      generic01,     defaultAmbientColor, blankTexture,  blankTexture);
  Material *generic02Material   = createMaterial(&lightingShader,  blankTexture,        16.0f,      generic02,     defaultAmbientColor, blankTexture,  blankTexture);
  Material *skyboxMaterial      = createMaterial(&skyboxShader,    blankTexture,        16.0f,      skyboxTexture, defaultAmbientColor, blankTexture,  blankTexture);
  //Material *depthMaterial       = createMaterial(&debugDepthShader, blankTexture,       16.0f,      generic01,  defaultAmbientColor, blankTexture,  blankTexture);

  /* declare vertices */
  float vertices_cube[] = {
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
    0.5f,  -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
    0.5f,   0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
    0.5f,   0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
    0.5f,  -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
    0.5f,  -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
    0.5f,  -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    0.5f,  -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
    0.5f,   0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f
  };

  float vertices_plane[] = {
    -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,    0.0f,   0.0f,
    0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,    100.0f, 0.0f,
    0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,    100.0f, 100.0f,
    -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,    0.0f,   0.0f,
    -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,    0.0f,   100.0f,
    0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,    100.0f, 100.0f,
  };

  float vertices_skybox[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
  };

  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);

  const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
               SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Mesh *cubeMesh = createMesh(vertices_cube, 36, sizeof(vertices_cube), WITH_ATTRIBUTES);
  Mesh *planeMesh = createMesh(vertices_plane, 6, sizeof(vertices_plane), WITH_ATTRIBUTES);
  //Mesh *quadMesh = createMesh(vertices_quad, 6, sizeof(vertices_quad), WITH_ATTRIBUTES);
  Mesh *skyboxMesh = createMesh(vertices_skybox, 36, sizeof(vertices_skybox), WITHOUT_ATTRIBUTES);

  GameObject *plane = createGameObject(planeMesh, generic02Material, glm::vec3(0.0f, -0.5f, 0.0f));
  GameObject *cube = createGameObject(cubeMesh, container2Material, glm::vec3(0.0f, 1.0f, 0.0f));
  //GameObject *debugQuad = createGameObject(quadMesh, depthMaterial, glm::vec3(1.0f, 0.5f, 0.0f));
  //GameObject *wall = createGameObject(cubeMesh, generic01Material, glm::vec3(0.0f));
  GameObject *skybox = createGameObject(skyboxMesh, skyboxMaterial, glm::vec3(0.0f));
  plane->scale = glm::vec3(100.0f, 0.0f, 100.0f);
  //debugQuad->rot = glm::vec3(1.0f, 0.0f, 0.0f);
  //debugQuad->angle = glm::radians(90.0f);
  //debugQuad->mat->diffuseTexture = depthMap;
  //debugQuad->mat->specularTexture = depthMap;

  skyboxShader.use();
  skyboxShader.setInt("skybox", skyboxTexture);
  //debugDepthShader.use();
  //debugDepthShader.setInt("depthMap", depthMap);
  lightingShader.use(); // don't forget to activate the shader before setting uniforms!
  lightingShader.setInt("skybox", skyboxTexture);
  lightingShader.setInt("shadowMap", depthMap);

  lightingShader.setVec3f("dirLight.dir", dirLight.dir.x, dirLight.dir.y, dirLight.dir.z);
  lightingShader.setVec3f("dirLight.diffuse", dirLight.diffuse.r, dirLight.diffuse.g, dirLight.diffuse.b);
  lightingShader.setVec3f("dirLight.ambient", dirLight.ambient.r, dirLight.ambient.g, dirLight.ambient.b);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window, &cam);
    processCamera(&cam, deltaTime, currentFrame);

    // proj and view set up for depth buffer
    float near_plane = 10.0f, far_plane = 64.0f;
    glm::mat4 projection = glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, near_plane, far_plane);
    glm::mat4 view = glm::lookAt(glm::vec3(-15.0f, 19.0f, -30.0f),
                                 glm::vec3(0.0f, 0.0f,  0.0f),
                                 glm::vec3(0.0f, 1.0f,  0.0f));
    glm::mat4 lightSpaceMatrix = projection * view;
    unsigned int lightSpaceLoc = glGetUniformLocation(lightingShader.ID, "lightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    // lights begin
    lightingShader.use(); // used for everything kinda
    //lightingShader.setInt("lightsUsed", lightsUsed);
    lightingShader.setVec3f("viewPos", cam.pos.x, cam.pos.y, cam.pos.z);  // this is the "player cam pos" :/

    // sendPointLightColors(&lightingShader, pointLights, lightsUsed);
    // sendPointLightPositions(&lightingShader, pointLights, lightsUsed);

    // for shadow mapping:
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    // render to depth buffer
    renderScene(skybox, plane, cube, view, projection, FOR_REAL);

    // put framebuffer back to normal
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

    projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    view = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);

    //debugDepthShader.use();
    //debugDepthShader.setFloat("near_plane", near_plane);
    //debugDepthShader.setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE0 + depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    //renderGameObject(debugQuad, view, projection);
    renderScene(skybox, plane, cube, view, projection, FOR_REAL);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}