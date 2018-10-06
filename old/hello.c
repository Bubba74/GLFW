
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


int main(){

	float c_t = 512/512.0f; //c_t
	float c_b = 0/512.0f; //c_btom
	float c_l = 0/1024.0f; //c_l
	float c_r = 512/1024.0f; //c_r

	float vertices[] = {
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

	int rectangle[] = {
		0, 1, 2,
		0, 3, 2
	};

	float tiles[] = {
		0,0,
		1,0,
		2,0,
		3,0,
		4,0,
		5,0
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


	// --- Shader Programs --- //
	Shader *shader = getShaderObject();
	shader->loadShader(shader, "res/vertex_shader.vs.c", "res/fragment_shader.vs.c");

	// --- Vertex Array Buffers --- //
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	// unsigned int VBO2;
	// glGenBuffers(1, &VBO2);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	//Setup VAO for left triangle
	glBindVertexArray(VAO);
	//Bind array buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//Bind EBO for left triangle
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);
	//Inform VBO of vertices format
	int stride = 5*sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	unsigned int crateTexture = loadTexture("textures/map.png");
	unsigned int faceTexture = loadTexture("textures/awesome.png");

	shader->use(shader);
	shader->setInt(shader, "crate", 0);
	shader->setInt(shader, "face",  1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, crateTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, faceTexture);

	unsigned int localLoc = glGetUniformLocation(shader->ID, "local");
	unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
	unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
	unsigned int perspectiveLoc = glGetUniformLocation(shader->ID, "perspective");


// //   ---- Second VAO ----
//
// 	//Setup VAO for right triangle
// 	glBindVertexArray(VAO_right);
// 	//Bind array buffer
// 	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
// 	glBufferData(GL_ARRAY_BUFFER, sizeof(tile_verticc_bes), tile_vertices, GL_STATIC_DRAW);
//
// 	//Inform VBO of vertices format
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
// 	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3*sizeof(float)));
// 	glEnableVertexAttribArray(0);
// 	glEnableVertexAttribArray(1);
//
// 	unsigned int tile_texture = loadTexture("textures/tile.png");

//   -------------------- //

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	float mix_val = 0;

	double prev_curx, prev_cury;
	glfwGetCursorPos(window, &prev_curx, &prev_cury);
	double curx, cury;

	float x = 0, y = 0, z = 0;
	float theta = 0;


	glfwSetTime(0);
	glClearColor(0.2f, 0.3f, 0.3, c_t);
	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ------------------ Process Input --------------------- //

		if (glfwGetKey(window, GLFW_KEY_EQUAL)) mix_val += 0.01;
		else if (glfwGetKey(window, GLFW_KEY_MINUS)) mix_val -= 0.01;
		shader->setFloat(shader, "mix_val", mix_val);

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
		if (glfwGetKey(window, GLFW_KEY_W)){
			z += 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_S)){
			z -= 0.01f;
		}

		glfwGetCursorPos(window, &curx, &cury);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
			// cam_yaw -= (curx-prev_curx)/10.f;
			// printf("Yaw: %.2f\n", cam_yaw);

		}


		// --------------- Calculate Matrices ---------------- //

		mat4x4 cameraView, perspective, viewTimesPerspective;

		//Camera view
		double camX = 3*sin(glfwGetTime()/3);
		double camZ = 3*cos(glfwGetTime()/3);

		mat4x4_identity(cameraView);
		vec3 camPos = {camX, 1, camZ },
				 origin = {0, 0, 0   },
				 up			= {0, 1, 0   };
		mat4x4_look_at(cameraView, camPos, origin, up);

		//Perspective
		mat4x4_perspective(perspective, M_PI/3, (float)WIN_WIDTH/(float)WIN_HEIGHT, 0.1f, 100.0f);

		//Product
		mat4x4_mul(viewTimesPerspective, perspective, cameraView);


		// ------------------- Render Crate -------------------- //

		// /*
		mat4x4 crateLocal, crateModel;
		mat4x4_identity(crateModel);
		mat4x4_identity(crateLocal);//rot
		mat4x4_translate_in_place(crateLocal, x, -y, z); //translate
		mat4x4_rotate_Z(crateLocal, crateLocal, theta); //output, rot

		// Bind Crate Shader Program
		glUseProgram(shader->ID);

		//Send matrix transformations to shader program
		glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)cameraView);
		glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);

		//Bind proper data
		glBindVertexArray(VAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, crateTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, faceTexture);

		//Draw main crate
		glDrawArrays(GL_TRIANGLES, 0, 36);

		int i;
		for (i=0; i<6; i++){
			mat4x4_translate(crateModel, tiles[i*2], tiles[i*2+1], 0);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


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

		prev_curx = curx;
		prev_cury = cury;
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
