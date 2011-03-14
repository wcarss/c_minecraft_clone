#include "perlin.h"
#include "visibility.h"
#include "engine.h"
#include "world.h"

/* Takes a number and truncs it into the bounds provided. */
double bounds(double number, double low, double high)
{
  if(number > high) number = high;
  if(number < low) number = low;
  return number;
}

void fill(int low, int high, int type)
{
	int i, j, k;

	for(i = 0; i < WORLDX; i++)
		for(j = low; j <= high; j++)
			for(k = 0; k < WORLDZ; k++)
				world[i][j][k] = type;
}

void perlin(int low, int high, int type)
{
  int x, y, z, i;
  int level = (high + low)/2;
  int scale = level - low;

  for(x = 0; x <= WORLDX; x++)
  {
    for(z = 0; z < WORLDZ; z++)
    {
      /* Give it small x/z values to increase smoothness; 2 octaves is enough */
      y = level + (scale * PerlinNoise2D(x/47.0, z/47.0, 2, 2, 2));

      /* Have to keep the result safely inside the boundaries of the earth. */		
      y = bounds(y, 0, WORLDY-1); 

      for(i = y; world[x][i][z] == EMPTY; i--)
      {
        world[x][i][z] = type;
      }
    }
  }
}

void cover(int type)
{
  int i, j, k;

  for(i = 0; i < WORLDX; i++)
    for(k = 0; k < WORLDZ; k++)
      for(j = WORLDY-1; j > 0; j--)
        if(world[i][j][k] != EMPTY)
        {
          world[i][j+1][k] = type;
          break;
        }
}

void fill_lakes(int level)
{
  int i, j, k;

  for(i = 0; i < WORLDX; i++)
    for(k = 0; k < WORLDZ; k++)
      for(j = WORLDY-1; j > 0; j--)
      {
        if(j <= level && world[i][j][k] == EMPTY)
        {
          world[i][j][k] = BLUE;
        }
        else if(world[i][j][k] != EMPTY)
        {
          break;
        }
     }
}

void cut(int x, int z, int level)
{
  int i;

  for(i = level; i <WORLDY; i++)
    world[x][i][z] = EMPTY;
}

void mark(int x, int z, int type)
{
  int i;

  for(i = WORLDY-1; i > 0; i--)
  {
    if(world[x][i][z] != EMPTY)
    {
      world[x][i+1][z] = type;
      break;
    }
  }
}

void maze(int low, int high)
{
  int x, y, z, lines, num, dimension_increment;

  for(x = 1; x < WORLDX-1; x++)
    for(y = low; y <= high; y++)
      for(z = 1; z < WORLDZ-1; z++)
        if(0 < PerlinNoise3D(x/12.0, y/12.0, z/12.0, 2.1, 1.9, 2))
        {
           if(rand() % 3 > 1)
             world[x][y][z] = STONE;
           else
             world[x][y][z] = DIRT;
        }

  for(dimension_increment = 0; dimension_increment <= 1; dimension_increment++)
  {
    for(x = 0; x < WORLDX; x++)
      for(y = low; y <= high; y++)
      {
        if(dimension_increment == 0)
        {
          world[x][y][0] = STONE;
          world[x][y][WORLDZ-1] = STONE;
        }
        else
        {
          world[0][y][x] = STONE;
          world[WORLDX-1][y][x] = STONE;
        }
      }


    lines = rand() % 15;
    for(num = 0; num < lines; num++)
    {
      if(num == 0) x = 45;
      else x = rand() % (WORLDX-1);
      for(z = 1; z < WORLDZ-1; z++)
      {
        for(y = low; y <= high; y++)
        {
          if(dimension_increment == 0)
          {
            world[x][y][z] = EMPTY;
          } 
          else if(dimension_increment == 1)
          {
            world[z][y][x] = EMPTY;
          }
        }
      }
    }
  }
}

/* Uses Ken Perlin's PerlinNoise2D to generate smooth, interesting terrain.
   Generates terrain across the world, at y=level +/- scale 
   Code found at http://local.wasp.uwa.edu.au/~pbourke/texture_colour/perlin */
