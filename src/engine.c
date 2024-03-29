/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "engine.h"
#include "network.h"

int keyStates[256];

/* flags used to control the appearance of the image */
int lineDrawing = 0;  // draw polygons as solid or lines
int lighting = 1;  // use diffuse and specular lighting
int smoothShading = 1;  // smooth or flat shading
int textures = 0;

/* texture data */
GLubyte  Image[64][64][4];
GLuint   textureID[1];

float windowWidth, windowHeight;

/* mouse direction coordiates */
float mvx = 0.0, mvy = 0.0, mvz = 0.0;

/* location for the light source (the sun), the first three
     values are the x,y,z coordinates */
GLfloat lightPosition[] = {0.0, 99.0, 0.0, 0.0};
/* location for light source that is kept at viewpoint location */
GLfloat viewpointLight[] = { -50.0, -50.0, -50.0, 1.0};

/* command line flags */
int flycontrol = 0;  // allow viewpoint to move in y axis when 1
int displayAllCubes = 0;  // draw all of the cubes in the world when 1
int testWorld = 0;  // sample world for timing tests
int showFPS = 0;  // turn on frame per second output
int fullscreen = 0; // render a fullscreen window or not
int randomSeedGiven = 0; // flag for whether a specific seed was supplied
int randomSeed = 0; // random seed will be set to time(NULL) if not passed in

/* list of cubes to display */
int displayList[MAX_DISPLAY_LIST][3];
int displayCount = 0; // count of cubes in displayList[][]

Mob mobs[MOB_COUNT];
Player players[PLAYER_COUNT];
int sun_flag = 0;
int clouds_flag = 0;
/* flag indicating the user wants the cube in front of them removed */
int dig = 0;
int digflag[4];
/***************/


/* player control functions */
/* set all player location, rotation, and visibility values to zero */
void initPlayerArray()
{
  for (int i = 0; i < PLAYER_COUNT; i++) {
    players[i].pos.x = 0;
    players[i].pos.y = 0;
    players[i].pos.z = 0;
    players[i].rot.x = 0;
    players[i].rot.y = 0;
    players[i].rot.z = 0;
    players[i].visible = false;
    players[i].flag = false;
  }
}

/* create player with identifier "number" at x,y,z with */
/* heading of rotx, roty, rotz in deg */
void createPlayer(int number, float x, float y, float z, float rotx, float roty)
{
  if (number >= PLAYER_COUNT) {
    printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
    exit(1);
  }

  players[number].pos.x = x;
  players[number].pos.y = y;
  players[number].pos.z = z;
  players[number].rot.x = x;
  players[number].rot.y = y;
  players[number].rot.z = 0; // z-rotation is unused
  players[number].visible = true;
}

/* move player to a new position xyz with rotation rotx,roty,rotz */
void setPlayerPosition(int number, float x, float y, float z, float rotx, float roty)
{
  if (number >= PLAYER_COUNT) {
    printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
    exit(1);
  }

  if (number == identity) {
    printf("p%d(x,y,z): %f,%f,%f\n", number, players[number].pos.x, players[number].pos.y, players[number].pos.z);
    players[number].flag = true;

    players[number].oldpos.x = players[number].pos.x;
    players[number].oldpos.y = players[number].pos.y;
    players[number].oldpos.z = players[number].pos.z;
  }

  players[number].pos.x = x;
  players[number].pos.y = y;
  players[number].pos.z = z;
  players[number].rot.x = rotx;
  players[number].rot.y = roty;
}

/* turn off drawing for player number */
void hidePlayer(int number)
{
  if (number >= PLAYER_COUNT) {
    printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
    exit(1);
  }

  players[number].visible = false;
}

/* turn on drawing for player number */
void showPlayer(int number)
{
  if (number >= PLAYER_COUNT) {
    printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
    exit(1);
  }

  players[number].visible = true;
}

/* set all mob location, rotation, and visibility values to zero */
void initMobArray()
{
  for (int i = 0; i < MOB_COUNT; i++) {
    mobs[i].pos.x = 0;
    mobs[i].pos.y = 0;
    mobs[i].pos.z = 0;
    mobs[i].rot.x = 0;
    mobs[i].rot.y = 0;
    mobs[i].rot.z = 0;
    mobs[i].visible = false;
    mobs[i].flag = false;
  }
}

