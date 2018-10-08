#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <mmsystem.h>
#include <math.h>
#include "raaCamera/raaCamera.h"
#include "raaUtilities/raaUtilities.h"
#include "raaMaths/raaMaths.h"
#include "raaMaths/raaVector.h"
#include "raaMaths/raaMatrix.h"
#include "linkedList/doublyLinkedList.h"
#include "imageloader.h"

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <mmsystem.h>

raaCameraInput g_Input;
raaCamera g_Camera;
raaCamera g_planetCamera;

unsigned long g_ulGrid = 0;
int g_aiLastMouse[2];
int g_aiStartMouse[2];
bool g_bExplore = false;
bool g_bFly = false;
bool g_bRun = false;
bool g_bColl = true;
bool g_bAdd = false;
bool g_bFog = false;

//void gridInit();
void drawCar();
void display();
void idle();
void reshape(int iWidth, int iHeight);
void keyboard(unsigned char c, int iXPos, int iYPos);
void keyboardUp(unsigned char c, int iXPos, int iYPos);
void sKeyboard(int iC, int iXPos, int iYPos);
void sKeyboardUp(int iC, int iXPos, int iYPos);
void mouse(int iKey, int iEvent, int iXPos, int iYPos);
void mouseMotion();
void myInit();

void collisionDetection();
void addBody(float *v_planetPosition, float *v_velocity, float *v_rUp, float *v_distancePlanetSun);
float *translatePositionFromScreenToGL(int x, int y);
void writeToTextFile();
unsigned int loadTexture(Image* image);

unsigned long g_ulLastTime = 0;
float g_fFrameTime = 0.0f;

const static unsigned int csg_uiInitialNumSpheres = 200;
unsigned int g_uiLiveNumSpheres = csg_uiInitialNumSpheres;

float g_afPos[3 * csg_uiInitialNumSpheres];
float g_afCol[3 * csg_uiInitialNumSpheres];
float g_afSizes[csg_uiInitialNumSpheres];
float m_fSphOri = 0.0f;
float g_fFogDensity = 0.001f; //set the density to 0.001 not to much thick
float g_afDumpingCoeff = 0.97f;

//car animation
float f_carForwBack = -200.f;
float f_carSide = -7.f;
float f_carRotAngle = 90.f;
float f_carIngnitionDisplacement = 1.68f;
float v_CarCamera[4]; 
bool b_carCamera = false;
bool b_carIngnitionDisplacement = false;

Node *pSun;

//deal with texturing
unsigned int _textureId;
GLUquadric *quad;

void display()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	camApply(g_Camera);

	//	gridDraw(g_ulGrid);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_LIGHTING);

	// car initilisation 
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();
	drawCar();
	glPopMatrix();
	glDisable(GL_COLOR_MATERIAL);



	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext)
	{

		glPushMatrix();

		if (pElement != pSun) {
			glRotatef(m_fSphOri, 0.05f, 0.5f, 0.05f);
			glTranslatef(pElement->m_position[0], pElement->m_position[1], pElement->m_position[2]);

			colToMat(pElement->m_pcolor);
			glutSolidSphere(powf(pElement->m_mass, 0.33f), 10, 10);
			//glutSolidTeapot(powf(pElement->m_mass, 0.33f));
		}
		else {
			//texture the sun
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, _textureId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			gluQuadricTexture(quad, 1);
			gluSphere(quad, powf(pElement->m_mass, 0.33f), 10, 10);
		}

		glPopMatrix();

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		glColor3fv(pElement->m_pcolor);
		glBegin(GL_LINE_STRIP);
		for (Data *historyPosition = pElement->registerPosition; historyPosition; historyPosition = historyPosition->m_pNextData) {
			glVertex3fv(historyPosition->m_data_position);
		}

		glEnd();
		glPopAttrib(); // end isolation of the object properties
	}

	glPopAttrib(); // end isolation of the object properties


	//end placeholder

	glFlush();
	glutSwapBuffers();
}


