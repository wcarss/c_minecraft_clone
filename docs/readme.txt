
Additions and Changes for Assignment 3
--------------------------------------

Mob Controls
------------
The following functions have been added to control the creation and
movement of the mobs:

   void createMob(int number, float x, float y, float z, float roty);
        -creates mob number at position x,y,z with rotation y
   void setMobPosition(int number, float x, float y, float z, float roty);
        -move a created mob to a new position and rotation
   void hideMob(int number);
        -stops drawing mob number, it become invisible
	-making mobs invisible is equivalent to removing them from the world
   void showmob(int number);
        -start drawing mob number, make it visible again

In all of the above functions:
number  -is the identifier for each mob. There can be a maximum of 10
         mobs in the game. They are numbered from 0 to 9 and this number
         is passed to all functions to indicate which mob you are updating.
x,y,z   -are the x,y,z coordinates in the world space. They are floats.
         These are world coordinates.
roty    -is the rotation of the mob around the y axis. This allows you
         to position the mob so it is facing in the direction it is
         moving or looking. It is a float.

A small sample of the mob control is included in a3.c. You can remove
this and replace it with your own code. To see this demo you need to
run the sample world using:
	./a3 -testworld -drawall
You can move around in the test world after pressing he f key.


Mouse Orientation
-----------------
This was added in assignment 2 but may be useful now.

void getViewOrientation(float *xaxis, float *yaxis, float *zaxis);
-Returns the direction the mouse is pointing.
-The xaxis and yaxis values are the amount of rotation around the
 x and y axis respectively.
-The zaxis value will always be zero.
-The values can be larger then 360 degrees which indicates more than
 one rotation. You can return them to the range of 0 to 360 using fmodf().

Dig Flag
--------
A flag has been created which will be set to 1 when the user presses
the space bar. This is used to indicate that the user wants to
remove the block which they are facing. You need to reset the flag to 0
once you have used it. The flag is:
	int dig;
A message is printed when the space bar is pressed. This message is
in a3.c and can be removed in your assignment.

Lighting
--------
Lighting has been modified to display cubes more clearly. There is now a
light which moves with the viewpoint and it should show the local objects
more clearly.

More Colours
------------
The colours for cubes has been explanded.
	0 empty
	1 green
	2 blue
	3 red
	4 black
	5 white
	6 purple
	7 orange
	8 yellow

Frames Per Second (FPS) Printing
--------------------------------
The FPS are no longer printed automatically. There is a -fps command
line flag which turns this functionality one.




========================================
Additions and Changes for Assignment 2
--------------------------------------

Important Note
--------------
Do not remove or modify the code which prints out the frames per second (FPS)
in a2.c.  This will be used to test the framerates achieved from culling.

Do not remove or modify the code which builds the sample world in a2.c
in the main() when testworld == 1. This is used for testing the culling code.

Don't change the starting location of the viewpoint.


World Notes
-----------
-The cubes measure one unit along each axis.
-Cubes are positioned so their centre is at 0.5 greater than their
 x,y,z coordinates. So the cube at 0,0,0 is centred at 0.5, 0.5, 0.5. 
-Added a frames per second routine in update.
-You may see the edges of the screen don't update quickly when the viewpoint
 moves quickly. It looks like the edge of the world stops and there is a
 blocky edge visible. This isn't something you need to fix. 

Command Line Arguments
----------------------
-The -help command line argument will print usage information for a2.
-The -drawall command line argument will force the system to draw all
 of the cubes in the world. It ignores the display list and draws everything
 in the world array. This will be slow.
-The -full command will open the display in full screen.
-The -testworld command will build a very large sample world. It will be
 slow if there is no culling implemented. The lower half of the world will be
 filled with cubes when this option is chosen.

Mouse Control
-------------
Motion in the y-axis (up and down) has been disabled. This is necessary
for gravity to operate correctly.  Without this it would be possible
to look up and move in that direction (and defy gravity). 

You can recreate the flying behaviour by typing f on the keyboard.
This toggles the flying control on and off.  It will let you move in any
direction which the mouse is pointing and could be useful for testing
before gravity is implemented. If you want the f key to work
correctly with the gravity code which you write them you will have
to disable gravity when the integer flag flycontrol is set equal to 1.

If you are stuck and cannot move then try pressing f. It should let you move
in any direction.

New Parameter Manipulation Functions
------------------------------------
void getViewPosition(float *x, float *y, float *z);
-Returns the position where the viewpoint will move to on the next step.
-Returns negative numbers which you may need to make positive for some
 calculations such as using them as an index into the world array.

void setViewPosition(float x, float y, float z);
-Sets the position where the viewpoint will move to on the next step.
-Numbers taken from the world array need to be made negative before they
 are used with setViewPosition.

void getOldViewPosition(float *x, float *y, float *z);
-Returns the position where the viewpoint is currently.
-Returns negative numbers which you may need to make positive for some
 calculations such as using them as an index into the world array.

void getViewOrientation(float *xaxis, float *yaxis, float *zaxis); 
-Returns the direction the mouse is pointing. 
-The xaxis and yaxis values are the amount of rotation around the
 x and y axis respectively.