/*void perlin_terrain(double level, double scale)
{
  int x, y, z, block;
  int result;
  float rnd; 
  srand(time(NULL));
  rnd = ((rand() % 200 ) / 200.0) * 20;
  printf("rnd is %f\n", rnd);
*/
/*  if(level > 15)
  {
    for(x = 0; x < WORLDX; x++)
      for(z = 0; z < WORLDZ; z++)
      {
        result = 4 + (2 * PerlinNoise2D(x/12.0, z/12.0, 2.1, 1.9, 2));
        world[x][result][z] = BROWN;
      }

    for(x = 0; x < WORLDX; x++)
      for(z = 0; z < WORLDZ; z++)
      {
        result = 20 + (2 * PerlinNoise2D(x/12.0, z/12.0, 2.1, 1.9, 2));
        world[x][result][z] = BROWN;
      }
*/
 /*   for(x = 0; x < WORLDX; x++)
      for(y = 0; y < level-scale; y++)
        for(z = 0; z < WORLDZ; z++)
          {
            if( x % 2 == 0 && y % 2 == 0)
              block = BROWN;
            else if( x % 2 == 1 && y % 2 == 0)
              block = GRAY;
            else if( x % 2 == 0 && y % 2 == 1)
              block = GRAY;
            else if( x % 2 == 1 && y % 2 == 1)
              block = BROWN;
            
            world[x][y][z] = block;
          }

  bore(WORLDX/2, 5, WORLDZ/2);

  for(x = 0; x < WORLDX; x++)
  {
    for(z = 0; z < WORLDZ; z++)
    {
   */   /* Give it small x/z values to increase smoothness; 2 octaves is enough */
     /* result = level + (scale * PerlinNoise2D(x/37.0, z/37.0, 2, 2, 2));
*/
      /* Have to keep the result safely inside the boundaries of the earth. */		
  //    result = bounds(result, 0, WORLDY-1);
      /* This will draw water. Starting at the result and working up to just below
	 the average terrain level, pits will be filled with blue blocks. */
    /*  while(result < level - 2)
      {
	world[x][result-1][z] = BLUE;
	result++;
      }
*/
      /* Draw green everywhere I didn't draw blue. */
  /*    if(world[x][result-2][z] != BLUE)
	world[x][result-2][z] = GREEN;

      if(level > 15)
      {
	if(x == 45 && z == 50)
	{
	  for(y = level+scale+5; y >= 4; y--)
	  {
	    world[x][y][z] = EMPTY;

	    if(world[x+1][y-1][z] == GREEN)
	      world[x+1][y][z] = RED;
	    if(world[x-1][y-1][z] == GREEN)
	      world[x-1][y][z] = RED;
	    if(world[x][y-1][z+1] == GREEN)
	      world[x][y][z+1] = RED;
	  }
	}
      }
    */  /* I've decided to stop doing things this way, but I'm keeping the code in case I go back. */
      /* This will clear out the green blocks above - we're starting with a box full
	 of green and cutting out of it, to make sure there are no terrain holes. */
      /*while(world[x][result][z] != 0)
	{
	  world[x][result][z] = 0;
	  result++;
	}*/
//    }
//  }
//}

/* Makes perlin-noise clouds! Uses Ken Perlin's PerlinNoise3D function, taken from
http://local.wasp.uwa.edu.au/~pbourke/texture_colour/perlin */
/*void perlin_clouds(double level, double scale, double deg)
{
  int x, z;
  GLfloat *light = getLightPosition();

  level = bounds(level, 0, WORLDY-1);
  for(x = 0; x < WORLDX; x++)
  {
    for(z = 0; z < WORLDZ; z++)
    {
  */    /* Overwrite whatever was here before, as long as it isn't the sun. */
    //  if(!((float)x == light[0] && level == light[1] && (float)z == light[2]))
//	world[x][(int)level][z] = EMPTY;

      /* Using deg (from update) as the y-axis, step through slices of
	 smooth volumetric noise to get realistic looking clouds. */
  /*    if((PerlinNoise3D(x/76.0, z/76.0, deg/108.0, 1.99, 2.1, 4) * scale) > 0)
	world[x][(int)level][z] = WHITE;
    }
  }
}
*/
int build_world()
{
//  fill(0,5,BLACK);
//  fill(8,8,COAL);
//  fill(11,11,COAL);
//  fill(14,14,STONE);
  fill(17, 17,STONE);
  fill(21, 21,STONE);
  perlin(21, 30,STONE);
  perlin(25, 75,DIRT);
  cover(GREEN);
  fill_lakes(40);
  mark(45, 44,RED);
  mark(44, 45,RED); 
  mark(46, 45,RED);
  mark(45, 46,RED);
  maze(18, 20);
  cut(45, 45, 18);  
  return 0;
}
