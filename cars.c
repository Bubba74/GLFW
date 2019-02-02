
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

float x = 0, y = 0;
float theta = 0;

//									//black		//blue		//green   //cyan     //red   //magenta //yellow  //white
float colors[8][3] = {{0,0,0} , {0,0,1} , {0,1,0} , {0,1,1} , {1,0,0} , {1,0,1} , {1,1,0} , {1,1,1}};

typedef struct Car {
	float x, y, theta;
	float *rgb;
} Car;

float randFloat(){
	return ((float)rand())  /  ((float)RAND_MAX);
}

void my_mat4x4_scale(mat4x4 out, mat4x4 in, float sf){
	int i,j;
	for (i=0; i<3; i++)
		for (j=0; j<3; j++)
			out[i][j] = sf*in[i][j];
}

int main(){

	// float vertices[] = {
	// 		//coords       colors      tex-coords
	// 	 0.9, 0.3,    1.0, 0.0, 0.0,    0, 1.0,//Top Right
	// 	 0.8,-0.3,    0.0, 1.0, 0.0,    1.0, 0.0,//Bottom Right
	// 	-0.9, 0.4,    0.0, 0.0, 1.0,    0.0, 1.0,//Top Left
	// 	-0.8,-0.3,    1.0, 1.0, 1.0,    0.0, 0.0//Bottom Left
	// };

	float vertices[] = {
		//coords			//tex-coords
		 1.0, 0.5,		1, 1, //Top Right
		-1.0, 0.5,		0, 1, //Top left
		 1.0,-0.5,		1, 0, //Bottom Right
		-1.0,-0.5,		0, 0  //Bottom Left
	};

	int rectangle[] = {
		0, 1, 2,
		1, 3, 2 };


	srand(45);

	Car cars[8];
	int i;
	for (i=0; i<8; i++){
		cars[i].x = 2*randFloat()-1;
		cars[i].y = 2*randFloat()-1;
		cars[i].theta = 6.28*randFloat();

		cars[i].rgb = colors[i];
	}


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

	// --- Shader Programs --- //
	Shader *shader = getShaderObject();
	shader->loadShader(shader, "res/cars.vs.c", "res/cars.fs.c");

	// --- Vertex Array Buffers --- //
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	// glGenVertexArrays(1, &VAO_right);

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	// glGenBuffers(1, &EBO_right);

	//Setup VAO for left triangle
	glBindVertexArray(VAO);
	//Bind array buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//Bind EBO for left triangle
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);
	//Inform VBO of vertices format
	int stride = 4*sizeof(float);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (2*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	unsigned int crateTexture = loadTexture("textures/UncoloredCar.png");
	glBindTexture(GL_TEXTURE_2D, crateTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);


	shader->use(shader);

	glBindTexture(GL_TEXTURE_2D, crateTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float zoom = 0.1;
	glfwSetTime(0);
	glClearColor(0.2f, 0.3f, 0.3, 1.0f);
	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT);

		/* Render */
		glUseProgram(shader->ID);
		glBindVertexArray(VAO);

		mat4x4 idnty, rot, scaled, translate, output;
		for (i=0; i<8; i++){

			mat4x4_identity(scaled);
			mat4x4_identity(idnty);
			mat4x4_rotate_Z(rot, idnty, cars[i].theta);
			// mat4x4_scale(scaled, rot, 0.5);
			my_mat4x4_scale(scaled, rot, zoom);

			mat4x4_translate(translate, cars[i].x, -cars[i].y, 0);
			mat4x4_mul(output, translate, scaled);

			unsigned int transLoc = glGetUniformLocation(shader->ID, "transform");
			glUniformMatrix4fv(transLoc, 1, GL_FALSE, (const GLfloat *)output);
			unsigned int colorLoc = glGetUniformLocation(shader->ID, "carColor");
			glUniform3fv(colorLoc, 1, (const GLfloat *)cars[i].rgb);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		}

		// if (glfwGetKey(window, GLFW_KEY_EQUAL)) mix_val += 0.01;
		// else if (glfwGetKey(window, GLFW_KEY_MINUS)) mix_val -= 0.01;
		// shader->setFloat(shader, "mix_val", mix_val);
		//
		if (glfwGetKey(window, GLFW_KEY_LEFT)) zoom *= 5.0/6;
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) zoom *= 1.2;
		//
		// if (glfwGetKey(window, GLFW_KEY_UP)){
		// 		x += cosf(-theta)/100;
		// 		y += sinf(-theta)/100;
		// }
		// if (glfwGetKey(window, GLFW_KEY_DOWN)){
		// 		x -= cosf(-theta)/100;
		// 		y -= sinf(-theta)/100;
		// }


		// glBindVertexArray(VAO_right);
		// glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		//GLFW Update Ticks
		glfwPollEvents();
		processInput(window);
		glfwSwapBuffers(window);
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