-The zaxis value will always be zero.
-The values can be larger then 360 degrees which indicates more than
 one rotation.


Display Lists
--------------
An array named displayList has been created which you put the cube indices
that you want to be drawn. The function addDisplayList() is used to
add cubes to the list.
        eg. The following would set the cube at world[1][3][5] to be drawn.
            addDisplayList(1,3,5);
This is used so then entire world is not drawn with each frame.
Only the cubes which you determine are visible should be added
to the display list.

Add the cubes you derive from visibility testing to the list.
There is also a counter named displayCount which contains the
number of elements in displayList[][].  You do not need to increment
displayCount but you need to set it equal to zero when you build a new
display list.  You need to build a new displayList each time you
perform culling (each time buildDisplayList() is called).


New Empty Functions
-------------------
void collisionResponse()
-This is where you will write the collision detection and response code
 in the a2.c file.

void buildDisplayList()
-This is where you perform culling and add visible cubes to the display
 list.  There is some sample code here which moves all of the cubes in
 the world to the display list. This duplicates the original behaviour of
 assignment 1.  This should be replaced with your visibility/culling code.


Culling Information
-------------------
-The web page at:
	http://www.crownandcutlass.com/features/technicaldetails/frustum.html
contains a good explanation of how to determine the viewing frustum for
a viewpoint in OpenGL. There is also some useful code there. 


========================================
Readme from Assignment 1
------------------------
Building and Running the Graphics System
----------------------------------------
The program consists of two .c files.  The a1.c file contains the main()
routine and the update() function. All of the changes necessary for the
assignment can be made to this file.  The graphics.c file contains all
of the code to create the 3D graphics for the assignment. You should not
need to change this code for assignment 1.

There is a makefile which will compile the code on the Macs.
The executable is named a1. If the program is run with the -full
command line option then it will run in fullscreen.

When the program runs the view is controlled through the mouse and
keyboard. The mouse moves the viewpoint left-right and up-down.
The keyboard controls do the following:
	w  move forward
	a  strafe left
	s  move forward
	d  strafe right
	q  quit

The 1,2,3 buttons also change the rendering options.
	
Note: If the controls appear to be reversed then the viewpoint is upside down.
Pull or push the mouse until you turn over.

There are a few sample boxes drawn in the middle of the world and a
set of boxes which show the outer width and depth of the world.
These are defined in a1.c and should be removed before your
assignment is submitted.


Programming Interface to the Graphics System
--------------------------------------------

1. Drawing the world
--------------------

The only shape drawn by the graphics system is the cube. The data
structure which holds all of the objects is the three dimensional array:

	GLubyte world[100][50][100]

The GLubyte is an unsigned byte defined by OpenGL. It is the same as
any other unsigned byte.

The indices of the array correspond to the dimensions of the world.
In order from left to right they are x,y,z.  This means the world is 100 units
in the x dimension (left to right), 50 units in the y dimension (up and down),
and 100 units in z (back to front).

The cube at location world[0][0][0] is in the lower corner of the 3D world.
The cube at location world[99][49][99] is diagonally across from
world[0][0][0] in the upper corner of the world.

Each cube drawn in the world is one unit length in each dimension.

Values are stored in the array to indicate if that position in the
world is occupied or are empty. The following would mean that
position 25,25,25 is empty:
	world[25][25][25] = 0

If the following were used:
	world[25][25][25] = 1
then position 25,25,25 would contain a green cube. 

Cubes can be drawn in different colours depending on that value stored
in the world array. The current colours which can be drawn are:
	0 empty
	1 green
	2 blue
	3 red
	4 black
	5 white

2. Setting the Light Position
-----------------------------
There is a single light in the world.  The position of the light
is controlled through two functions:

	void setLightPosition(GLfloat x, GLfloat y, GLfloat z);
	GLfloat* getLightPosition();

The setLightPosition() function moves the light source to location x,y,z in the
world. The getLightPosition() function returns a pointer to an array
of floats containing current the x,y,z location of the light source.

To see the effect of a change through setLightPosition() you will
need to call glutPostRedisplay() to update the screen. 
 

3. Timing Events
----------------
OpenGL is event driven. The events which this program will respond to 
include keyboard and mouse input. The glutMainLoop() function receives
these inputs and processes them. 

The glutMainLoop() function will loop until the program ends. This means
that all of your code to initialize the world must be run before this
function is called. It also means that changes to the world must occur
inside function called by OpenGL. The only function which you have
access to to make these updates is named update() in a1.c.

When it is not otherwise drawing the scene the system will call the
update() function. This is where you can make changes to the world
array and lighting while program is running.

If you make changes to the world or the light in the udpate()
function then you should call glutPostRedisplay() to refresh the screen.

The update() function is not called on a predictable schedule. You will
need to check the time during updates to control the rate at which
the world changes. 



Changing graphics.c
-------------------
You can make changes to graphics.c if you wish but you are responsible
for making them work. If you break the graphics system then you have
to fix it yourself.


