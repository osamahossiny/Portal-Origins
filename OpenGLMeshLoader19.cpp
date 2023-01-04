#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <iostream>
#include <string>
#include<string.h>
#include <stdlib.h>
#include <time.h>

#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <playsoundapi.h>

#include<chrono>
#include<thread>
#include <stdio.h>
#include <random>
#include <vector>
#include <tuple>
using namespace std;


#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)


void setupLights();
void setupCamera();
void drawPlayer(double x, double y, double z, double rotation);
void drawWall(double x, double y, double z, double rotation);
void setUpSecondLevel();
void setUpFirstLevel();


//================================================================================================//

int WIDTH = 1920;
int HEIGHT = 1080;
bool level1Passed = false;
bool level1Elevator = false;
bool elv1 = false;
bool doorOpen = false;

int preX = 1535 / 2, preY = 863 / 2;
bool gameReady = false;
int level = 1;
GLuint tex;
char title[] = "3D Model Loader Sample";
int coins = 0;
bool hasRed = false;
bool hasBlue = false;
bool win = false;
// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 200;


// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_gun;
Model_3DS model_display;
Model_3DS model_terror;
Model_3DS model_1;
Model_3DS model_2;
Model_3DS model_3;
Model_3DS model_4;
Model_3DS model_vending;
Model_3DS model_goldCoin;
Model_3DS model_silverCoin;
Model_3DS model_key;



// Textures
GLTexture tex_ground;
GLTexture wallTex;
GLTexture boarderWall;
GLTexture doorTex;
GLTexture goldTex;
GLTexture silverTex;
GLTexture elevatorTex;
GLTexture slider;
GLTexture red;
GLTexture portalTex;






//================================================================================================//
void drawCylinder(GLdouble r, GLdouble h) {
	glPushMatrix();
	glRotated(-90, 1, 0, 0);
	GLUquadricObj* quad;
	quad = gluNewQuadric();
	gluCylinder(quad, r / 5.0, r / 5.0, h, 10, 10);
	glPopMatrix();
}


