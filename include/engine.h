#ifndef _ENGINE
#define _ENGINE

#include <stdbool.h>

#ifdef UBUNTU
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
// as of 2023-12, required to silence 'OpenGL API deprecated. (Define GL_SILENCE_DEPRECATION to silence these warnings) [-Wdeprecated-declarations]'
#define GL_SILENCE_DEPRECATION 1
#include <GLUT/glut.h>
#endif

/* world size and storage array */
#define WORLDX 100
#define WORLDY 100
#define WORLDZ 100
GLubyte  world[WORLDX][WORLDY][WORLDZ];
GLubyte  visible[WORLDX][WORLDY][WORLDZ];

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

#define PI 3.1415
#define MOB_COUNT 10
#define PLAYER_COUNT 10
#define MAX_DISPLAY_LIST 500000

void parseArgs(int, char **);
void initializeOpenGL();
void setLightPosition(GLfloat, GLfloat, GLfloat);
GLfloat* getLightPosition();

void setViewPosition(float, float, float);
void getViewPosition(float *, float *, float *);
void getOldViewPosition(float *, float *, float *);
void getViewOrientation(float *, float *, float *);

int addDisplayList(int, int, int);

void initPlayerArray();
void createPlayer(int number, float x, float y, float z, float rotx, float roty);
void setPlayerPosition(int number, float x, float y, float z, float rotx, float roty);
void hidePlayer(int number);
void showPlayer(int number);

void initMobArray();
void createMob(int, float, float, float, float, float);
void setMobPosition(int, float, float, float, float, float);
void hideMob(int);
void showMob(int);

void update();
void buildDisplayList();

typedef struct {
  float x;
  float y;
  float z;
} Vec3f;

typedef struct {
  Vec3f pos;
  Vec3f oldpos;
  Vec3f rot;
  Vec3f speed;
  bool visible;
  bool flag;
} Player;

typedef struct {
  Vec3f pos;
  Vec3f oldpos;
  Vec3f rot;
  Vec3f speed;
  bool visible;
  bool flag;
} Mob;

extern int keyStates[256];
/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* list and count of polygons to be displayed, set during culling */
int displayList[MAX_DISPLAY_LIST][3];
int displayCount;

extern Player players[PLAYER_COUNT];
extern Mob mobs[MOB_COUNT];

extern int clouds_flag;
extern int sun_flag;
extern int highlight[3];
/* flag to print out frames per second */
extern int showFPS;
extern int fullscreen;
extern int randomSeedGiven;
extern int randomSeed;

/* flag to indicate removal of cube the viewer is facing */
extern int displayAllCubes;
extern int dig;
extern int digflag[4];
#endif
