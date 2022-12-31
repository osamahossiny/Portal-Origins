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


//================================================================================================//

int WIDTH = 1920;
int HEIGHT = 1080;

int preX = 1535/2, preY = 863/2;
bool gameReady = false;
int level = 1;
GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;


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

// Textures
GLTexture tex_ground;
GLTexture wallTex;
GLTexture boarderWall;


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
		return to_string(x) +" " + to_string(y) + " " + to_string(z);
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
class Portal {
public:
	Vector pos = Vector(0, 0, 0);
	GLdouble  r = 10, h = 1;
	Portal() {

	};
	void draw() {
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		drawCylinder(r, h);
		glPopMatrix();
	}
	void update(GLdouble _x, GLdouble _y, GLdouble _z) {
		pos.x = _x;
		pos.y = _y;
		pos.z = _z;
	}
};
GLdouble square(GLdouble num) {
	return num * num;
}
GLdouble getDistanceFromCenter(GLdouble x, GLdouble y, GLdouble z, Vector v2) {
	return (square(x - v2.x) + square(y - v2.y) + square(z - v2.z));
}

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
	GLdouble x, y, z, r, l, h, w;
	GLTexture wallTex;

	Wall(GLdouble _x, GLdouble _y, GLdouble _z, GLdouble _r, GLdouble _l, GLdouble _h, GLdouble _w, GLTexture _wallTex)
	{
		x = _x; // x coord of the center point
		y = _y; // y coord of the center point
		z = _z;	// z coord of the center point
		r = _r;	// rotation angle
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
		glRotated(r, 0, 1, 0);

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

class Player
{
public:
	GLdouble x, y, z, r;
	GLdouble speed = 0.2;
	Camera camera;
	bool left = false, right = false, front = false, back = false, isFirstPerson = true;
	Portal portal1, portal2;
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
		r = (r / 360 - (int)r / 360) * 360 ;
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
		if(!isFirstPerson) glTranslated(.5, -.2, -.3);
		glRotated(55,0,1,0);
		glScaled(.5,.5,.5);
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

	void update() {
		if (left) {
			//x-=speed;
			Vector vec = Vector(0,0,0);
			vec.x = camera.Eye.x;
			vec.y = camera.Eye.y;
			vec.z = camera.Eye.z;
			camera.moveX(speed);
			vec = camera.Eye - vec;
			x += vec.x;
			z += vec.z;
		}
		if (right) {
			//x += speed;
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

Wall wall = Wall(0, 0, 0, 0, 40, 10, 40, boarderWall);
vector< Wall > firstLevelWalls;

Player player = Player(18, 2.35, 18, 0);

void playerUpdate(int x) {
	player.update();
	glutTimerFunc(20, playerUpdate, 0);
}



void LoadAssets()
{
	// Loading Model files
	model_gun.Load("Models/gun/PortalGun.3DS");
	model_display.Load("Models/display/scifi_display_3DS.3DS");
	model_terror.Load("Models/torremoba/torremoba3ds.3DS");
	model_1.Load("Models/ironman/Homem de ferro Edvaldo Costa Cordeiro.3DS");
	//model_2.Load("Models/respawn/respawntool.3DS");
	//model_3.Load("Models/wall/wall.3DS");
	//model_4.Load("Models//	.3DS");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	wallTex.Load("Textures/wall.bmp");
	boarderWall.Load("Textures/wall7.bmp");
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
void Keyboard(unsigned char key, int x, int y) {
	float d = 0.2;

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
		//player.camera.moveZ(d);
		break;
	case 'e':
		player.camera.moveZ(-d);
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
	case ' ' :
		player.teleport();
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

	if (x == 0 || x==1535 || y==0 || y==863) {
		glutWarpPointer(1535 / 2, 863 / 2);
		preX = 1535 / 2;
		preY = 863 / 2;
	}
	else {
	preX = x;
	preY = y;
	}


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

void drawShot(double x, double y, double z) {
	glPushMatrix();

	GLfloat lmodel_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	GLfloat l0Diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat l0Spec[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat l0Ambient[] = { 0.1f, 0.0f, 0.0f, 1.0f };
	GLfloat l0Position[] = { 10.0f, 0.0f, 0.0f, true };
	GLfloat l0Direction[] = { -1.0, 0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);

	glPopMatrix();
}



void myDisplay(void)
{
	setupCamera();
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

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
	wall.draw();
	player.draw();

	cout<<"player "<<Vector(player.x, player.y, player.z).toString() << endl;
	cout<<" Eye "<<player.camera.Eye.toString() << endl;
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

	if(level ==1){
		for (auto i = firstLevelWalls.begin(); i != firstLevelWalls.end();i++) {
			(* i).draw();
		}
	}


	glutSwapBuffers();
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
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
	player.portal1.update(5,5,5);
	player.portal2.update(0, 0, 0);


	// x, y, z, rotation, length, height, width, texture
	wall.wallTex = boarderWall;

	// first wall to the right 
	firstLevelWalls.push_back(Wall(15, 0, 15, 0, 10, 10, 1, wallTex));

	// first wall in the front 
	firstLevelWalls.push_back(Wall(5, 0, 15, 90, 10, 10, 1, wallTex));


	//firstLevelWalls.push_back(Wall(0, 0, 0, 5, 5, 8, 1, wallTex));
	gameReady = true;
}

void refresh(int) { // 100 frames per second
	glutPostRedisplay();
	glutTimerFunc(20,refresh,0);
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glutWarpPointer(1535 / 2, 863/ 2);

	glutMainLoop();
}