class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
	void operator /=(float value)
	{
		x /= value;
		y /= value;
		z /= value;
	}
	void operator -=(Vector v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}
	Vector operator -(Vector v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	Vector operator +(Vector v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	void operator +=(Vector v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	Vector operator*(float value) {
		return Vector(x * value, y * value, z * value);
	}
	Vector operator/(float value) {
		return Vector(x / value, y / value, z / value);
	}
	Vector operator *(Vector v) {//cross multiblication
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	string toString() {
		return to_string(x) + " " + to_string(y) + " " + to_string(z);
	}

};
Vector unitVector(Vector b) {
	Vector unitVector = b / (sqrt(b.x * b.x + b.y * b.y + b.z * b.z));
	return unitVector;
}
double magnitude(Vector v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
double dotProduct(Vector a, Vector b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
double rotation_angle(Vector a, Vector b) {
	// the angle between two vectors is to get the cos inverse of the dot product diviede by the magnitudes of the vectors 
	return DEG2RAD(acos(dotProduct(a, b) / (magnitude(a) * magnitude(b))));
}

GLdouble square(GLdouble num) {
	return num * num;
}
GLdouble getDistanceFromCenter(GLdouble x, GLdouble y, GLdouble z, Vector v2) {
	return (square(x - v2.x) + square(y - v2.y) + square(z - v2.z));
}
vector< Vector> portals;


class Door {
public:
	Vector pos;
	GLdouble l = 2, h = 6, w = 5;
	Door() {};
	Door(Vector _pos) {
		pos = _pos;
	}
	void draw() {
		glPushMatrix();

		glDisable(GL_LIGHTING);

		glColor3f(1, 1, 1);
		glTranslated(pos.x, pos.y, pos.z);

		glEnable(GL_TEXTURE_2D);


		glBindTexture(GL_TEXTURE_2D, doorTex.texture[0]);

		glPushMatrix();

		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(1, 0, 0);
		glTexCoord2f(0, 0);
		glVertex3f(-l / 2, 0, w / 2);
		glTexCoord2f(1, 0);
		glVertex3f(-l / 2, h, w / 2);
		glTexCoord2f(1, 1);
		glVertex3f(-l / 2, h, -w / 2);
		glTexCoord2f(0, 1);
		glVertex3f(-l / 2, 0, -w / 2);
		glEnd();
		glPopMatrix();




		glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.
		glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
		glDisable(GL_TEXTURE_2D);

		glPopMatrix();
	}

};

Door door = Door(Vector(-19, 0, 0));

void doorSlideUp(int x) {
	if (door.pos.y < 6) {
		door.pos.y += .1;
		glutTimerFunc(50, doorSlideUp, x);
	}
	else {
		if (x == 1) {
			doorOpen = true;
		}
	}
}
void doorSlideDown(int x) {
	if (door.pos.y > 0) {
		door.pos.y -= .1;
		glutTimerFunc(50, doorSlideDown, x);
	}
	else if (x == 1) {
		level = 2;
		setUpSecondLevel();
	}
}


class Portal {
public:
	Vector pos = Vector(0, 0, 0);
	GLdouble  r = 10, h = .5;

	Portal() {};
	Portal(Vector _pos) {
		pos = _pos;
		pos.y = 4;
	}
	void draw() {
		/*glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		drawCylinder(r, h);
		glPopMatrix();*/
		glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

		glDisable(GL_LIGHTING);	// Disable lighting 
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);

		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glRotated(-90, 1, 0, 0);
		glBindTexture(GL_TEXTURE_2D, portalTex.texture[0]);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluCylinder(qobj,2,2,.5, 20,20 );
		gluDeleteQuadric(qobj);

		glPopMatrix();

		glEnable(GL_TEXTURE_2D);	// Enable 2D texturing//sky box
	}
	void update(GLdouble _x, GLdouble _y, GLdouble _z) {
		pos.x = _x;
		pos.y = 4;
		pos.z = _z;
	}
};



class Camera {
public:
	Vector Eye, At, Up;

	Camera(float EyeX = 7.6f, float EyeY = 2.35f, float EyeZ = 0.0f, float AtX = 0.0f, float AtY = 0.0f, float AtZ = 0.0f, float UpX = 0.0f, float UpY = 1.0f, float UpZ = 0.0f) {
		Eye = Vector(EyeX, EyeY, EyeZ);
		At = Vector(AtX, AtY, AtZ);
		Up = Vector(UpX, UpY, UpZ);
	}

	void moveX(float d) {
		Vector right = unitVector(Up * (At - Eye));
		right.y = 0;
		Eye = Eye + right * d;
		At = At + right * d;
	}

	void moveY(float d) {
		//Eye = Eye + unitVector(Up) * d;
		//At = At + unitVector(Up) * d;
		/*At.y =  min(At.y, 3);*/
		At.y += d;
		if (At.y > 5) At.y = 5;
		else if (At.y < -2) At.y = -2;


	}

	void moveZ(float d) {
		Vector view = unitVector(At - Eye);
		view.y = 0;
		Eye = Eye + view * d;
		At = At + view * d;
	}

	void rotateX(float a) {
		Vector view = unitVector(At - Eye);
		Vector right = unitVector(Up * (view));
		view = view * cos(DEG2RAD(a)) + Up * sin(DEG2RAD(a));
		Up = view * (right);
		At = Eye + view;
	}

	void rotateY(float a) {
		/*Vector temp1 = Eye + Vector(0, 0, 0);
		Vector temp2 = At + Vector(0, 0, 0);
		temp1.y = 0;
		temp2.y = 0;
		GLdouble atY = At.y;
		Vector view = unitVector(temp2 - temp1);
		Vector right = unitVector(Up * (view));
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view * (Up);
		At = Eye + view;
		At.y = atY;*/

		At = Eye + unitVector(At - Eye) * cos(DEG2RAD(a)) + unitVector(Up * (unitVector(At - Eye))) * sin(DEG2RAD(a));


	}

	void look() {
		gluLookAt(
			Eye.x, Eye.y, Eye.z,
			At.x, At.y, At.z,
			Up.x, Up.y, Up.z
		);
	}

};

Camera cam;

class Wall
{
public:
	GLdouble x, y, z, l, h, w;
	GLTexture wallTex;

	Wall(GLdouble _x, GLdouble _y, GLdouble _z, GLdouble _l, GLdouble _h, GLdouble _w, GLTexture _wallTex)
	{
		x = _x; // x coord of the center point
		y = _y; // y coord of the center point
		z = _z;	// z coord of the center point
		l = _l; // length of the wall
		h = _h; // height of the wall
		w = _w; // width of the wall
		wallTex = _wallTex;

	}
	void draw() {
		glPushMatrix();

		glDisable(GL_LIGHTING);

		glColor3f(1, 1, 1);
		glTranslated(x, 0, z);

		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, wallTex.texture[0]);

		glPushMatrix();
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, 0);
		glVertex3f(l / 2, 0, -w / 2);
		glTexCoord2f(1, 0);
		glVertex3f(l / 2, h, -w / 2);
		glTexCoord2f(1, 1);
		glVertex3f(-l / 2, h, -w / 2);
		glTexCoord2f(0, 1);
		glVertex3f(-l / 2, 0, -w / 2);
		glEnd();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, wallTex.texture[0]);

		glPushMatrix();
		glColor3f(.8, .5, .2);

		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, 0);
		glVertex3f(l / 2, 0, w / 2);
		glTexCoord2f(1, 0);
		glVertex3f(l / 2, h, w / 2);
		glTexCoord2f(1, 1);
		glVertex3f(-l / 2, h, w / 2);
		glTexCoord2f(0, 1);
		glVertex3f(-l / 2, 0, w / 2);
		glEnd();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, wallTex.texture[0]);

		glPushMatrix();

		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(1, 0, 0);
		glTexCoord2f(0, 0);
		glVertex3f(-l / 2, 0, w / 2);
		glTexCoord2f(1, 0);
		glVertex3f(-l / 2, h, w / 2);
		glTexCoord2f(1, 1);
		glVertex3f(-l / 2, h, -w / 2);
		glTexCoord2f(0, 1);
		glVertex3f(-l / 2, 0, -w / 2);
		glEnd();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, wallTex.texture[0]);

		glPushMatrix();

		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(1, 0, 0);
		glTexCoord2f(0, 0);
		glVertex3f(l / 2, 0, w / 2);
		glTexCoord2f(1, 0);
		glVertex3f(l / 2, h, w / 2);
		glTexCoord2f(1, 1);
		glVertex3f(l / 2, h, -w / 2);
		glTexCoord2f(0, 1);
		glVertex3f(l / 2, 0, -w / 2);
		glEnd();
		glPopMatrix();

		glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.
		glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
		glDisable(GL_TEXTURE_2D);

		glPopMatrix();
	}


};
vector< Wall > firstLevelWalls;