void collisionDetection() {
	if (g_bColl) {
		//boolean flag to escape outer loop
		bool escapeOuterLoop = false;
		for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext)
		{
			if (!escapeOuterLoop) {
				//initialise vector for collision and float distance
				float v_collision[4]; vecInitPVec(v_collision); float distance = 0.0f;
				//detecting collision
				for (Node *pBody = g_pHead; pBody; pBody = pBody->m_pNext) {
					//if element and are different 
					if (pBody != pElement) {
						//calculate the distance between pBody vec and pElement vec
						vecSub(pElement->m_position, pBody->m_position, v_collision);
						//calculate lenght of vec
						distance = vecLength(v_collision);
						//if distance is smaller than body or element radius
						if (distance <= powf(pBody->m_mass, 0.33f) || distance <= powf(pElement->m_mass, 0.33f)) {
							//if body is not the sun
							if (pBody != pSun) {
								//add mass to element
								pElement->m_mass += pBody->m_mass;
								remove(pBody);
								break;
							}
							else {
								//add mass to body
								pBody->m_mass += pElement->m_mass;
								remove(pElement);
								escapeOuterLoop = true;
							}
							//decreasing number of spheres count
							g_uiLiveNumSpheres--;
						}
					}
				}
			}
			//if boolean flag is set to true, escape from outer loop
			if (escapeOuterLoop)
				break;
		}
	}
}
 


void idle()
{
	mouseMotion();
	
	//getting the frame time
	if (g_ulLastTime) {
		//get time stamp
		unsigned long ulTime = timeGetTime();
		//time between the current and the last frame in seconds
		g_fFrameTime = ((float)(ulTime - g_ulLastTime)) / 1000.0f;
		//update time
		g_ulLastTime = ulTime;
	}
	else {
		//get time stamp
		g_ulLastTime = timeGetTime();
	}

	// camera locked to the car
	if (b_carCamera) {
		v_CarCamera[0] = f_carSide;
		v_CarCamera[2] = f_carForwBack;
		camExploreUpdateTarget(g_Camera, v_CarCamera);
	}

	if (f_carIngnitionDisplacement > 1.8f) {
		b_carIngnitionDisplacement = true;
	}else if (f_carIngnitionDisplacement < 1.68f) {
		b_carIngnitionDisplacement = false;
	}

	if (b_carIngnitionDisplacement) {
		f_carIngnitionDisplacement -= 0.005;
	}
	else {
		f_carIngnitionDisplacement += 0.005;
	}

	//if flag is set to true add new body to simulation at runtime
	if (g_bAdd) {

		//creating a vector to generate a random position
		float v_planetPosition[4];
		vecInitPVec(v_planetPosition);

		//creating velocity vector
		float v_velocity[4];
		vecInitDVec(v_velocity);

		//creating rUp vector based on assumption
		float v_rUp[4];
		vecInitPVec(v_rUp);
		v_rUp[0] = 0.0f; v_rUp[1] = 1.0f; v_rUp[2] = 0.0f;

		//creating distance sun - planet vector
		float v_distancePlanetSun[4];
		vecInitPVec(v_distancePlanetSun);

		//add body at run time
		addBody(v_planetPosition, v_velocity, v_rUp, v_distancePlanetSun);

		//set flag to false
		g_bAdd = false;
	}

	if (g_bRun)
	{
		for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext)
		{
			//if planet has been selected
			if (pElement->m_camera == 1) {
				//camera update, camera is relative to planet target
				camExploreUpdateTarget(g_Camera, pElement->m_position);
			}

			if (pElement != pSun) {
				//initialise velocity vector
				float v[4];
				vecInitDVec(v);
				//initialise displacment vector
				float v_displacement[4];
				vecInitDVec(v_displacement);
				//initialise displacement normalised vector
				float v_displacement_norm[4];
				vecInitDVec(v_displacement_norm);
				//initialise vector force
				float v_force[4];
				vecInitDVec(v_force);
				//create var for lenght and for scalar force
				float f_length = 0.0f;
				float f_force = 0.0f;
				//create vector for sum of forces
				float F[4];
				vecInitDVec(F);
				//create acceleration vector
				float a[4];
				vecInitDVec(a);
				//create provisional vector to store 1/2at^2
				float at[4];
				vecInitDVec(at);

				for (Node *pBody = g_pHead; pBody; pBody = pBody->m_pNext)
				{
					if (pBody != pElement)
					{
						//calculate displacement between pElement and pBody
						vecSub(pBody->m_position, pElement->m_position, v_displacement);
						//normalise displacement vector
						f_length = vecNormalise(v_displacement, v_displacement_norm);
						//calculate scalar force from Newton's first law of motion
						f_force = 9.8f*((pElement->m_mass*pBody->m_mass) / (f_length*f_length));
						//calculate vector force using scalar product
						vecScalarProduct(v_displacement_norm, f_force, v_force);
						//add pBody force to sum of forces F
						vecAdd(F, v_force, F);
					}
				}

				//calculating acceleration vector from Newton's second law of motion
				vecScalarProduct(F, 1.0f / pElement->m_mass, a);
				//ut
				vecScalarProduct(pElement->m_velocity, g_fFrameTime, v); //cw
																		 //at^2
				vecScalarProduct(a, g_fFrameTime*g_fFrameTime, at);
				//1/2(at^2)
				vecScalarProduct(at, 1.0f / 2.0f, at);
				//ut+1/2(at^2)
				vecAdd(v, at, v);
				//s=p+(ut+1/2at^2) updating position
				vecAdd(pElement->m_position, v, pElement->m_position); //cw
																	   //v=(s-p)/t updating velocity
				vecScalarProduct(v, 1.0f / g_fFrameTime, v);
				vecCopy(v, pElement->m_velocity);
				//apply dumping coefficient 
				vecScalarProduct(v, g_afDumpingCoeff, v);

				//storing history position
				Data *data = createNewData(pElement->m_position);
				pushToHeadData(data, pElement);
			}
		}
	}
	glutPostRedisplay();
	//detecting collision
	collisionDetection();
}