/* create mob with identifier "number" at x,y,z with */
/* heading of rotx, roty, rotz */
void createMob(int number, float x, float y, float z, float rotx, float roty)
{
  if (number >= MOB_COUNT) {
    printf("ERROR: mob number greater than %d\n", MOB_COUNT);
    exit(1);
  }

  mobs[number].pos.x = x;
  mobs[number].pos.y = y;
  mobs[number].pos.z = z;
  mobs[number].rot.x = rotx;
  mobs[number].rot.y = roty;
  mobs[number].visible = true;
}

/* move mob to a new position xyz with rotation rotx,roty,rotz */
void setMobPosition(int number, float x, float y, float z, float rotx, float roty)
{
  if (number >= MOB_COUNT) {
    printf("ERROR: mob number greater than %d\n", MOB_COUNT);
    exit(1);
  }

  mobs[number].pos.x = x;
  mobs[number].pos.y = y;
  mobs[number].pos.z = z;
  mobs[number].rot.x = rotx;
  mobs[number].rot.y = roty;
  mobs[number].flag = true;
}

/* turn off drawing for mob number */
void hideMob(int number)
{
  if (number >= MOB_COUNT) {
    printf("ERROR: mob number greater than %d\n", MOB_COUNT);
    exit(1);
  }

  mobs[number].visible = false;
}

/* turn on drawing for mob number */
void showMob(int number)
{
  if (number >= MOB_COUNT) {
    printf("ERROR: mob number greater than %d\n", MOB_COUNT);
    exit(1);
  }

  mobs[number].visible = true;
}

/* allows user to set position of the light */
void setLightPosition(GLfloat x, GLfloat y, GLfloat z)
{
  lightPosition[0] = x;
  lightPosition[1] = y;
  lightPosition[2] = z;
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

/* returns current position of the light */
GLfloat* getLightPosition()
{
  return (lightPosition);
}

/* functions store and return the current location of the viewpoint */
void getViewPosition(float *x, float *y, float *z)
{
  *x = players[identity].pos.x;
  *y = players[identity].pos.y;
  *z = players[identity].pos.z;
}

void setViewPosition(float x, float y, float z)
{
  players[identity].pos.x = x;
  players[identity].pos.y = y;
  players[identity].pos.z = z;
}

/* returns the previous location of the viewpoint */
void getOldViewPosition(float *x, float *y, float *z)
{
  *x = players[identity].oldpos.x;
  *y = players[identity].oldpos.y;
  *z = players[identity].oldpos.z;
}

void setViewOrientation(float xaxis, float yaxis, float zaxis)
{
  players[identity].rot.x = xaxis;
  players[identity].rot.y = yaxis;
  // ignore zaxis;
}

/* returns the current orientation of the viewpoint */
void getViewOrientation(float *xaxis, float *yaxis, float *zaxis)
{
  *xaxis = players[identity].rot.x;
  *yaxis = players[identity].rot.y;
  *zaxis = 0; // we don't store this or use it
}

/* add the cube at world[x][y][z] to the display list and */
/* increment displayCount */
int addDisplayList(int x, int y, int z)
{
  displayList[displayCount][0] = x;
  displayList[displayCount][1] = y;
  displayList[displayCount][2] = z;
  displayCount++;

  if (displayCount > MAX_DISPLAY_LIST) {
    printf("You have put more items in the display list then there are\n");
    printf("cubes in the world. Set displayCount = 0 at some point.\n");
    exit(1);
  }

  return displayCount;
}

/*  Initialize material property and light source.  */
void init(void)
{
  GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 0.3 };
  GLfloat light_diffuse[] = { 0.3, 0.3, 0.3, 0.7 };
  GLfloat light_specular[] = { 0.2, 0.2, 0.2, 0.8 };
  GLfloat light_full_off[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat light_full_on[] = {1.0, 1.0, 1.0, 1.0};

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  /* if lighting is turned on then use ambient, diffuse and specular
     lights, otherwise use ambient lighting only */
  if (lighting == 1) {
    /* sun light */
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    /* no specular reflection from sun, it is too distracting */
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_full_off);
  } else {
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_full_on);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_full_off);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_full_off);
  }

  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

  /* viewpoint light */
  glLightfv(GL_LIGHT1, GL_POSITION, viewpointLight);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.5);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  glEnable(GL_DEPTH_TEST);
}