class Player
{
public:
	GLdouble x, y, z, r;
	GLdouble speed = 0.2;
	Camera camera;
	bool left = false, right = false, front = false, back = false, isFirstPerson = true;
	Portal portal1, portal2 ;

	//tuple<Shot, Shot> shots;
	Player(GLdouble _x, GLdouble _y, GLdouble _z, GLdouble _r) {
		x = _x; // x coord of the center point
		y = _y; // y coord of the center point
		z = _z;	// z coord of the center point
		r = _r;	// rotation angle
	}

	void rotateY(double m) {
		if (isFirstPerson) {
			camera.Eye.x = x - .3 * cos(DEG2RAD(-r));
			camera.Eye.z = z - .3 * sin(DEG2RAD(-r));
			camera.Eye.y = 3;
			camera.At.x = x - 8 * cos(DEG2RAD(-r));
			camera.At.z = z - 8 * sin(DEG2RAD(-r));

		}
		else {
			camera.Eye.x = x + 5 * cos(DEG2RAD(-r));
			camera.Eye.z = z + 5 * sin(DEG2RAD(-r));
			camera.Eye.y = 5;
			camera.At.x = x - 8 * cos(DEG2RAD(-r));
			camera.At.z = z - 8 * sin(DEG2RAD(-r));
		}
		r += m;
		r = (r / 360 - (int)r / 360) * 360;
	}
	void moveY(double m) {
		camera.moveY(m);
	}

	void draw() {
		glPushMatrix();
		glTranslated(x, 0, z);
		glRotated(r, 0, 1, 0);

		glPushMatrix();
		glRotated(-90, 0, 1, 0);
		glScaled(.2, .2, .2);
		model_1.Draw();


		glPopMatrix();
		glEnable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslated(-1, 2, -.4);
		if (!isFirstPerson) glTranslated(.5, -.2, -.3);
		glRotated(55, 0, 1, 0);
		glScaled(.5, .5, .5);
		model_gun.Draw();
		glPopMatrix();
		glPopMatrix();
		portal1.draw();
		portal2.draw();

	}
	void firstPerson() {
		isFirstPerson = true;

		camera.Up.x = 0;
		camera.Up.y = 1;
		camera.Up.z = 0;

		camera.Eye.x = x - .3 * cos(DEG2RAD(-r));
		camera.Eye.z = z - .3 * sin(DEG2RAD(-r));
		camera.Eye.y = 3;
		camera.At.x = x - 8 * cos(DEG2RAD(-r));
		camera.At.z = z - 8 * sin(DEG2RAD(-r));

	}
	void thirdPerson() {

		isFirstPerson = false;

		camera.Up.x = 0;
		camera.Up.y = 1;
		camera.Up.z = 0;

		camera.Eye.x = x + 5 * cos(DEG2RAD(-r));
		camera.Eye.z = z + 5 * sin(DEG2RAD(-r));
		camera.Eye.y = 5;
		camera.At.x = x - 8 * cos(DEG2RAD(-r));
		camera.At.z = z - 8 * sin(DEG2RAD(-r));
	}
	int isInWall(Wall wall) {
		GLdouble radius = 1;
		GLdouble l; GLdouble w;
		l = wall.l;// x
		w = wall.w;// z

		if (abs(wall.x - x) <= radius + l / 2.0 && abs(wall.z - z) <= radius + w / 2.0) {
			GLdouble dirX = (wall.x - x);
			GLdouble dirZ = (wall.z - z);
			if (wall.x + l / 2.0 > x && x > wall.x - l / 2.0) {
				if (dirZ >= 0)
					return 4;
				return 3;
			}
			if (wall.z + w / 2.0 > z && z > wall.z - w / 2.0)
			{
				if (dirX >= 0)
					return 2;
				return 1;
			}
		}

		return 0;
	}
	void update() {

		GLdouble dir = 0;
		for (int i = 0; i < firstLevelWalls.size(); i++) {
			Wall cur = firstLevelWalls[i];
			if (isInWall(cur))
				dir = isInWall(cur);
			cout << isInWall(cur) << endl;
			Vector look = camera.At - camera.Eye;
			cout << look.toString() << endl;

		}
		Vector look = camera.At - camera.Eye;
		if (left) {
			if (look.z >= 0 && dir == 2)
				return;
			if (look.z < 0 && dir == 1)
				return;
			if (look.x >= 0 && dir == 3)
				return;
			if (look.x < 0 && dir == 4)
				return;
			Vector vec = Vector(0, 0, 0);
			vec.x = camera.Eye.x;
			vec.y = camera.Eye.y;
			vec.z = camera.Eye.z;
			camera.moveX(speed);
			vec = camera.Eye - vec;
			x += vec.x;
			z += vec.z;
		}
		if (right) {
			if (look.z >= 0 && dir == 1)
				return;
			if (look.z < 0 && dir == 2)
				return;
			if (look.x >= 0 && dir == 4)
				return;
			if (look.x < 0 && dir == 3)
				return;
			Vector vec = Vector(0, 0, 0);
			vec.x = camera.Eye.x;
			vec.y = camera.Eye.y;
			vec.z = camera.Eye.z;
			camera.moveX(-speed);
			vec = camera.Eye - vec;
			x += vec.x;
			z += vec.z;
		}
		if (front) {
			if (look.x >= 0 && dir == 2)
				return;
			if (look.x < 0 && dir == 1)
				return;
			if (look.z >= 0 && dir == 4)
				return;
			if (look.z < 0 && dir == 3)
				return;
			//z -= speed;
			Vector vec = Vector(0, 0, 0);
			vec.x = camera.Eye.x;
			vec.y = camera.Eye.y;
			vec.z = camera.Eye.z;
			camera.moveZ(speed);
			vec = camera.Eye - vec;
			x += vec.x;
			z += vec.z;

		}
		if (back) {
			if (look.x >= 0 && dir == 1)
				return;
			if (look.x < 0 && dir == 2)
				return;
			if (look.z >= 0 && dir == 3)
				return;
			if (look.z < 0 && dir == 4)
				return;
			//z += speed;
			Vector vec = Vector(0, 0, 0);
			vec.x = camera.Eye.x;
			vec.y = camera.Eye.y;
			vec.z = camera.Eye.z;
			camera.moveZ(-speed);
			vec = camera.Eye - vec;
			x += vec.x;
			z += vec.z;
		}

		if (x > 19) { 
			x = 19; 
			if (isFirstPerson) firstPerson();
			else thirdPerson();
			
		}

		if (z > 19) {
			z = 19;
			if (isFirstPerson) firstPerson();
			else thirdPerson();

		}

		if (x < -19 && !level1Passed) {
			x = -19;
			if (isFirstPerson) firstPerson();
			else thirdPerson();

		}

		if (z < -19) {
			z = -19;
			if (isFirstPerson) firstPerson();
			else thirdPerson();

		}
		if (level1Elevator) {
			if (x < -24) x = -24;
			if (x > -21 && !doorOpen) x = -21;
			if (z < -2) z = 2;
			if (z > 2) z = 2;
			if (isFirstPerson) firstPerson();
			else thirdPerson();
		}

		if (level1Passed) {
			if (x < -20 && !level1Elevator) {
				if (z > -2.5 && z < 2.5) {
					level1Elevator = true;
					glutTimerFunc(10000, doorSlideUp, 1);
					glutTimerFunc(0, doorSlideDown, 1);
				}
				else {
					x = -20;
					if (isFirstPerson) firstPerson();
					else thirdPerson();
				}
			}
			
		}
		if (doorOpen && x > -20) {
			glutTimerFunc(1000, doorSlideDown, 0);
			level1Elevator = false;
			level1Passed = false;
			doorOpen = false;
		}



	}