void reshape(int iWidth, int iHeight)
{
	glViewport(0, 0, iWidth, iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0f, ((float)iWidth) / ((float)iHeight), 0.1f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}

void keyboard(unsigned char c, int iXPos, int iYPos)
{
	switch (c)
	{
	case 'r':
		//reset the camera to original position
		camReset(g_Camera);
		for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext)
		{
			pElement->m_camera = 0;
		}
		break;
	case 'w':
		camInputTravel(g_Input, tri_pos);
		break;
	case 's':
		camInputTravel(g_Input, tri_neg);
		break;
	case 'y':
		//run or stop simulation
		g_bRun = !g_bRun;
		break;
	case 'c':
		//switch collision on or off
		g_bColl = !g_bColl;
		break;
	case 'a':
		//if updated number of spheres is less than initial number than allow addition of body, control performance
		if (g_uiLiveNumSpheres < csg_uiInitialNumSpheres)
			g_bAdd = true;
		break;
	case 'x':
		writeToTextFile();
		break;
	case 'd':
		//decrease dumping coefficient
		g_afDumpingCoeff -= 0.01;
		break;
	case 'v':
		//disable and enable fog
		if (g_bFog) { glDisable(GL_FOG); g_bFog = false; }
		else { glEnable(GL_FOG); g_bFog = true; }
		break;
	case 'f':
		//increase fog by a small index
		g_fFogDensity = +0.001f;
		glFogf(GL_FOG_DENSITY, g_fFogDensity); //set the density
		break;

	// car animation
	case 'i':
		f_carForwBack += 1.f;
		break;
	case 'j':
		f_carSide += 1.f;
		break;
	case 'l':
		f_carSide -= 1.f;
		break;
	case 'm':
		f_carForwBack -= 1.f;
		break;
	case 'u':
		f_carRotAngle += 1.f;
		break;
	case 'o':
		f_carRotAngle -= 1.f;
		break;
	case 'n':
		b_carCamera = !b_carCamera;
		break;
	}
}

void keyboardUp(unsigned char c, int iXPos, int iYPos)
{
	switch (c)
	{
	case 'w':
		camInputTravel(g_Input, tri_pos);
		break;
	case 's':
		camInputTravel(g_Input, tri_null);
		break;
	case 'f':
		camInputFly(g_Input, !g_Input.m_bFly);
		break;
	}
}

void sKeyboard(int iC, int iXPos, int iYPos)
{
	switch (iC)
	{
	case GLUT_KEY_UP:
		camInputTravel(g_Input, tri_pos);
		break;
	case GLUT_KEY_DOWN:
		camInputTravel(g_Input, tri_neg);
		break;
	}
}

void sKeyboardUp(int iC, int iXPos, int iYPos)
{
	switch (iC)
	{
	case GLUT_KEY_UP:
		camInputTravel(g_Input, tri_pos);
		break;
	case GLUT_KEY_DOWN:
		camInputTravel(g_Input, tri_null);
		break;
	}
}

