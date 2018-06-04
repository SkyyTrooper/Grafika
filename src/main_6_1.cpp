#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>


GLuint programColor;
GLuint programTexture;

Core::Shader_Loader shaderLoader;

float appLoadingTime;

obj::Model shipModel;
obj::Model sphereModel;
float cameraAngley = 0;
float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -10.0f, 0.0f));

GLuint textureEarth;
GLuint textureAsteroid;

static const int NUM_ASTEROIDS = 8;
glm::vec3 asteroidPositions[NUM_ASTEROIDS];
static const int NUM_CAMERA_POINTS = 10;
glm::vec3 cameraKeyPoints[NUM_CAMERA_POINTS];
float fish[NUM_ASTEROIDS];

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'f': cameraAngley -= angleSpeed; break;
	case 'r': cameraAngley += angleSpeed; break;
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	}
}

glm::mat4 createCameraMatrix()
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;

	// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	cameraDir = glm::vec3(cosf(cameraAngle), sinf(cameraAngley), sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0,1,0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture2(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId, glm::mat4 transformationMatrix)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * transformationMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}



void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	// (Bardziej elegancko byloby przekazac je jako argumenty do funkcji, ale robimy tak dla uproszczenia kodu.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));



	for (int i = 0; i < NUM_ASTEROIDS; i++)
	{
		float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

		glm::mat4 rotation2;
		int move = i*fish[i];
		///////
		rotation2[0][0] = cos(time+move); /////+
		rotation2[2][0] = -sin(time+move); /////+
		rotation2[0][2] = sin(time+move); /////-
		rotation2[2][2] = cos(time+move); ////+


		glm::mat4 transformation2;
		transformation2[3][1] = fish[i];
		transformation2[3][0] = -8.0;
		transformation2[3][2] = -5.0;

		glm::mat4 transformation;
		transformation = transformation*rotation2*transformation2;
		drawObjectTexture2(&sphereModel, glm::translate(asteroidPositions[i]) * glm::scale(glm::vec3(1,1,1)), textureAsteroid, transformation);
	}

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	glm::mat4 rotation2;
	///////
	rotation2[0][0] = cos(time); /////+
	rotation2[2][0] = -sin(time); /////+
	rotation2[0][2] = sin(time); /////-
	rotation2[2][2] = cos(time); ////+

	glm::mat4 transformation2;
	transformation2[3][1] = 8.5;
	transformation2[3][0] = -8.0;
	transformation2[3][2] = -5.0;

	glm::mat4 transformation;
	transformation = transformation*rotation2*transformation2;
	drawObjectTexture2(&sphereModel, glm::translate(glm::vec3(0, 0, 0)), textureEarth, transformation);
	//////////////

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sphereModel = obj::loadModelFromFile("models/wraisse_main.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	textureEarth = Core::LoadTexture("textures/scale1.png");
	textureAsteroid = Core::LoadTexture("textures/scale1.png");

	static const float astRadius = 6.0;
	for (int i = 0; i < NUM_ASTEROIDS; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / NUM_ASTEROIDS);
		asteroidPositions[i] = glm::vec3(cosf(angle), 0.0f, sinf(angle)) * astRadius + glm::sphericalRand(0.5f);
	}

	static const float camRadius = 3.55;
	static const float camOffset = 0.6;
	for (int i = 0; i < NUM_CAMERA_POINTS; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / NUM_CAMERA_POINTS);
		float radius = camRadius *(0.95 + glm::linearRand(0.0f, 0.1f));
		cameraKeyPoints[i] = glm::vec3(cosf(angle) + camOffset, 0.0f, sinf(angle)) * radius;
	}

	appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}



int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	for (int i = 0; i < NUM_ASTEROIDS; i++) {
		float LO = -8.5;
		float HI = 8.5;
		float r3 = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
		fish[i] = r3;
	}
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);


	glutMainLoop();

	shutdown();

	return 0;
}