	void teleport() {
		if (getDistanceFromCenter(x, y, z, portal1.pos) < portal1.r) {
			x = portal2.pos.x;
			//y = portal2.pos.y;
			z = portal2.pos.z;
			camera.Eye.x = x;
			camera.Eye.y = y + 3;
			camera.Eye.z = z;


		}
		else if (getDistanceFromCenter(x, y, z, portal2.pos) < portal2.r) {
			x = portal1.pos.x;
			//y = portal1.pos.y;
			z = portal1.pos.z;
			camera.Eye.x = x;
			camera.Eye.y = y + 3;
			camera.Eye.z = z;
		}
		if (isFirstPerson) firstPerson();
		else thirdPerson();
	}
};



Wall wall = Wall(0, 0, 0, 40, 10, 40, boarderWall);

Player player = Player(18, 2.35, 18, 0);


class Shot
{
public:
	Vector pos, dir;
	int color;
	bool state; // moving or still

	Shot() {
		pos = Vector(-5, -5, -5);
	}
	Shot(Vector _pos, Vector _dir, int _color) : pos(_pos), dir(unitVector(_dir)), color(_color), state(true) {
	}

	void draw() {
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		glRotated(90, 1, 0, 0);
		glColor3f(1, 0, 0);
		if (color == 0) glColor3f(0, 0, 1);
		glScaled(.3, .3, .3);
		GLUquadricObj* sphere = gluNewQuadric();
		gluQuadricTexture(sphere, true);
		gluQuadricNormals(sphere, GLU_SMOOTH);
		gluSphere(sphere, .2, 50, 50);
		gluDeleteQuadric(sphere);
		glPopMatrix();
	}
	//int isInWall(Wall wall) {
	//	GLdouble radius = 1;
	//	GLdouble l; GLdouble w;
	//	l = wall.l;// x
	//	w = wall.w;// z
	//	if (abs(wall.x - pos.x) <= radius + l / 2.0 && abs(wall.z - pos.z) <= radius + w / 2.0) {
	//		GLdouble dirX = (wall.x - pos.x);
	//		GLdouble dirZ = (wall.z - pos.z);
	//		if (wall.x + l / 2.0 > pos.x && pos.x > wall.x - l / 2.0) {
	//			if (dirZ >= 0)
	//				return 4;
	//			return 3;
	//		}
	//		if (wall.z + w / 2.0 > pos.z && pos.z > wall.z - w / 2.0)
	//		{
	//			if (dirX >= 0)
	//				return 2;
	//			return 1;
	//		}
	//	}
	//	return 0;
	//}