float *translatePositionFromScreenToGL(int x, int y)
{
	float vecPosition[3];
	//Viewport values holder
	int viewport[4];
	//Modelview matrix holder
	double modelview[16];
	//Projection matrix holder
	double projection[16];

	float winX, winY, winZ;
	double posX, posY, posZ;

	//retrieve the Modelview Matrix
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	//retrieve the Projection Matrix
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	//retrieve the Viewport Values (X, Y, Width, Height)
	glGetIntegerv(GL_VIEWPORT, viewport);

	//holds X mouse coordinates
	winX = (float)x;
	//subtract the corrent mouse Y coordinates from the screen
	winY = (float)viewport[3] - (float)y;
	//get Z coordinates
	glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	//converting screen coordinates to OpenGL coordinates
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
	//set up vector to be returned
	vecPosition[0] = (float)posX; vecPosition[1] = (float)posY; vecPosition[2] = (float)posZ;
	//return vector having position X,Y,Z stored
	return vecPosition;
}

unsigned int loadTexture(Image* image) {
	unsigned int textureId;
	glGenTextures(1, &textureId); //Make room for texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
											 //Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
	return textureId; //Returns the id of the texture
}

void mouse(int iKey, int iEvent, int iXPos, int iYPos)
{
	if (iKey == GLUT_LEFT_BUTTON)
	{
		camInputMouse(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);

	}
	else if (iKey == GLUT_RIGHT_BUTTON) {
		//create and initialise a new vector
		float vecPosition[4];
		vecInitPVec(vecPosition);
		//copy the returned value to new vector position
		vecCopy(translatePositionFromScreenToGL(iXPos, iYPos), vecPosition);
		unsigned int iCount = 0;
		bool l_bBodySelected = false;
		for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext)
		{
			iCount++;
			//if absolute value of the clicked position is in between the absolute value of the planet position and absolute value of the planet position plus its radius
			if (((fabsf(vecPosition[0]) >= fabsf(pElement->m_position[0])) && (fabsf(vecPosition[0]) <= (fabsf(pElement->m_position[0]) + powf(pElement->m_mass, 0.33f))))
				&& ((fabsf(vecPosition[1]) >= fabsf(pElement->m_position[1])) && (fabsf(vecPosition[1]) <= (fabsf(pElement->m_position[1]) + powf(pElement->m_mass, 0.33f))))) {
				//this flag deal with concurrect selection
				if (l_bBodySelected == false) {
					l_bBodySelected = true;
					//print out information about the planet
					printf("Planet Number: %d\n", iCount);
					printf("Planet Mass: %f\n", pElement->m_mass);
					printf("Planet Radius: %f\n", powf(pElement->m_mass, 0.33f));
					printf("Planet Pos: %f | | %f | | %f\n", pElement->m_position[0], pElement->m_position[1], pElement->m_position[2]);
					printf("--------------------------------------------------------------------------\n");
					//set value to 1
					pElement->m_camera = 1;
					//camera update, camera is relative to planet target
					camExploreUpdateTarget(g_Camera, pElement->m_position);
					//decrease m.fFar to make camera closer to the planet
					g_Camera.m_fFar = 2000.0f;
					//iterate through the rest of the body setting values to 0
					for (Node *pBody = g_pHead; pBody; pBody = pBody->m_pNext) {
						if (pElement != pBody) {
							pBody->m_camera = 0;
						}
					}
				}
			}
		}
	}
}

void motion(int iXPos, int iYPos)
{
	if (g_Input.m_bMouse) camInputSetMouseLast(g_Input, iXPos, iYPos);
}

void mouseMotion()
{
	camProcessInput(g_Input, g_Camera);
	glutPostRedisplay();
}

void addBody(float *v_planetPosition, float *v_velocity, float *v_rUp, float *v_distancePlanetSun) {
	if (g_bAdd) {
		//generating random float values for x,y,z position
		v_planetPosition[0] = randFloat(-30.0f*powf(pSun->m_mass, 0.33f), 30.0f*powf(pSun->m_mass, 0.33f)); //generate a random value for x
		v_planetPosition[1] = randFloat(-22.0f, 22.0f); //generate a random value for y
		v_planetPosition[2] = randFloat(-30.0f*powf(pSun->m_mass, 0.33f), 30.0f*powf(pSun->m_mass, 0.33f)); //generate a random value z

		int i = rand() % 20;
		//generate a random vector for color
		vecRand(0.3f, 1.0f, g_afCol + 3 * i);
		//generate a random float for size
		g_afSizes[i] = randFloat(1.0f, 1000.0f);
		//create a single linked list for each planet to store position history
		Data *historyPosition = createNewData(v_planetPosition);
		//create new planet, vector position, size and color
		Node *pElement = createNewElement(v_planetPosition, powf(g_afSizes[i], 0.33f), g_afCol + 1 * i, historyPosition, 0);
		pushToHead(pElement);
		//vector subtraction to find distance sun - planet
		vecSub(pSun->m_position, pElement->m_position, v_distancePlanetSun);
		//vector cross product distance and rUp
		vecCrossProduct(v_distancePlanetSun, v_rUp, pElement->m_velocity);
		vecNormalise(pElement->m_velocity, pElement->m_velocity);
		vecScalarProduct(pElement->m_velocity, randFloat(1.0f, 100.0f), pElement->m_velocity);
	}
}

