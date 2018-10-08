
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <camera.h>
#include <linmath.h>
#include <curl/curl.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// #include <textures.h>

#include "Shader.h"

// int WIN_X = 1000;
// int WIN_Y = 200;
// int WIN_WIDTH = 800;
// int WIN_HEIGHT = 600;

int WIN_X = 0;
int WIN_Y = 0;
int WIN_WIDTH = 1920;
int WIN_HEIGHT = 1080;

unsigned char *stbi_load_from_memory(char *data, int len, int *, int *, int *, int);

void setViewPort();
void loadShaders(unsigned int, const char *, const char *);

void framebuffer_size_callback (GLFWwindow*, int, int);
void window_pos_callback (GLFWwindow*, int, int);

void processInput (GLFWwindow*);

float c_t = 512/512.0f; //c_t
float c_b = 0/512.0f; //c_btom
float c_l = 0/1024.0f; //c_l
float c_r = 512/1024.0f; //c_r

struct frame {
	float x, y, z;
	float width, height;
};

#define IMAGE_MAX_SIZE 100000
struct image {
	unsigned int size;
	char *data;
} Image;

#define side1 0
#define side2 8
#define side3 16
#define side4 24

CURL *fetchFrame;
unsigned int cameraTexture, newFrameReady = 0;
unsigned int width, height, nrChannels;

size_t curlWrite(char *data, size_t one, size_t nmemb, void *userPtr){
	struct image *outputImage = (struct image*)userPtr;

	if (outputImage->size+nmemb > IMAGE_MAX_SIZE)
		printf("Trying to load a file too big for the buffer! %dbytes\n", outputImage->size+(int)nmemb);

	int i;
	for (i=0; i<nmemb; i++){
		outputImage->data[outputImage->size++] = data[i];
	}
	return nmemb;
}

void loadCameraTexture(){
		//Wait for an available frame
		if (!newFrameReady)
					return;

		glDeleteTextures(1, &cameraTexture);

		glGenTextures(1, &cameraTexture);
	  glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, cameraTexture);

	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	  GLint inputFormat = GL_RGB;
	  if (nrChannels == 4){
	    inputFormat = GL_RGBA;
	  }
		unsigned char *imageData = stbi_load_from_memory(Image.data, Image.size, &width, &height, &nrChannels, 0);

	  glTexImage2D(GL_TEXTURE_2D, 0, inputFormat, width, height, 0, inputFormat, GL_UNSIGNED_BYTE, imageData);

		stbi_image_free(imageData);
		//Request new frame from camera thread
		newFrameReady = 0;

}//loadCameraImage

double lastTime = 0;
int frames = 0;
void fetchCameraImage(){

	//Don't read new frames if the old one is still there
	if (newFrameReady)
		return;

// if (glfwGetTime()-lastTime < 0.1) return;


	//Get Camera image
	Image.size = 0; //Reset buffer to index 0

	//Loads Camera frame into Image.data
	CURLcode curlCode = curl_easy_perform(fetchFrame);

	if (curlCode != CURLE_OK){
		printf("Curl fetch failed\n%s", curl_easy_strerror(curlCode));
	} else {

			newFrameReady = 1;

		frames++;

		double now = glfwGetTime();
		if (now-lastTime > 1){
			printf("FPS: %d\n", frames);
			frames = 0;
			lastTime = now;
		}

	}
}//loadNewFrame