	bool move() {
		// true if shot hit a portal window and false otherwise
		if (state) pos += dir * .3;
		for (int i = 0; i < portals.size(); i++) {
			Vector a = portals[i];
			if (getDistanceFromCenter(pos.x, pos.y, pos.z, a) < square(1)) {
				state = false;
				if (color == 0) {
					player.portal1.update(a.x, a.y, a.z);
				}
				else {
					player.portal2.update(a.x, a.y, a.z);
				}
					pos = Vector(-5, -5, -5);
				return true;
			}
			else if (pos.x > 20 || pos.x < -20 || pos.z>20 || pos.z < -20) {
				state = false;
				pos = Vector(-5, -5, -5);
				return false;
			}
		}

		return false;
	}

};




class Coin {
public:
	Vector pos;
	int type;
	Coin(Vector _pos, int _type) {
		pos = _pos;
		type = _type;
	}
	void draw() {
		if (type == 0) {
			glPushMatrix();
			glTranslated(pos.x, pos.y, pos.z);

			glRotated(90, 0, 0, 1 );

			//glRotated(90,0, 0, 1);		

			glScaled(.1, .1, .1);

			model_silverCoin.Draw();

			glPopMatrix();
		}
		else {
			glPushMatrix();
			glTranslated(pos.x, pos.y, pos.z);

			glRotated(90, 0, 0, 1);

			//glRotated(90,0, 0, 1);		

			//glScaled(.1, .1, .1);

			model_goldCoin.Draw();

			glPopMatrix();

		}
	}

	void pickUp() {
		if (getDistanceFromCenter(player.x, player.y, player.z, pos) < square(1)) {
			pos = Vector(-5, -5, -5);
			coins++;
		}
	}
};

class PressurePlate {
public:
	Vector pos;
	PressurePlate(Vector _pos) {
		pos = _pos;
	}
	void draw() {
		//glPushMatrix();
		//glColor3f(1, 1, 0);
		//glTranslated(pos.x, pos.y, pos.z);
		//glRotated(-90, 1, 0, 0);
		//glutSolidCone(2, 2, 20, 20);
		//glPopMatrix();

		glDisable(GL_LIGHTING);	// Disable lighting 

		glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

		glPushMatrix();

		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(pos.x, .4, pos.z);
		glBindTexture(GL_TEXTURE_2D, red.texture[0]);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		glScaled(1, .1, 1);
		gluSphere(qobj, 1, 20, 20);
		glRotated(-90, 1, 0, 0);
		glScaled(1, 1, 1);
		gluCylinder(qobj, 1,1,.2, 20, 20);

		gluDeleteQuadric(qobj);

		glPopMatrix();
		glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.


	}
	bool isPushed() {
		if (getDistanceFromCenter(player.x, 0, player.z, pos) < square(2))
			return true;
		return false;
	}
	void controlWalls() {
		int n = firstLevelWalls.size();
		if (n > 0)
			firstLevelWalls[n - 1].h = isPushed() ? 3 : 10;


	}
};

class Buzzer {
public:
	Vector pos;
	GLdouble r = 1;
	bool state = false;
	Buzzer(Vector _pos) {
		pos = _pos;

	}

	void draw() {
		glPushMatrix();
		glColor3f(1, 0, 0);
		glTranslated(pos.x, .2, pos.z);
		glScaled(1,.1,1);
		glutSolidSphere(r, 20, 20);
		glPopMatrix();
	}

	void isPushed() {
		if (getDistanceFromCenter(player.x, 0, player.z, pos) < square(r))
		{
			state = 1;
			win = true;
			sndPlaySound(TEXT("win.wav"), SND_ASYNC);
		}
	}

};

class SlidingPlatform {
public:
	Vector pos;
	GLdouble l = 15, h = 1, w = 6;
	Buzzer buzzer = Buzzer(Vector(pos.x, pos.y, pos.z));
	bool wasUnderPlayer = false;
	SlidingPlatform(Vector _pos) {
		pos = _pos;
		buzzer.pos = Vector(pos.x - l / 4.0, pos.y, pos.z);

	}

	void draw() {
		glDisable(GL_LIGHTING);	// Disable lighting 

		
		//glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

		glEnable(GL_TEXTURE_2D);	// Enable 2D texturing
		glPushMatrix();
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, slider.texture[0]);	// Bind the ground texture
		glTranslated(pos.x, .1, pos.z);
		glPushMatrix();
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);	// Set quad normal direction.
		glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
		glVertex3f(-7.5, 0, -3);
		glTexCoord2f(1, 0);
		glVertex3f(7.5, 0, -3);
		glTexCoord2f(1, 1);
		glVertex3f(7.5, 0, 3);
		glTexCoord2f(0, 1);
		glVertex3f(-7.5, 0, 3);
		glEnd();
		glPopMatrix();

		glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

		glColor3f(1, 1, 1);
		glPopMatrix();
		//glPushMatrix();
		//glColor3f(1, 1, 0);

		//glTranslated(pos.x, pos.y - (h / 2.0 - 0.03), pos.z);
		//glScaled(l, h, w);
		//glutSolidCube(1);
		//glPopMatrix();
		buzzer.draw();
	}

	bool underPlayer() {
		if (pos.x - l / 2.0 < player.x && player.x < pos.x + l / 2.0) {
			if (pos.z - w / 2.0 < player.z && player.z < pos.z + w / 2.0) {
				return true;
			}
		}
		return false;
	}

	void update() {
		if (underPlayer()) {
			if (pos.z - w / 2.0 > -20.0) {

				pos.z = max(pos.z - 0.1, -20 + w / 2.0);
				buzzer.pos.z -= 0.1;
				player.x = pos.x;
				player.z = pos.z;
				if (player.isFirstPerson) {
					player.firstPerson();
				}
				else {
					player.thirdPerson();
				}
			}
			wasUnderPlayer = true;

		}
		else if (wasUnderPlayer) {
			player.z = min(-14, player.z);
			if (player.isFirstPerson) {
				player.firstPerson();
			}
			else {
				player.thirdPerson();
			}
		}


		if(!win) buzzer.isPushed();
		cout << buzzer.state << endl;
	}


};


