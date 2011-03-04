#ifdef UBUNTU
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#else
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
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
#define MAX_DISPLAY_LIST 500000

void graphicsInit(int *, char **);
void setLightPosition(GLfloat, GLfloat, GLfloat);
GLfloat* getLightPosition();

void setViewPosition(float, float, float);
void getViewPosition(float *, float *, float *);
void getOldViewPosition(float *, float *, float *);
void getViewOrientation(float *, float *, float *);

int addDisplayList(int, int, int);

void createMob(int, float, float, float, float);
void setMobPosition(int, float, float, float, float);
void hideMob(int);
void showMob(int);

void update();
void collisionResponse();
void buildDisplayList();

/* flag which is set to 1 when flying behaviour is desired */
int flycontrol;
/* flag used to indicate that the test world should be used */
int testWorld;
/* list and count of polygons to be displayed, set during culling */
int displayList[MAX_DISPLAY_LIST][3];
int displayCount;
/* flag to print out frames per second */
int fps;
/* flag to indicate removal of cube the viewer is facing */
int dig;