/* draw cube in world[i][j][k] */
void drawCube(int i, int j, int k)
{
  GLfloat blue[]  = {0.0, 0.0, 1.0, 1.0};
  GLfloat red[]   = {1.0, 0.0, 0.0, 1.0};
  GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
  GLfloat yellow[]   = {1.0, 1.0, 0.0, 1.0};
  GLfloat purple[]   = {1.0, 0.0, 1.0, 1.0};
  GLfloat orange[]   = {1.0, 0.64, 0.0, 1.0};
  GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat coal[] = {0.15, 0.15, 0.15, 1.0};
  GLfloat stone[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat brown[] = {102.0 / 255, 69.0 / 255, 35.0 / 255, 1.0};

  GLfloat dcoal[] = {0.075, 0.075, 0.075, 1.0};
  GLfloat dstone[] = {0.25, 0.25, 0.25, 1.0};
  GLfloat dbrown[] = {102.0 / 510, 69.0 / 510, 35.0 / 510, 1.0};
  GLfloat dblue[]  = {0.0, 0.0, 0.5, 1.0};
  GLfloat dred[]   = {0.5, 0.0, 0.0, 1.0};
  GLfloat dgreen[] = {0.0, 0.5, 0.0, 1.0};
  GLfloat dyellow[]   = {0.5, 0.5, 0.0, 1.0};
  GLfloat dpurple[]   = {0.5, 0.0, 0.5, 1.0};
  GLfloat dorange[]   = {0.5, 0.32, 0.0, 1.0};

  /* select colour based on value in the world array */
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);

  if (world[i][j][k] == GREEN) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dgreen);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
  } else if (world[i][j][k] == BLUE) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dblue);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
  } else if (world[i][j][k] == RED) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dred);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
  } else if (world[i][j][k] == BLACK) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  } else if (world[i][j][k] == WHITE) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  } else if (world[i][j][k] == PURPLE) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dpurple);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, purple);
  } else if (world[i][j][k] == ORANGE) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dorange);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orange);
  } else if (world[i][j][k] == COAL) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dcoal);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, coal);
  } else if (world[i][j][k] == STONE) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dstone);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, stone);
  } else if (world[i][j][k] == BROWN) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dbrown);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, brown);
  } else if (world[i][j][k] == YELLOW) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, dyellow);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
  } else {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  }

  glPushMatrix();
  /* offset cubes by 0.5 so the centre of the */
  /* cube falls in the centre of the world array */
  glTranslatef(i + 0.5, j + 0.5, k + 0.5);
  glutSolidCube(1.0);

  if (players[identity].highlight.x == i && players[identity].highlight.y == j && players[identity].highlight.z == k) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, coal);
    glLineWidth(2.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glutSolidCube(1.01);
    if (lineDrawing == 1) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }

  glPopMatrix();
}