PressurePlate pressurePlate = PressurePlate(Vector(0, 0, 15));
SlidingPlatform slidingPlatform = SlidingPlatform(Vector(12.5, 0, 11.5));
Shot shot1, shot2;
Coin coin1 = Coin(Vector(10, 2, -15),0), coin2 = Coin(Vector(10,2,11),0);



void playerUpdate(int x) {
	player.update();
	if (level == 2) {
		slidingPlatform.update();
	}
	shot1.move();
	shot2.move();
	glutTimerFunc(20, playerUpdate, 0);
}

void drawTargets() {
	for (int i = 0; i < portals.size(); i++) {
		Vector portalPos = portals[i];
		glPushMatrix();
		glTranslated(portalPos.x, portalPos.y, portalPos.z);
		glutSolidSphere(.1, 20, 20);
		glPopMatrix();

	}
}

void drawElevator() {

	double l = 5, w = 5, h = 6;

	glPushMatrix();

	glDisable(GL_LIGHTING);

	glColor3f(1, 1, 1);
	glTranslated(-22.5, 0, 0);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, elevatorTex.texture[0]);

	glPushMatrix();
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);
	glVertex3f(l / 2, 0, -w / 2);
	glTexCoord2f(1, 0);
	glVertex3f(l / 2, h, -w / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-l / 2, h, -w / 2);
	glTexCoord2f(0, 1);
	glVertex3f(-l / 2, 0, -w / 2);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, elevatorTex.texture[0]);

	glPushMatrix();
	glColor3f(.8, .5, .2);

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);
	glVertex3f(l / 2, 0, w / 2);
	glTexCoord2f(1, 0);
	glVertex3f(l / 2, h, w / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-l / 2, h, w / 2);
	glTexCoord2f(0, 1);
	glVertex3f(-l / 2, 0, w / 2);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, elevatorTex.texture[0]);

	glPushMatrix();

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(-l / 2, 0, w / 2);
	glTexCoord2f(1, 0);
	glVertex3f(-l / 2, h, w / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-l / 2, h, -w / 2);
	glTexCoord2f(0, 1);
	glVertex3f(-l / 2, 0, -w / 2);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, elevatorTex.texture[0]);

	glPushMatrix();

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(l / 2, h, w / 2);
	glTexCoord2f(1, 0);
	glVertex3f(-l / 2, h, w / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-l / 2, h, -w / 2);
	glTexCoord2f(0, 1);
	glVertex3f(l / 2, h, -w / 2);
	glEnd();
	glPopMatrix();


	glBindTexture(GL_TEXTURE_2D, elevatorTex.texture[0]);

	glPushMatrix();

	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(l / 2, 0, w / 2);
	glTexCoord2f(1, 0);
	glVertex3f(-l / 2, 0, w / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-l / 2, 0, -w / 2);
	glTexCoord2f(0, 1);
	glVertex3f(l / 2, 0, -w / 2);
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.
	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	//glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}



void LoadAssets()
{
	// Loading Model files
	model_gun.Load("Models/gun/PortalGun.3DS");
	model_display.Load("Models/display/scifi_display_3DS.3DS");
	model_terror.Load("Models/torremoba/torremoba3ds.3DS");
	model_1.Load("Models/ironman/Homem de ferro Edvaldo Costa Cordeiro.3DS");
	model_vending.Load("Models/Bitcoin_metal_coin.3ds");
	model_goldCoin.Load("Models/gold/gold.3DS");
	model_silverCoin.Load("Models/silver/Coin1.3DS");
	model_key.Load("Models/Key_B_02.3ds/Key_B_02.3DS");

	//model_2.Load("Models/respawn/respawntool.3DS");
	//model_3.Load("Models/wall/wall.3DS");
	//model_4.Load("Models//	.3DS");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	wallTex.Load("Textures/wall.bmp");
	boarderWall.Load("Textures/wall7.bmp");
	doorTex.Load("Textures/door.bmp");
	goldTex.Load("Textures/gold.bmp");
	silverTex.Load("Textures/silver.bmp");
	elevatorTex.Load("Textures/elevator.bmp");
	slider.Load("Textures/slider.bmp");
	red.Load("Textures/red.bmp");
	portalTex.Load("Textures/portal.bmp");


	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}





