#ifndef _NEW_WORLD
#define _NEW_WORLD

double bounds(GameContext *gc, double number, double low, double high);
void fill(GameContext *gc, int low, int high, int type);
void perlin(GameContext *gc, int low, int high, int type);
int build_world(GameContext *gc);
int build_little_world(GameContext *gc);
void perlin_clouds(GameContext *gc, double level, double scale, double deg);
#endif