/* called each time the world is redrawn */
void display(void)
{
  GLfloat skyblue[]  = {0.52, 0.74, 0.84, 1.0};
  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat gray[] = {0.15, 0.15, 0.15, 0.8};
  GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
  int i, j, k;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* position viewpoint based on mouse rotation and keyboard translation */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1, 300.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST);

  glRotatef(players[identity].rot.x, 1.0, 0.0, 0.0);
  glRotatef(players[identity].rot.y, 0.0, 1.0, 0.0);
  /* Subtract 1 to raise viewpoint slightly above objects. */
  /* Gives the impression of a head on top of a body. */
  glTranslatef(-players[identity].pos.x, -players[identity].pos.y - 1.2, -players[identity].pos.z);

  /* set viewpoint light position */
  viewpointLight[0] = players[identity].pos.x;
  viewpointLight[1] = players[identity].pos.y;
  viewpointLight[2] = players[identity].pos.z;
  glLightfv(GL_LIGHT1, GL_POSITION, viewpointLight);

  /* draw surfaces as either smooth or flat shaded */
  if (smoothShading == 1) {
    glShadeModel(GL_SMOOTH);
  } else {
    glShadeModel(GL_FLAT);
  }

  /* draw polygons as either solid or outlines */
  if (lineDrawing == 1) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  /* give all objects the same shininess value and specular colour */
  glMaterialf(GL_FRONT, GL_SHININESS, 90.0);

  /* set starting location of objects */
  glPushMatrix();
  buildDisplayList();

  /* make a blue sky cube */
  glShadeModel(GL_SMOOTH);
  /* turn off all reflection from sky so it is a solid colour */
  glMaterialfv(GL_FRONT, GL_AMBIENT, black);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, skyblue);
  glPushMatrix();
  /* move the sky cube center to middle of world space */
  glTranslatef(50, 25, 50);
  glutSolidCube(150.0);
  glPopMatrix();
  glShadeModel(GL_SMOOTH);
  /* turn off emision lighting, use only for sky */
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, gray);

  /* draw mobs in the world */
  for (i = 0; i < MOB_COUNT; i++) {
    if (mobs[i].visible) {
      //printf("rendering mob %d at %f,%f,%f\n", i, mobs[i].pos.x, mobs[i].pos.y, mobs[i].pos.z);
      glPushMatrix();
      /* black body */
      glTranslatef(
        mobs[i].pos.x,
        mobs[i].pos.y + 0.5,
        mobs[i].pos.z
      );
      glMaterialfv(GL_FRONT, GL_AMBIENT, black);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, gray);
      glutSolidSphere(0.5, 8, 8);
      /* white eyes */
      glRotatef(mobs[i].rot.x, 0.0, 1.0, 0.0);
      glRotatef(mobs[i].rot.y, 1.0, 0.0, 0.0);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
      glTranslatef(0.3, 0.1, 0.3);
      glutSolidSphere(0.1, 4, 4);
      glTranslatef(-0.6, 0.0, 0.0);
      glutSolidSphere(0.1, 4, 4);
      glPopMatrix();
    }
  }

  /* draw players in the world */
  for (i = 0; i < PLAYER_COUNT; i++) {
    if (players[i].visible) {
      glPushMatrix();
      /* white body */
      glTranslatef(
        players[i].pos.x,
        players[i].pos.y + 0.5,
        players[i].pos.z
      );
      glMaterialfv(GL_FRONT, GL_AMBIENT, white);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, gray);
      glutSolidSphere(0.5, 8, 8);
      // mouse left looking left, mouse right looking right
      // mouse up looking up, mouse down looking down
      glRotatef(players[i].rot.x, 0.0, -1.0, 0.0);
      glRotatef(players[i].rot.y, -1.0, 0.0, 0.0);
      /* red eyes */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
      glTranslatef(0.3, 0.1, -0.3);
      glutSolidSphere(0.1, 4, 4);
      glTranslatef(-0.6, 0.0, 0.0);
      glutSolidSphere(0.1, 4, 4);
      glPopMatrix();
    }
  }

  /* draw all cubes in the world array */
  if (displayAllCubes == 1) {
    /* draw all cubes */
    for (i = 0; i < WORLDX; i++) {
      for (j = 0; j < WORLDY; j++) {
        for (k = 0; k < WORLDZ; k++) {
          if (world[i][j][k] != 0) {
            drawCube(i, j, k);
          }
        }
      }
    }
  } else {
    /* draw only the cubes in the displayList */
    /* these should have been selected in the update function */

    for (i = 0; i < displayCount; i++) {
      drawCube(
        displayList[i][0],
        displayList[i][1],
        displayList[i][2]
      );
    }
  }

  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

  // // Set up the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);

  // Set color to white
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

  // Draw a rectangle for the crosshair
  glBegin(GL_QUADS);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 10, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 2);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 + 10, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 2);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 + 10, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 2);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 10, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 2);

  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 2, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 10);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 + 2, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 10);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 + 2, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 10);
  glVertex2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 2, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 10);
  glEnd();

  glEnable(GL_DEPTH_TEST);
  glutSwapBuffers();
}