void myInit()
{
	float afGridColour[] = { 0.3f, 0.3f, 0.1f, 0.3f };
	float fog_angle = 0.0f;
	float fogColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

	initMaths();
	//creating the sun
	float v_sunPosition[4];
	vecInitPVec(v_sunPosition);
	v_sunPosition[0] = 0.0f; v_sunPosition[1] = 0.0f; v_sunPosition[2] = 0.0f;
	Data *historySun = 0;
	pSun = createNewElement(v_sunPosition, 80000.0f, g_afCol + 4, historySun, 0);
	pushToHead(pSun);

	//creating a vector to generate a random position
	float v_planetPosition[4];
	vecInitPVec(v_planetPosition);

	//creating velocity vector
	float v_velocity[4];
	vecInitDVec(v_velocity);

	//creating rUp vector based on assumption
	float v_rUp[4];
	vecInitPVec(v_rUp);
	v_rUp[0] = 0.0f; v_rUp[1] = 1.0f; v_rUp[2] = 0.0f;

	//creating distance sun - planet vector
	float v_distancePlanetSun[4];
	vecInitPVec(v_distancePlanetSun);

	//initialise vector to store car - camera position
	vecInitDVec(v_CarCamera);

	for (unsigned int i = 0; i < csg_uiInitialNumSpheres; i++)
	{
		//set flag to true
		g_bAdd = true;
		//add body
		addBody(v_planetPosition, v_velocity, v_rUp, v_distancePlanetSun);
		//set flag to true
		g_bAdd = false;
	}

	camInit(g_Camera);
	camInputInit(g_Input);
	camInputExplore(g_Input, true);
	//gridInit(g_ulGrid, afGridColour, -500, 500, 50.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);

	//create new quadric
	quad = gluNewQuadric();

	//deal with fog
	glFogi(GL_FOG_MODE, GL_EXP2); //set the fog mode to GL_EXP2
	glFogfv(GL_FOG_COLOR, fogColor); //set the fog color 
	glFogf(GL_FOG_DENSITY, g_fFogDensity); //set the density
	glHint(GL_FOG_HINT, GL_NICEST); // set the fog to look the nicest

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//load image to texture the sun
	Image* image = loadBMP("sun.bmp");
	_textureId = loadTexture(image);
	delete image;
}

void writeToTextFile() {
	char filename[60];
	//getting current time
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);
	//creating filename with current time
	strftime(filename, sizeof(filename), "Solar_System_State_%H-%M-%S_%d-%m-%Y.txt", tm);
	//open file to write
	FILE *f = fopen(filename, "w");
	//check if file is null
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	//write solar system info to file
	unsigned int iCount = 0;
	fprintf(f, "Solar System State\n");
	fprintf(f, "----------------------------------------------------------------------------\n");
	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext) {
		fprintf(f, "Number: %d\n\t", iCount++);
		fprintf(f, "X pos: %f, Y pos: %f, Z pos: %f\n\t", pElement->m_position[0], pElement->m_position[1], pElement->m_position[2]);
		fprintf(f, "X vel: %f, Y vel: %f, Z vel: %f\n\t", pElement->m_velocity[0], pElement->m_velocity[1], pElement->m_velocity[2]);
		fprintf(f, "Mass: %f\n\t", pElement->m_mass);
		fprintf(f, "Colour: %f\n", *pElement->m_pcolor);
		fprintf(f, "------------------------------------------------------------------------\n");
	}
	fclose(f);
}

GLfloat r = 1, g = 0, b = 0;
GLfloat xt = 0.0, yt = 0.0, zt = 0.0, xw = 100.0;   /* x,y,z translation */
GLUquadricObj *t;
GLfloat xangle = 0.0, yangle = 0.0, zangle = 0.0, angle = 0.0;   /* axis angles */
float i, theta;