void *loadFramesLoop(void *param){
	//Global CURL *
	curl_global_init(CURL_GLOBAL_ALL);

	char *URL = "169.232.114.129:8080/shot.jpg";
	//Global Image struct
	Image.size = 0;
	Image.data = malloc(IMAGE_MAX_SIZE);//Allocate 100K for frame

	//Setup fetch mechanice for camera frame
	fetchFrame = curl_easy_init();

	if (!fetchFrame) printf("Failed to initialize curl!");

	curl_easy_setopt(fetchFrame, CURLOPT_URL, URL);
	curl_easy_setopt(fetchFrame, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(fetchFrame, CURLOPT_TCP_NODELAY, 1);
	curl_easy_setopt(fetchFrame, CURLOPT_BUFFERSIZE, 100000L);

	curl_easy_setopt(fetchFrame, CURLOPT_WRITEFUNCTION, curlWrite);
	curl_easy_setopt(fetchFrame, CURLOPT_WRITEDATA, &Image);

	while (!glfwWindowShouldClose((GLFWwindow *)param)){
		fetchCameraImage();
	}
	return NULL;
}//loadFramesLoop

int buildingSideIndices[] = {
	side1+0, side1+1, side1+2,   side1+1, side1+2, side1+3,
	side1+4, side1+5, side1+6,   side1+5, side1+6, side1+7,

	side2+0, side2+1, side2+2,   side2+1, side2+2, side2+3,
	side2+4, side2+5, side2+6,   side2+5, side2+6, side2+7,

	side3+0, side3+1, side3+2,   side3+1, side3+2, side3+3,
	side3+4, side3+5, side3+6,   side3+5, side3+6, side3+7,

	side4+0, side4+1, side4+2,   side4+1, side4+2, side4+3,
	side4+4, side4+5, side4+6,   side4+5, side4+6, side4+7

};

unsigned int getBuildingVAO(struct frame building){
	//4 sides
	//top = 4 points (each w/ texture vertex);
	//base = 4 points (each w/ texture vertex);
	//total_space = 4 sides * 8 points = 32 vertices
	//vertex = 3xyz + 2Tex_XY
	int cVertices = 32, cFloats = 5;

	//Data values for the building
	float *vertices = malloc(sizeof(float)*cVertices*cFloats);
	unsigned int vertexWriteIndex = 0;

	//The y-values for each of the rows drawn of the building
	float topPartHeight = building.width/2; //maintain aspect ratio of textures
	float top = building.y+building.height;
	float heights[] = {top, top-topPartHeight, top-topPartHeight, building.y};

	//For each side of the building, we draw left-to-right top-to-bottom
	//since the top of the building occupies the left half of the src image,
	//the left/right pairs of the first two points are 0->0.5
		//				top {   L,   R, L,   R}	  bottom{ L,   R,   L,   R};
	float texXcoords[] = {0, 0.5, 0, 0.5,					0.5, 1.0, 0.5, 1.0};
		//For the top part(i=0,1), the entire height of the texture fits the side
		//for the bottom, we repeat the bottom a bunch to adjust for height variability
	float texYcoords[] = {0,1,0,(building.height-topPartHeight)/topPartHeight};

	//Combined, these makeup the four corners of the building
	float xCoords[] = {building.x, building.x+building.width};
	float zCoords[] = {building.z, building.z+building.width};
	//By going from index 0, 1, 2, 3, one travels CCW around the building
	int orderXindices[] = {0, 0, 1, 1};
	int orderZindices[] = {0, 1, 1, 0};

	int leftEdgeIndex;
	for (leftEdgeIndex=0; leftEdgeIndex<4; leftEdgeIndex++){
			//Grab the correct x,z coordinates for the left/right edge of the side
			float leftX  = xCoords[orderXindices[leftEdgeIndex]];
			float leftZ  = zCoords[orderZindices[leftEdgeIndex]];
			float rightX = xCoords[orderXindices[(leftEdgeIndex+1)%4]];
			float rightZ = zCoords[orderZindices[(leftEdgeIndex+1)%4]];

			//0 = very top, 1 = bottom of top(mid), 2 = top of bottom(mid), 3 = bottom of bottom
			unsigned int row_index = 0;
			for (row_index=0; row_index<4; row_index++){
				//Left point
				vertices[vertexWriteIndex++] = leftX;
				vertices[vertexWriteIndex++] = heights[row_index];
				vertices[vertexWriteIndex++] = leftZ;
				//Tex_xy
				vertices[vertexWriteIndex++] = texXcoords[2*row_index];
				vertices[vertexWriteIndex++] = texYcoords[row_index];

				//Right point
				vertices[vertexWriteIndex++] = rightX;
				vertices[vertexWriteIndex++] = heights[row_index];
				vertices[vertexWriteIndex++] = rightZ;
				//Tex_xy
				vertices[vertexWriteIndex++] = texXcoords[2*row_index+1];
				vertices[vertexWriteIndex++] = texYcoords[row_index];

			}
	}

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	//Bind/setup VBO data
	glBufferData(GL_ARRAY_BUFFER, cVertices*cFloats*sizeof(float), vertices, GL_STATIC_DRAW);
	int stride = 5*sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//Bind/setup EBO data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(buildingSideIndices), buildingSideIndices, GL_STATIC_DRAW);



	glBindVertexArray(0);

	return VAO;
}//getBuildingVAO



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
	glfwSetTime(0);

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
	unsigned int colorLoc = glGetUniformLocation(crateShader->ID, "paint");

	unsigned int buildingTexture = loadTexture("textures/buildings/building.png");

	struct frame buildings[] = {
		{-20, 0, -20, 10, 50},
		{-18,50, -18,  6, 20},

		{10, 0,  10,  5,  10},
		{15, 0,  10,  5,  10},

		{10, 0, -10,  5,  10},
		{15, 0, -10,  5,  10},

		{-10, 0, 10,  2,  6},
		{-15, 0, 10,  2,  4},

		{50, 0, 40,  30, 150},
		{-51, 0, -30,  30, 100}
	};

	vec3 colors[] = {

		{1, 0, 0},
		{0, 0, 1},

		{1, 0, 0},
		{1, 1, 0},
		{1, 0, 1},

		{0, 1, 0},
		{0, 1, 1},
		{1, 1, 0},

		{0, 0, 1},
		{0, 1, 1}
		// {1, 0, 1},

	};

	// struct frame building = {-20, 0, -20, 10, 50};
	unsigned int buildingVAOs[10];
	int temp;
	for (temp=0; temp<10; temp++)
		buildingVAOs[temp] = getBuildingVAO(buildings[temp]);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);



	//----------- For Camera Fetching ------------- //
	//TV Panel
	float tvVertices[] = {
		-0.51,  0.25,  0.5, 0, 0,//TL
		-0.51,  0.25, -0.5, 1, 0,//TR
		-0.51, -0.25,  0.5, 0, 1,//BL
		-0.51, -0.25,  0.5, 0, 1,//BL
		-0.51,  0.25, -0.5, 1, 0,//TR
		-0.51, -0.25, -0.5, 1, 1//BR
	};

	unsigned int tvVAO;
	glGenVertexArrays(1, &tvVAO);
	glBindVertexArray(tvVAO);

	unsigned int tvVBO;
	glGenBuffers(1, &tvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tvVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tvVertices), tvVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	//Fetch camera frames from separate thread
	pthread_t framesThread;
	pthread_create(&framesThread, NULL, loadFramesLoop, window);

	// ----------------------------------------------//

	//Variables
	float x = 0, y = 0, z = 0;
	float theta = 0;

	double cam_prevx = WIN_WIDTH/2;
	double cam_prevy = WIN_HEIGHT/2;
	glfwGetCursorPos(window, &cam_prevx, &cam_prevy);
	double camx, camy;

	struct Camera *cam = cameraGetNew();
	cameraPosition3d(cam, 0, 1, 2);
	cameraRotate3d(cam, -0.2, 0, -1.5);

	glClearColor(0.2f, 0.3f, 0.3, 1.0);
	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		loadCameraTexture();

		// ------------------ Process Input --------------------- //
		if (glfwGetKey(window, GLFW_KEY_LEFT)) theta += 0.03;
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) theta -= 0.03;

		if (glfwGetKey(window, GLFW_KEY_UP)){
			x += cosf(-theta)/20;
			y += sinf(-theta)/20;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN)){
			x -= cosf(-theta)/20;
			y -= sinf(-theta)/20;
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER)){
			z += 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)){
			z -= 0.1f;
		}

		int keyR = glfwGetKey(window, GLFW_KEY_R);
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

		if (keyR){
			speedForward *= 2;
			speedStrafe *= 2;
			speedRaise *= 2;
		}

		if (!(keyW && keyS))
			if (keyW) cameraForward(cam, speedForward);
			else if (keyS) cameraForward(cam, -speedForward);

		if (!(keyA && keyD))
			if (keyA) cameraStrafe(cam, -speedStrafe);
			else if (keyD) cameraStrafe(cam, speedStrafe);

		if (!(keySpace && keyLeftShift))
			if (keySpace) cameraUp(cam, speedRaise);
			else if (keyLeftShift) cameraUp(cam, -speedRaise);


		glfwGetCursorPos(window, &camx, &camy);
		cameraRotateByMouse(cam, camx-cam_prevx, camy-cam_prevy);

		cameraGenerateViewMatrix(cam);

		//------------- Calculate Matrices --------------------- //

		mat4x4 perspective, viewTimesPerspective;


		//Perspective
		mat4x4_perspective(perspective, 5*M_PI/12, (float)WIN_WIDTH/(float)WIN_HEIGHT, 0.1f, 1000.0f);

		//Product
		mat4x4_mul(viewTimesPerspective, perspective, cam->viewMatrix);


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
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)cam->viewMatrix);
		glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);

		vec3 black = {0, 0, 0};
		glUniform3fv(colorLoc, 1, black);
		//Bind crate and face textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, crateTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, faceTexture);

		//Bind CrateVAO and draw triangles
		glBindVertexArray(crateVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Bind TV VAO
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cameraTexture);
		//Draw TV
		glBindVertexArray(tvVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		//Bind building texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buildingTexture);

		//Replace crateLocal with neutral matrix
		mat4x4_identity(crateLocal);
		glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);

		int buildingc;
		for (buildingc=0; buildingc<10; buildingc++){
			//Bind buildingVAO and draw triangles
			glBindVertexArray(buildingVAOs[buildingc]);
			glUniform3fv(colorLoc, 1, colors[buildingc]);
			glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
		}

		//Unbind VAO
		glBindVertexArray(0);

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

		glBindVertexArray(0);

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