/* sets viewport information */
void reshape(int w, int h)
{
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 300.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void keyboard_up(unsigned char key, int x, int y)
{
  keyStates[key] = 0;
  // useful for debugging keystates:
  // printf("Key Up %c\n", key);
}

/* respond to keyboard events */
void keyboard(unsigned char key, int x, int y)
{
  keyStates[key] = 1;

  switch (key) {
  case 27:
  case 'q':
    exit(0);
    break;

  case '1':  // draw polygons as outlines
    lineDrawing = 1;
    lighting = 0;
    smoothShading = 0;
    textures = 0;
    init();
    break;

  case '2':  // draw polygons as filled
    lineDrawing = 0;
    lighting = 0;
    smoothShading = 0;
    textures = 0;
    init();
    break;

  case '3':  // diffuse and specular lighting, flat shading
    lineDrawing = 0;
    lighting = 1;
    smoothShading = 0;
    textures = 0;
    init();
    break;

  case '4':  // diffuse and specular lighting, smooth shading
    lineDrawing = 0;
    lighting = 1;
    smoothShading = 1;
    textures = 0;
    init();
    break;

  case '5':  // texture with  smooth shading
    lineDrawing = 0;
    lighting = 1;
    smoothShading = 1;
    textures = 1;
    init();
    break;

  case 'f':  // toggle flying controls
    if (flycontrol == 0) { flycontrol = 1; }
    else { flycontrol = 0; }
    break;

  case ' ':  // toggle digging controls
    dig = 1;
    break;
  }

  glutPostRedisplay();
}

/* load a texture from a file */
/* not currently used */
void loadTexture()
{
  FILE *fp;
  int i, j;
  int red, green, blue;

  if ((fp = fopen("image.txt", "r")) == 0) {
    printf("Error, failed to find the file named image.txt.\n");
    exit(0);
  }

  for (i = 0; i < 64; i++) {
    for (j = 0; j < 64; j++) {
      fscanf(fp, "%d %d %d", &red, &green, &blue);
      Image[i][j][0] = red;
      Image[i][j][1] = green;
      Image[i][j][2] = blue;
      Image[i][j][3] = 255;
    }
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, textureID);
  glBindTexture(GL_TEXTURE_2D, textureID[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, Image);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);

  fclose(fp);
}

void motion(int x, int y)
{
  static float oldx, oldy;
  static int motion_initialized = 0;

  if (!motion_initialized) {
    oldy = y;
    oldx = x;
    motion_initialized = 1;
  }

  // mvx is "rotation about the x axis", i.e. "looking up and down"
  // mvy is "rotation about the y axis", i.e. "looking left/right"
  mvx += (float) y - oldy;
  mvy += (float) x - oldx;
  setViewOrientation(mvx, mvy, 0);
  oldx = x;
  oldy = y;
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  /* capture mouse button events - not currently used */
  /*
    if (button == GLUT_LEFT_BUTTON)
      printf("left button - ");
    else if (button == GLUT_MIDDLE_BUTTON)
      printf("middle button - ");
    else
      printf("right button - ");

    if (state == GLUT_UP)
      printf("up - ");
    else
      printf("down - ");

    printf("%d %d\n", x, y);
  */
}


/* initialize graphics information and mob data structure */
void initializeOpenGL(char *windowTitle)
{
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

  if (fullscreen == 1) {
    glutGameModeString("1024x768:32@75");
    glutEnterGameMode();
  } else {
    glutInitWindowSize(1920, 1280);
    glutCreateWindow(windowTitle);
  }

  init();

  /* not used at the moment */
  //   loadTexture();

  /* attach functions to GL events */
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboard_up);
  glutPassiveMotionFunc(motion);
  glutMouseFunc(mouse);
  glutIdleFunc(update);

  printf("gl version: %s\n", glGetString(GL_VERSION));

  /* starts the graphics processing loop */
  glutMainLoop();
}