void drawCar() {
	t = gluNewQuadric();
	gluQuadricDrawStyle(t, GLU_FILL);
	
	//f_xCar = cos(f_carTraslationAngle)*radius;
	//f_zCar = sin(f_carTraslationAngle)*radius;
	//glTranslatef(f_xCar, 0.f, f_zCar);
	//glRotatef(redToDeg(f_carTraslationAngle), 0.f, 1.f, 0.f);
	
	glTranslatef(f_carSide, -3.f, f_carForwBack);
	glRotatef(f_carRotAngle, 0.f, 1.f, 0.f);
	glScalef(20.f, 20.f, 20.f);

	glBegin(GL_QUADS);              
	                                             /* OBJECT MODULE*/

	/* top of cube*/
	//************************FRONT BODY****************************************
	
	glColor3f(r, g, b);
	glVertex3f(0.2, 0.4, 0.6);
	glVertex3f(0.6, 0.5, 0.6);
	glVertex3f(0.6, 0.5, 0.2);
	glVertex3f(0.2, 0.4, 0.2);

	/* bottom of cube*/
	glVertex3f(0.2, 0.2, 0.6);
	glVertex3f(0.6, 0.2, 0.6);
	glVertex3f(0.6, 0.2, 0.2);
	glVertex3f(0.2, 0.2, 0.2);

	/* front of cube*/
	glVertex3f(0.2, 0.2, 0.6);
	glVertex3f(0.2, 0.4, 0.6);
	glVertex3f(0.2, 0.4, 0.2);
	glVertex3f(0.2, 0.2, 0.2);

	/* back of cube.*/
	glVertex3f(0.6, 0.2, 0.6);
	glVertex3f(0.6, 0.5, 0.6);
	glVertex3f(0.6, 0.5, 0.2);
	glVertex3f(0.6, 0.2, 0.2);

	/* left of cube*/
	glVertex3f(0.2, 0.2, 0.6);
	glVertex3f(0.6, 0.2, 0.6);
	glVertex3f(0.6, 0.5, 0.6);
	glVertex3f(0.2, 0.4, 0.6);

	/* Right of cube */
	glVertex3f(0.2, 0.2, 0.2);
	glVertex3f(0.6, 0.2, 0.2);
	glVertex3f(0.6, 0.5, 0.2);
	glVertex3f(0.2, 0.4, 0.2);
	//****************************************************************************
	glVertex3f(0.7, 0.65, 0.6);
	glVertex3f(0.7, 0.65, 0.2);
	glVertex3f(1.7, 0.65, 0.2);        //top cover
	glVertex3f(1.7, 0.65, 0.6);
	//***************************back guard******************************
	glColor3f(r, g, b);            /* Set The Color To Blue*/
	glVertex3f(1.8, 0.5, 0.6);
	glVertex3f(1.8, 0.5, 0.2);
	glVertex3f(2.1, 0.4, 0.2);
	glVertex3f(2.1, 0.4, 0.6);

	/* bottom of cube*/
	glVertex3f(2.1, 0.2, 0.6);
	glVertex3f(2.1, 0.2, 0.2);
	glVertex3f(1.8, 0.2, 0.6);
	glVertex3f(1.8, 0.2, 0.6);

	/* back of cube.*/
	glVertex3f(2.1, 0.4, 0.6);
	glVertex3f(2.1, 0.4, 0.2);
	glVertex3f(2.1, 0.2, 0.2);
	glVertex3f(2.1, 0.2, 0.6);

	/* left of cube*/
	glVertex3f(1.8, 0.2, 0.2);
	glVertex3f(1.8, 0.5, 0.2);
	glVertex3f(2.1, 0.4, 0.2);
	glVertex3f(2.1, 0.2, 0.2);

	/* Right of cube */
	glVertex3f(1.8, 0.2, 0.6);
	glVertex3f(1.8, 0.5, 0.6);
	glVertex3f(2.1, 0.4, 0.6);
	glVertex3f(2.1, 0.2, 0.6);
	//******************MIDDLE BODY************************************
	glVertex3f(0.6, 0.5, 0.6);
	glVertex3f(0.6, 0.2, 0.6);
	glVertex3f(1.8, 0.2, 0.6);
	glVertex3f(1.8, 0.5, 0.6);

	/* bottom of cube*/
	glVertex3f(0.6, 0.2, 0.6);
	glVertex3f(0.6, 0.2, 0.2);
	glVertex3f(1.8, 0.2, 0.2);
	glVertex3f(1.8, 0.2, 0.6);

	/* back of cube.*/
	glVertex3f(0.6, 0.5, 0.2);
	glVertex3f(0.6, 0.2, 0.2);
	glVertex3f(1.8, 0.2, 0.2);
	glVertex3f(1.8, 0.5, 0.2);
	//*********************ENTER WINDOW**********************************
	glColor3f(0.3, 0.3, 0.3);
	glVertex3f(0.77, 0.63, 0.2);
	glVertex3f(0.75, 0.5, 0.2);        //quad front window
	glVertex3f(1.2, 0.5, 0.2);
	glVertex3f(1.22, 0.63, 0.2);

	glVertex3f(1.27, 0.63, .2);
	glVertex3f(1.25, 0.5, 0.2);        //quad back window
	glVertex3f(1.65, 0.5, 0.2);
	glVertex3f(1.67, 0.63, 0.2);

	glColor3f(r, g, b);
	glVertex3f(0.7, 0.65, 0.2);
	glVertex3f(0.7, 0.5, .2);       //first separation
	glVertex3f(0.75, 0.5, 0.2);
	glVertex3f(0.77, 0.65, 0.2);

	glVertex3f(1.2, 0.65, 0.2);
	glVertex3f(1.2, 0.5, .2);       //second separation
	glVertex3f(1.25, 0.5, 0.2);
	glVertex3f(1.27, 0.65, 0.2);

	glVertex3f(1.65, 0.65, 0.2);
	glVertex3f(1.65, 0.5, .2);     //3d separation
	glVertex3f(1.7, 0.5, 0.2);
	glVertex3f(1.7, 0.65, 0.2);

	glVertex3f(0.75, 0.65, 0.2);
	glVertex3f(0.75, 0.63, 0.2);        //line strip
	glVertex3f(1.7, 0.63, 0.2);
	glVertex3f(1.7, 0.65, 0.2);

	glVertex3f(0.75, 0.65, 0.6);
	glVertex3f(0.75, 0.63, 0.6);        //line strip
	glVertex3f(1.7, 0.63, 0.6);
	glVertex3f(1.7, 0.65, 0.6);

	glColor3f(0.3, 0.3, 0.3);
	glVertex3f(0.77, 0.63, 0.6);
	glVertex3f(0.75, 0.5, 0.6);        //quad front window
	glVertex3f(1.2, 0.5, 0.6);
	glVertex3f(1.22, 0.63, 0.6);

	glVertex3f(1.27, 0.63, .6);
	glVertex3f(1.25, 0.5, 0.6);        //quad back window
	glVertex3f(1.65, 0.5, 0.6);
	glVertex3f(1.67, 0.63, 0.6);

	glColor3f(r, g, b);
	glVertex3f(0.7, 0.65, 0.6);
	glVertex3f(0.7, 0.5, .6);       //first separation
	glVertex3f(0.75, 0.5, 0.6);
	glVertex3f(0.77, 0.65, 0.6);

	glVertex3f(1.2, 0.65, 0.6);
	glVertex3f(1.2, 0.5, .6);       //second separation
	glVertex3f(1.25, 0.5, 0.6);
	glVertex3f(1.27, 0.65, 0.6);

	glVertex3f(1.65, 0.65, 0.6);
	glVertex3f(1.65, 0.5, .6);
	glVertex3f(1.7, 0.5, 0.6);
	glVertex3f(1.7, 0.65, 0.6);
	glEnd();


	//**************************************************************
	glBegin(GL_QUADS);

	/* top of cube*/
	glColor3f(0.3, 0.3, 0.3);
	glVertex3f(0.6, 0.5, 0.6);
	glVertex3f(0.6, 0.5, 0.2);        //quad front window
	glVertex3f(0.7, 0.65, 0.2);
	glVertex3f(0.7, 0.65, 0.6);

	glVertex3f(1.7, 0.65, .6);
	glVertex3f(1.7, 0.65, 0.2);        //quad back window
	glVertex3f(1.8, 0.5, 0.2);
	glVertex3f(1.8, 0.5, 0.6);

	glEnd();


	/* ROAD
	glPushMatrix();
	glTranslatef(xw, 0, 0);
	glColor3f(0.5, 0.2, 0.3);
	glBegin(GL_QUADS);
	
	
	for (i = 0; i<200; i += 0.2)
	{
	glVertex3f(-100 + i, 0, 1);
	glVertex3f(-99.9 + i, 0, 1);
	glVertex3f(-99.9 + i, 0.2, 1);
	glVertex3f(-100 + i, 0.2, 1);
	i += 0.5;
	}
	for (i = 0; i<200; i += 0.2)
	{
	glVertex3f(-100 + i, 0, 0);
	glVertex3f(-99.9 + i, 0, 0);
	glVertex3f(-99.9 + i, 0.2, 0);
	glVertex3f(-100 + i, 0.2, 0);
	i += 0.5;
	}
	
	glEnd();
	glPopMatrix();
	*/

	//*************************************************************************************************
	glBegin(GL_TRIANGLES);                /* start drawing the cube.*/

	 									  /* top of cube*/
	glColor3f(float(34/255), float(113/255), float(179/255));
	
	glVertex3f(0.6, 0.5, 0.6);
	glVertex3f(0.7, 0.65, 0.6);       //tri front window
	glVertex3f(0.7, 0.5, 0.6);

	glVertex3f(0.6, 0.5, 0.2);
	glVertex3f(0.7, 0.65, 0.2);       //tri front window
	glVertex3f(0.7, 0.5, 0.2);

	glVertex3f(1.7, 0.65, 0.2);
	glVertex3f(1.8, 0.5, 0.2);       //tri back window
	glVertex3f(1.7, 0.5, 0.2);

	glVertex3f(1.7, 0.65, 0.6);
	glVertex3f(1.8, 0.5, 0.6);       //tri back window
	glVertex3f(1.7, 0.5, 0.6);

	glEnd();

	/************IGNITION SYSTEM**********************************/
	glPushMatrix();
	glColor3f(0.7, 0.7, 0.7);
	//glTranslatef(1.65, 0.2, 0.3);
	glTranslatef(f_carIngnitionDisplacement, 0.2, 0.3);
	glRotatef(90.0, 0, 1, 0);
	gluCylinder(t, 0.02, 0.03, .5, 10, 10);
	glPopMatrix();
	/********************WHEEL*********************************************/

	glColor3f(0.7, 0.7, 0.7);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);


	glTranslatef(0.f, 0.f, 0.f);

	for (theta = 0; theta<360; theta = theta + 20)
	{
		glVertex3f(0.6, 0.2, 0.62);
		glVertex3f(0.6 + (0.08*(cos(((theta + angle)*3.14) / 180))), 0.2 + (0.08*(sin(((theta + angle)*3.14) / 180))), 0.62);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (theta = 0; theta<360; theta = theta + 20)
	{
		glVertex3f(0.6, 0.2, 0.18);
		glVertex3f(0.6 + (0.08*(cos(((theta + angle)*3.14) / 180))), 0.2 + (0.08*(sin(((theta + angle)*3.14) / 180))), 0.18);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (theta = 0; theta<360; theta = theta + 20)
	{
		glVertex3f(1.7, 0.2, 0.18);
		glVertex3f(1.7 + (0.08*(cos(((theta + angle)*3.14) / 180))), 0.2 + (0.08*(sin(((theta + angle)*3.14) / 180))), 0.18);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (theta = 0; theta<360; theta = theta + 20)
	{
		glVertex3f(1.7, 0.2, 0.62);
		glVertex3f(1.7 + (0.08*(cos(((theta + angle)*3.14) / 180))), 0.2 + (0.08*(sin(((theta + angle)*3.14) / 180))), 0.62);
	}
	glEnd();


	//glRotatef(index, 0.f, 0.f, 1.f);
	
	glTranslatef(0.6, 0.2, 0.6);
	glColor3f(0, 0, 0);
	glutSolidTorus(0.025, 0.07, 10, 25);

	glTranslatef(0, 0, -0.4);
	glutSolidTorus(0.025, 0.07, 10, 25);

	glTranslatef(1.1, 0, 0);
	glutSolidTorus(0.025, 0.07, 10, 25);

	glTranslatef(0, 0, 0.4);
	glutSolidTorus(0.025, 0.07, 10, 25);

	glPopMatrix();
}


int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1400, 800);
	glutCreateWindow("Interactive Graphics Assignment");

	myInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(sKeyboard);
	glutSpecialUpFunc(sKeyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();
	return 0;
}