//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void setUpSecondLevel() {
	//player.x = -15;
	//player.z = 0;
	//player.r = 180;
	player.portal1.update(-15, 0, 0);
	player.portal2.update(-5, 0, 10);
	coin1.pos = Vector(13, 3, 10);
	coin2.pos = Vector(15, 3, 18);
	firstLevelWalls.clear();
	portals.clear();
	firstLevelWalls.push_back(Wall(-10, 0, 0, 1, 3, 40, wallTex));
	// coin two compartment
	firstLevelWalls.push_back(Wall(12, 0, 15, 16, 3, 1, wallTex));
	firstLevelWalls.push_back(Wall(5, 0, 0, 1, 3, 40, wallTex));

	portals.push_back(Vector(15, 4, 18));
	portals.push_back(Vector(15, 4, 10));
	portals.push_back(Vector(0, 4, 0));

	// x, y, z, rotation, length, height, width, texture
	wall.wallTex = boarderWall;



	//firstLevelWalls.push_back(Wall(0, 0, 0, 5, 5, 8, 1, wallTex));
	gameReady = true;
}

void so(int) {
	sndPlaySound(TEXT("background.wav"), SND_ASYNC | SND_LOOP);

}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.2;
	Vector gunVector = Vector(player.x - 0.5 * sin(DEG2RAD(player.r)), 2, player.z - .5 * cos(DEG2RAD(player.r)));
	switch (key) {
	case 'w':
		player.front = true;
		//player.camera.moveY(d);
		break;
	case 's':
		player.back = true;
		//player.camera.moveY(-d);
		break;
	case 'a':
		player.left = true;
		//player.camera.moveX(d);
		break;
	case 'd':
		player.right = true;
		//player.camera.moveX(-d);
		break;
	case 'q':
		shot1 = Shot(gunVector, player.camera.At - gunVector, 0);
		break;
	case 'e':
		//player.camera.moveZ(-d);
		shot2 = Shot(gunVector, player.camera.At - gunVector, 1);
		break;
	case '1':
		player.firstPerson();
		break;
	case '2':
		player.thirdPerson();
		break;

	case 'i':
		player.camera.Eye.y += .1;
		break;
	case 'k':
		player.camera.Eye.y -= .1;
		break;
	case 'j':
		player.camera.Eye.x += .1;
		break;
	case 'l':
		player.camera.Eye.x -= .1;
		break;
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	case ' ':
		player.teleport();
		break;
	case ']':
		level = 2;
		setUpSecondLevel();
		break;
	case 'n':
		level1Passed = true;
		glutTimerFunc(0, doorSlideUp, 0);
		break;
	case 'g':
		if (coins > 0) {
			hasBlue = true;
			coins--;
			sndPlaySound(TEXT("buy.wav"), SND_ASYNC );
			glutTimerFunc(2000, so, 0);
		}
		break;
	case 'h':
		if (coins > 0) {
			hasRed = true;
			coins--;
			sndPlaySound(TEXT("buy.wav"), SND_ASYNC);
			glutTimerFunc(2000, so, 0);
		}
		break;
	case '.':
		win = true;
		break;
	}

}

void keyUp(unsigned char key, int x, int y) {

	switch (key) {
	case 'w':
		player.front = false;
		//player.camera.moveY(d);
		break;
	case 's':
		player.back = false;
		//player.camera.moveY(-d);
		break;
	case 'a':
		player.left = false;
		//player.camera.moveX(d);
		break;
	case 'd':
		player.right = false;
		//player.camera.moveX(-d);
		break;
	case 'q':
		//player.camera.moveZ(d);
		break;
	case 'e':
		break;
	case '1':
		//player.firstPerson();
		break;
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}

}

void Special(int key, int x, int y) {
	float a = 2.0;

	switch (key) {
	case GLUT_KEY_UP:
		player.camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		player.camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		player.camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		player.camera.rotateY(-a);
		break;
	}

}

void mouseMove(int x, int y) {
	/*double half = 863 / 2;
	player.camera.At.y += (half - y) / (abs(half-y)/5);
	cout << player.camera.At.y << endl;*/
	/*GLdouble temp = (half - y) / 10.0;
	if (temp < 0) {
		player.camera.moveY(-0.2);

	}else
		player.camera.moveY(0.2);*/


	if (!gameReady)
	{
		return;
	}
	if (x > preX) {
		player.rotateY(-1);

	}
	else if (x < preX) {
		player.rotateY(1);

	}

	if (y > preY) {
		player.moveY(-.1);
	}
	else if (y < preY) {
		player.moveY(0.1);
	}

	if (x == 0 || x == 1535 || y == 0 || y == 863) {
		glutWarpPointer(1535 / 2, 863 / 2);
		preX = 1535 / 2;
		preY = 863 / 2;
	}
	else {
		preX = x;
		preY = y;
	}


}

void mousePress(int button, int state, int x, int y) {
	Vector gunVector = Vector(player.x - 0.5 * sin(DEG2RAD(player.r)), 2, player.z - .5 * cos(DEG2RAD(player.r)));
	if (hasBlue && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		shot1 = Shot(gunVector, player.camera.At - gunVector, 0);

	}
	if (hasRed && button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
	shot2 = Shot(gunVector, player.camera.At - gunVector, 1);

	}
	//glutPostRedisplay();
}


//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================
void endgame(int) {
	exit(0);
}



