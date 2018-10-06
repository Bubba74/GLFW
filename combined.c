
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <camera.h>
#include <linmath.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Shader.h"

int WIN_X = 1000;
int WIN_Y = 200;
int WIN_WIDTH = 800;
int WIN_HEIGHT = 600;

unsigned int loadTexture(char *);

void setViewPort();
void loadShaders(unsigned int, const char *, const char *);

void framebuffer_size_callback (GLFWwindow*, int, int);
void window_pos_callback (GLFWwindow*, int, int);

void processInput (GLFWwindow*);

float c_t = 512/512.0f; //c_t
float c_b = 0/512.0f; //c_btom
float c_l = 0/1024.0f; //c_l
float c_r = 512/1024.0f; //c_r

int main(){

	float vertices[] = {
		-1.0, -1.0,   0.5,0, //BL
		-1.0, 1.0,    0.5,1,   //TL
		1.0, 1.0,     1,1,   //TR

		1.0, 1.0,     1,1,   //TR
		1.0, -1.0,     1,0,   //BR
		-1.0, -1.0,   0.5,0 //BL
	};

	float crateVertices[] = {
			-0.5f, -0.5f, -0.5f,  c_l, c_b,
			 0.5f, -0.5f, -0.5f,  c_r, c_b,
			 0.5f,  0.5f, -0.5f,  c_r, c_t,
			 0.5f,  0.5f, -0.5f,  c_r, c_t,
			-0.5f,  0.5f, -0.5f,  c_l, c_t,
			-0.5f, -0.5f, -0.5f,  c_l, c_b,

			-0.5f, -0.5f,  0.5f,  c_l, c_b,
			 0.5f, -0.5f,  0.5f,  c_r, c_b,
			 0.5f,  0.5f,  0.5f,  c_r, c_t,
			 0.5f,  0.5f,  0.5f,  c_r, c_t,
			-0.5f,  0.5f,  0.5f,  c_l, c_t,
			-0.5f, -0.5f,  0.5f,  c_l, c_b,

			-0.5f,  0.5f,  0.5f,  c_r, c_b,
			-0.5f,  0.5f, -0.5f,  c_r, c_t,
			-0.5f, -0.5f, -0.5f,  c_l, c_t,
			-0.5f, -0.5f, -0.5f,  c_l, c_t,
			-0.5f, -0.5f,  0.5f,  c_l, c_b,
			-0.5f,  0.5f,  0.5f,  c_r, c_b,

			 0.5f,  0.5f,  0.5f,  c_r, c_b,
			 0.5f,  0.5f, -0.5f,  c_r, c_t,
			 0.5f, -0.5f, -0.5f,  c_l, c_t,
			 0.5f, -0.5f, -0.5f,  c_l, c_t,
			 0.5f, -0.5f,  0.5f,  c_l, c_b,
			 0.5f,  0.5f,  0.5f,  c_r, c_b,

			-0.5f, -0.5f, -0.5f,  c_l, c_t,
			 0.5f, -0.5f, -0.5f,  c_r, c_t,
			 0.5f, -0.5f,  0.5f,  c_r, c_b,
			 0.5f, -0.5f,  0.5f,  c_r, c_b,
			-0.5f, -0.5f,  0.5f,  c_l, c_b,
			-0.5f, -0.5f, -0.5f,  c_l, c_t,

			-0.5f,  0.5f, -0.5f,  c_l, c_t,
			 0.5f,  0.5f, -0.5f,  c_r, c_t,
			 0.5f,  0.5f,  0.5f,  c_r, c_b,
			 0.5f,  0.5f,  0.5f,  c_r, c_b,
			-0.5f,  0.5f,  0.5f,  c_l, c_b,
			-0.5f,  0.5f, -0.5f,  c_l, c_t
	};

	// glfw initialization
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// window init
	GLFWwindow *window = glfwCreateWindow (WIN_WIDTH, WIN_HEIGHT, "OpenGL Window\n", NULL, NULL);
	if (window == NULL){
		printf("Failed to create OpenGL window!\n");
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, WIN_X, WIN_Y);
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);

	// GLAD init
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	setViewPort();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// --- Shader Programs --- //
	Shader *shader = getShaderObject();
	shader->loadShader(shader, "res/tile_shader.vs", "res/tile_shader.fs");

	Shader *crateShader = getShaderObject();
	crateShader->loadShader(crateShader, "res/vertex_shader.vs.c", "res/fragment_shader.vs.c");

	// --- Vertex Array Buffers --- //
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	unsigned int crateVAO;
	glGenVertexArrays(1, &crateVAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	unsigned int crateVBO;
	glGenBuffers(1, &crateVBO);

	//Setup VAO for left triangle
	glBindVertexArray(VAO);
	//Bind array buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//Inform VBO of vertices format
	int stride = 4*sizeof(float);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (2*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	//Setup VAO for crate
	glBindVertexArray(crateVAO);

	glBindBuffer(GL_ARRAY_BUFFER, crateVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crateVertices), crateVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	unsigned int tileTexture = loadTexture("textures/map.png");

	unsigned int crateTexture = loadTexture("textures/map.png");
	unsigned int faceTexture = loadTexture("textures/awesome.png");


	shader->use(shader);
	shader->setInt(shader, "tileTexture", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tileTexture);


	crateShader->use(crateShader);
	crateShader->setInt(crateShader, "crate", 0);
	crateShader->setInt(crateShader, "face", 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, crateTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, faceTexture);


	unsigned int localLoc = glGetUniformLocation(crateShader->ID, "local");
	unsigned int modelLoc = glGetUniformLocation(crateShader->ID, "model");
	unsigned int viewLoc = glGetUniformLocation(crateShader->ID, "view");
	unsigned int perspectiveLoc = glGetUniformLocation(crateShader->ID, "perspective");


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);


	//Variables
	float x = 0, y = 0, z = 0;
	float theta = 0;

	// struct rot3d {
	// 	float pitch, roll, yaw;
	// } cam = {0,0,-0.9};

	double cam_prevx = WIN_WIDTH/2;
	double cam_prevy = WIN_HEIGHT/2;
	glfwGetCursorPos(window, &cam_prevx, &cam_prevy);
	double camx, camy;

	// vec3 camPos = {0, 1, 2};
	// vec3 camDir = {0, 0, -1};
	setPosition3d(0, 1, 2);
	setRotation3d(0, 0, 0);

	glfwSetTime(0);
	glClearColor(0.2f, 0.3f, 0.3, 1.0);
	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ------------------ Process Input --------------------- //
		if (glfwGetKey(window, GLFW_KEY_LEFT)) theta += 0.01;
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) theta -= 0.01;

		if (glfwGetKey(window, GLFW_KEY_UP)){
			x += cosf(-theta)/100;
			y += sinf(-theta)/100;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN)){
			x -= cosf(-theta)/100;
			y -= sinf(-theta)/100;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER)){
			z += 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)){
			z -= 0.01f;
		}

		int keyW = glfwGetKey(window, GLFW_KEY_W);
		int keyA = glfwGetKey(window, GLFW_KEY_A);
		int keyS = glfwGetKey(window, GLFW_KEY_S);
		int keyD = glfwGetKey(window, GLFW_KEY_D);
		int keyLeftShift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
		int keySpace = glfwGetKey(window, GLFW_KEY_SPACE);

		//Scale direction vector to appropriate speed
		double speedForward = 0.08;
		double speedStrafe = 0.07;
		double speedRaise = 0.1;		//Positive for up

		if (!(keyW && keyS))
			if (keyW) moveStraight(speedForward);
			else if (keyS) moveStraight(-speedForward);

		if (!(keyA && keyD))
			if (keyA) moveStrafe(-speedStrafe);
			else if (keyD) moveStrafe(speedStrafe);

		if (!(keySpace && keyLeftShift))
			if (keySpace) moveVertical(speedRaise);
			else if (keyLeftShift) moveVertical(-speedRaise);


		glfwGetCursorPos(window, &camx, &camy);
		updateRotation(camx-cam_prevx, camy-cam_prevy);

		generateViewMatrix();
		//------------- Calculate Matrices --------------------- //

		mat4x4 perspective, viewTimesPerspective;


		//Perspective
		mat4x4_perspective(perspective, M_PI/3, (float)WIN_WIDTH/(float)WIN_HEIGHT, 0.1f, 100.0f);

		//Product
		mat4x4_mul(viewTimesPerspective, perspective, Camera.viewMatrix);


		// ------------------- Render Crate -------------------- //

		mat4x4 crateLocal, crateModel;
		mat4x4_identity(crateModel);
		mat4x4_identity(crateLocal);//rot
		mat4x4_translate_in_place(crateLocal, x, -y, z); //translate
		mat4x4_rotate_Z(crateLocal, crateLocal, theta); //output, rot


		glUseProgram(crateShader->ID);

		//Send matrix transformations to shader program
		glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)Camera.viewMatrix);
		glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, crateTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, faceTexture);

		glBindVertexArray(crateVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);



		// Bind Crate Shader Program
		glUseProgram(shader->ID);

		unsigned int tile_vpLoc = glGetUniformLocation(shader->ID, "vp");
		glUniformMatrix4fv(tile_vpLoc, 1, GL_FALSE, (const GLfloat *) viewTimesPerspective);

		//Send matrix transformations to shader program
		// glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);
		// glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
		// glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)cameraView);
		// glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);

		//Bind proper data
		glBindVertexArray(VAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tileTexture);

		//Draw main crate
		// glDrawArrays(GL_TRIANGLES, 0, 6);

		unsigned int localLoc = glGetUniformLocation(shader->ID, "local");

		mat4x4 local;
		mat4x4_identity(local);

		int i,j;
		for (i=-10; i<10; i++){
			for (j=-10; j<10; j++){
				mat4x4_translate(local, 2*i, 0, 2*j);
				glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)local);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}



		// int i;
		// for (i=0; i<6; i++){
		// 	mat4x4_translate(crateModel, tiles[i*2], tiles[i*2+1], 0);
		// 	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
		// 	glDrawArrays(GL_TRIANGLES, 0, 6);
		// }


		//Draw origin crate
		// mat4x4_translate_in_place(crateModel, -x, y, -z);
		// glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
		// glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		// */
		// ----------------Draw Objects --------------------//

		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// glBindVertexArray(VAO_right);
		// glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// glBindVertexArray(0);


		//GLFW Update Ticks
		glfwPollEvents();
		processInput(window);
		glfwSwapBuffers(window);

		cam_prevx = camx;
		cam_prevy = camy;

	}

	glfwTerminate();
	return 0;

}//main

void setViewPort(){
	// set OpenGL viewport
	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
}

void processInput (GLFWwindow* win){

	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(win, 1);

}//processInput

void framebuffer_size_callback (GLFWwindow* win, int width, int height){
	WIN_WIDTH = width;
	WIN_HEIGHT = height;
}