void myDisplay(void)
{
	if (win) {
		win = false;
		glutTimerFunc(5000, exit, 0);
	}


	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	//glEnable(GL_TEXTURE_2D);
	// Draw Ground
	RenderGround();


	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);

	glPopMatrix();

	//glDisable(GL_TEXTURE_2D);
	drawTargets();
	//wall.draw();
	player.draw();
	shot1.draw();
	shot2.draw();
	door.draw();
	coin1.draw();
	coin1.pickUp();
	coin2.draw();
	coin2.pickUp();
	if (level != 1) {
		pressurePlate.draw();
		pressurePlate.controlWalls();
		slidingPlatform.draw();
		glPushMatrix();
		glColor3f(0, 0, 0);
		glTranslated(12.5, -0.49, -2.5);
		glScaled(15, 1, 35);
		glutSolidCube(1);
		glPopMatrix();
	}
	cout << "player " << Vector(player.x, player.y, player.z).toString() << endl;
	cout << " Eye " << player.camera.Eye.toString() << endl;
	cout << " At " << player.camera.At.toString() << endl;

	//model_display.Draw();

	//glPushMatrix();
	//model_terror.Draw();
	//glPopMatrix();

	glPushMatrix();
	glTranslated(-5, 0, 0);
	//model_2.Draw();
	glPopMatrix();	glPushMatrix();
	glTranslated(0, 0, 5);
	//model_3.Draw();
	glPopMatrix();

	for (auto i = firstLevelWalls.begin(); i != firstLevelWalls.end(); i++) {
		(*i).draw();
	}

	drawElevator();
	if (!level1Passed && level != 2) {
		if (abs(player.x + 18) < 2 && abs(player.z - 17) < 2) {
			level1Passed = true;
			glutTimerFunc(0, doorSlideUp, 0);
		}
	}

	if (!level1Passed && level != 2) {

	glPushMatrix();
	glTranslated(-18,1, 17);
	glRotated(90,0,0,1);
	glScaled(.15,.15,.15);
	model_key.Draw();
	glPopMatrix();
	}


	glutSwapBuffers();
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 0.5f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, .5f };
	//GLfloat specular[] = { .5f, .5f, .5, .5f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
	GLfloat lightPosition[] = { 10.0f, 0.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	GLfloat l0Direction[] = { -1.0,0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);

	if (level == 2) {
		GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 0.5f };
		GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, .5f };
		//GLfloat specular[] = { .5f, .5f, .5, .5f };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
		//glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
		GLfloat lightPosition[] = { -10.0f, 0.0f, 0.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		GLfloat l0Direction[] = { 1.0,0.0, 0.0 };
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);
	}

	GLfloat l1Diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat l1Spec[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat l1Ambient[] = { 1.f, 0.0f, 0.0f, 1.0f };
	GLfloat l1Position[] = { player.portal2.pos.x,20.0f, player.portal2.pos.z, true};
	GLfloat l1Direction[] = { 0.0, -1.0, 0.0 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);

	GLfloat l2Diffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat l2Spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat l2Ambient[] = { 0.f, 0.0f, 1.0f, 1.0f };
	GLfloat l2Position[] = { player.portal1.pos.x,20.0f, player.portal1.pos.z, true };
	GLfloat l2Direction[] = { 0.0, -1.0, 0.0 };
	glLightfv(GL_LIGHT2, GL_DIFFUSE, l2Diffuse);
	glLightfv(GL_LIGHT2, GL_AMBIENT, l2Ambient);
	glLightfv(GL_LIGHT2, GL_POSITION, l2Position);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, l2Direction);
}
void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	player.camera.look();
}

//=======================================================================
// Main Function
//=======================================================================

void setUpFirstLevel() {
	player.portal1.update(8, 0, 17.8);
	player.portal2.update(18, 0, 10);


	// x, y, z, rotation, length, height, width, texture
	wall.wallTex = boarderWall;

	// first wall to the right 
	firstLevelWalls.push_back(Wall(12, 0, 15, 16, 10, 1, wallTex));


	// coin one compartment
	firstLevelWalls.push_back(Wall(12, 0, -10, 16, 3, 1, wallTex));
	firstLevelWalls.push_back(Wall(5, 0, -15, 1, 3, 10, wallTex));

	// door wall
	firstLevelWalls.push_back(Wall(-10, 0, 0, 1, 3, 40, wallTex));

	// first wall in the front 
	firstLevelWalls.push_back(Wall(5, 0, 15, 1, 10, 10, wallTex));

	firstLevelWalls.push_back(Wall(-15, 0, 10, 10, 10, 1, wallTex));




	portals.push_back(Vector(0, 4, 0));
	portals.push_back(Vector(5, 4, 5));
	portals.push_back(Vector(15, 4, -15));
	portals.push_back(Vector(-15, 4, 0));
	portals.push_back(Vector(-15,4, 15));


	//firstLevelWalls.push_back(Wall(0, 0, 0, 5, 5, 8, 1, wallTex));
	gameReady = true;
}

void refresh(int) { // 100 frames per second
	glutPostRedisplay();
	glutTimerFunc(20, refresh, 0);
}


void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(-150, -150);

	glutCreateWindow(title);
	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	glutPassiveMotionFunc(mouseMove);

	LoadAssets();
	glutDisplayFunc(myDisplay);

	glutTimerFunc(10, playerUpdate, 0);
	glutTimerFunc(10, refresh, 0);
	setUpFirstLevel();


	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(Special);
	glutMouseFunc(mousePress);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glutWarpPointer(1535 / 2, 863 / 2);
	sndPlaySound(TEXT("background.wav"), SND_ASYNC | SND_LOOP);

	glutMainLoop();
}