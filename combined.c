
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <linmath.h>
#include <camera.h>
#include <sphere.h>
#include <graph.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Shader.h"
#include <grabImage.h>

// int WIN_X = 1000;
// int WIN_Y = 200;
// int WIN_WIDTH = 800;
// int WIN_HEIGHT = 600;

int WIN_X = 0;
int WIN_Y = 0;
int WIN_WIDTH = 1920;
int WIN_HEIGHT = 1080;

double minf(double a, double b){
	return a<b?a:b;
}//minf

unsigned int loadTexture(char *);
void loadVideoTexture(Mat **, VideoCapture*);

void setViewPort();
void loadShaders(unsigned int, const char *, const char *);

void framebuffer_size_callback (GLFWwindow*, int, int);
void window_pos_callback (GLFWwindow*, int, int);

void processInput (GLFWwindow*);
void updateDroplets(Sphere *droplets[], int droplets_c, vec3 cam);
void renderGraph (Shader *, Graph *);

float c_t = 512/512.0f; //c_t
float c_b = 0/512.0f; //c_btom
float c_l = 0/1024.0f; //c_l
float c_r = 512/1024.0f; //c_r

struct frame {
	float x, y, z;
	float width, height;
};

typedef struct point {
	float x, y, z;
} P3;

typedef struct rotation3d {
	float pitch, roll, yaw;
} R3;

typedef struct objectPipeline {
	//3dim coordinate
	vec3 pt;
	float r;
	R3 rt;
	unsigned int vao;
	unsigned int vbo, vbo_c;
	unsigned int ebo, ebo_c;
} Object;

void updatePingBall(Sphere *ball, Object paddle);


#define side1 0
#define side2 8
#define side3 16
#define side4 24

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

	//Bind/setup EBO data
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(buildingSideIndices), buildingSideIndices, GL_STATIC_DRAW);

	//Bind/setup VBO data
	glBufferData(GL_ARRAY_BUFFER, cVertices*cFloats*sizeof(float), vertices, GL_STATIC_DRAW);
	int stride = 5*sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);




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
	window = glfwCreateWindow (WIN_WIDTH, WIN_HEIGHT, "OpenGL Window\n", NULL, NULL);
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


	//Create and render a sphere
	int lats = 10, lons = 10;

	Sphere *sphere = sphere_create(0, 0, 0, 0.2);
	sphere_rgba(sphere, 1, 0, 0, 1);
	sphere_init_model(sphere, lats, lons);
	sphere_attach_vao(sphere);

	float xyzr[] = {
		3,    10, 30,    10,
		3, 		25, 30, 	 7.5
	};

	int sphere_c = 2;
	Sphere *spheres[sphere_c];

	int i;
	for (i=0; i<sphere_c; i++){
		spheres[i] = sphere_create(xyzr[4*i], xyzr[4*i+1], xyzr[4*i+2], xyzr[4*i+3]);
		sphere_rgba(spheres[i], 1, 1, 1, 1);
		sphere_init_model(spheres[i], lats, lons);
		sphere_attach_vao(spheres[i]);
	}

	int SEED = 125236;
	srand(SEED);


	double cx = 30, cy = 10, cz = 5, cr = 10;
	int droplets_c = 50;
	Sphere *droplets[droplets_c];

	int dc;
	for (dc=0; dc<droplets_c; dc++){
		double x = cx+cr* 2*(rand()/(float)RAND_MAX)-cr;
		double y = cy+cr* 2*(rand()/(float)RAND_MAX)-cr;
		double z = cz+cr* 2*(rand()/(float)RAND_MAX)-cr;

		droplets[dc] = sphere_create(x, y, z, 1);
		droplets[dc]->vel[0] = 0;
		droplets[dc]->vel[1] = 0;
		droplets[dc]->vel[2] = 0;
		sphere_rgba(droplets[dc], 0, 0.1, 0.9, 1);
		sphere_init_model(droplets[dc], 5, 8);
		sphere_attach_vao(droplets[dc]);
	}


	Shader *sphereShader = getShaderObject();
	sphereShader->loadShader(sphereShader, "res/sphere.vs", "res/sphere.fs");
	unsigned int sphereLocalLoc = glGetUniformLocation(sphereShader->ID, "local"),
							 sphereModelLoc = glGetUniformLocation(sphereShader->ID, "model"),
							 sphereViewLoc  = glGetUniformLocation(sphereShader->ID, "view"),
							 spherePerspectiveLoc	= glGetUniformLocation(sphereShader->ID, "perspective"),
							 sphereColor		= glGetUniformLocation(sphereShader->ID, "rgba");

	//Targetting
	vec3 targetPos = {10, 0, 0};
	mat4x4 targetTransform;
	mat4x4_translate(targetTransform, targetPos[0], targetPos[1], targetPos[2]);
	mat4x4_scale_aniso(targetTransform, targetTransform, (float)sphere->r, (float)sphere->r, (float)sphere->r);

	//Variables
	float x = 0, y = 0, z = 0;
	float theta = 0;

	double cam_prevx = WIN_WIDTH/2;
	double cam_prevy = WIN_HEIGHT/2;
	glfwGetCursorPos(window, &cam_prevx, &cam_prevy);
	double camx, camy;

	Camera *cam = cameraGetNew();
	cameraPosition3d(cam, 0, -24, 0);
	cameraRotate3d(cam, -0.2, 0, -1.5);
	int rotateAroundPaddle = 0;
	int rotateAroundPaddleLast = 0;

	// vec3 paddle
	int drawPaddle = 0;
	Object paddle;
	{ //Paddle
		float points[3*2*10];
		int points_i = 0;
		for (points_i=0; points_i<20; points_i++){
			points[3*points_i] = cosf(points_i * 2*M_PI/10);
			points[3*points_i+1] = (points_i<10?1:-1);
			points[3*points_i+2] = sinf(points_i * 2*M_PI/10);
		}

		int paddle_ebo[2 *3*(10-2)  +  11*2];
		int paddle_ebo_i;
		for (paddle_ebo_i=0; paddle_ebo_i<10-2; paddle_ebo_i++){
			paddle_ebo[3*paddle_ebo_i  ] = 0;
			paddle_ebo[3*paddle_ebo_i+1] = paddle_ebo_i+1;
			paddle_ebo[3*paddle_ebo_i+2] = paddle_ebo_i+2;
		}
		for (paddle_ebo_i=0; paddle_ebo_i<10-2; paddle_ebo_i++){
			paddle_ebo[24+3*paddle_ebo_i  ] = 10;
			paddle_ebo[24+3*paddle_ebo_i+1] = 10+paddle_ebo_i+1;
			paddle_ebo[24+3*paddle_ebo_i+2] = 10+paddle_ebo_i+2;
		}
		for (paddle_ebo_i=0; paddle_ebo_i<=10; paddle_ebo_i++){
			paddle_ebo[48+2*paddle_ebo_i] = paddle_ebo_i%10;
			paddle_ebo[48+2*paddle_ebo_i+1] = 10+paddle_ebo_i%10;
		}

		vec3 paddle_start = {0, -5, 0};
		vec3_scale(paddle.pt, paddle_start, 1);
		paddle.r = 2;

		glGenVertexArrays(1, &paddle.vao);
		glBindVertexArray(paddle.vao);

		paddle.vbo_c = 3*20;
		glGenBuffers(1, &paddle.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, paddle.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
		glEnableVertexAttribArray(0);

		paddle.ebo_c = 2*3*(10-2)  +  11*2;
		glGenBuffers(1, &paddle.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, paddle.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(paddle_ebo), paddle_ebo, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	Graph *graph;
	Shader *graphShader;
	{ //Graph
		vec3 top_left = {-1, -1, 0};
		graph = graph_create(top_left, 2.0, 0.2, 512);

		//Shader
		graphShader = getShaderObject();
		loadShader(graphShader, "res/graph.vs", "res/graph.fs");
		glUseProgram(graphShader->ID);

		setInt(graphShader, "length", graph->len);

		setFloat(graphShader, "coordx", graph->pos[0]);
		setFloat(graphShader, "coordy", graph->pos[1]);
		setFloat(graphShader, "coordz", graph->pos[2]);

		setFloat(graphShader, "width", graph->width);
		setFloat(graphShader, "height", graph->height);

		// printf("%d %.1f %.1f %.1f %.1f %.1f\n", graph->len, graph->pos[0], graph->pos[1], graph->pos[2], graph->width, graph->height);

	}

	// Video Texture //
	// #define VIDEO
	#ifdef VIDEO
	// Video Texture //
	MatVideoStruct box; box.mat = 0;
	unsigned int earthTexture = createVideoTexture("textures/drone.mp4", &box.video);
	loadVideoTextureStruct(&box);
	#else
	unsigned int earthTexture = loadTexture("textures/earth.jpg");
	unsigned int earthNightTexture = loadTexture("textures/earth_night.jpg");
	#endif
	Sphere* earthSphere;
	Shader *texturedSphereShader;
	int sphereShaderMatrices[4];

	float earthWrapPercent = 1;
	int earthWrapDir = 1;

	{ //Round textured sphere EARTH!
		//Initial wireframe sphere object including texture coordinates
			// for equirectangular image
		earthSphere = sphere_create(0,-35,0,5);
		earthSphere->textured = 1;
		#ifdef VIDEO
		earthSphere->flipped = 1;
		#endif
		sphere_init_model(earthSphere,200,200);
		sphere_attach_vao(earthSphere);

		//Create shader program
		texturedSphereShader = getShaderObject();
		glUseProgram(texturedSphereShader->ID);
		loadShader(texturedSphereShader, "res/texturedSphereShader.vs", "res/texturedSphereShader.fs");

	 	sphereShaderMatrices[0] = glGetUniformLocation(texturedSphereShader->ID, "local");
	 	sphereShaderMatrices[1] = glGetUniformLocation(texturedSphereShader->ID, "model");
	 	sphereShaderMatrices[2] = glGetUniformLocation(texturedSphereShader->ID, "view");
	 	sphereShaderMatrices[3] = glGetUniformLocation(texturedSphereShader->ID, "perspective");

		printf("Earth has %d elements to draw\n", earthSphere->ebo_indices_c);
	}

	unsigned int sunTexture = loadTexture("textures/venus.jpg");
	Sphere* sunSphere;
	Shader *LTSS; //litTexturedSphereShader
	int LTSS_matrices[4];
	int LTSS_light3f, LTSS_camera3f;

	{ //Round textured sphere SUN!
		//Initial wireframe sphere object including texture coordinates
			// for equirectangular image
		sunSphere = sphere_create(0,15,30,10);
		sunSphere->textured = 1;

		sphere_init_model(sunSphere,20,20);
		sphere_attach_vao(sunSphere);

		//Create shader program
		LTSS = getShaderObject();
		glUseProgram(LTSS->ID);
		loadShader(LTSS, "res/litTexturedSphereShader.vs", "res/litTexturedSphereShader.fs");

		LTSS_matrices[0] = glGetUniformLocation(LTSS->ID, "local");
		LTSS_matrices[1] = glGetUniformLocation(LTSS->ID, "model");
		LTSS_matrices[2] = glGetUniformLocation(LTSS->ID, "view");
		LTSS_matrices[3] = glGetUniformLocation(LTSS->ID, "perspective");

		LTSS_light3f  = glGetUniformLocation(LTSS->ID, "light_pos");
		LTSS_camera3f = glGetUniformLocation(LTSS->ID, "camera_pos");
		glUseProgram(LTSS->ID);
		setInt(LTSS, "day", 0);
		setInt(LTSS, "night", 1);

		printf("Sun has %d elements to draw\n", sunSphere->ebo_indices_c);
	}

	//wstart
	glfwSetTime(0);
	glClearColor(0.2f, 0.3f, 0.3, 1.0);
	while (!glfwWindowShouldClose(window)){
		#ifdef VIDEO
		if (box.mat){
			updateTextureWithMat(earthTexture, &box.mat);
			pthread_t thread_id;
			pthread_create(&thread_id, NULL, loadVideoTextureStruct, (void*)&box);
		}
		#endif

		//Enable transparency and cam-z calculations
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update water graphics
		int asd = 5;
		for (asd=0; asd<5; asd++)
			 updateDroplets(droplets, droplets_c, cam->pos);

		//Move the paddle to the right position
		int dist_to_paddle = 10;
		vec3 rel_dir;
		vec3_scale(rel_dir, cam->dir, dist_to_paddle);
		vec3_add(paddle.pt, cam->pos, rel_dir);
		paddle.rt.yaw = cam->yaw + M_PI/2;
		paddle.rt.pitch = cam->pitch + M_PI/12;
		paddle.rt.pitch = 0;

		{ //Paddle-ball keys
			if (glfwGetKey(window, GLFW_KEY_B)){
				vec3_scale(sphere->vel, targetPos, 0);//Clear sphere vel
				vec3_scale(targetPos, paddle.pt, 1);
				targetPos[1] += 2;

				sphere->x = targetPos[0], sphere->y = targetPos[1], sphere->z = targetPos[2];
			}
			if (glfwGetKey(window, GLFW_KEY_N))
				drawPaddle = 1-drawPaddle;
		}

		updatePingBall(sphere, paddle);
		targetPos[0] = sphere->x, targetPos[1] = sphere->y, targetPos[2] = sphere->z;
		mat4x4_translate(targetTransform, targetPos[0], targetPos[1], targetPos[2]);
		mat4x4_scale_aniso(targetTransform, targetTransform, (float)sphere->r, (float)sphere->r, (float)sphere->r);

		graph_point(graph, targetPos[1]);


		{  // ------------------ Process Input --------------------- //
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

			//Animate the earth between equirectangular and spherical
			float animationStep = 0.01;
			int keyU = glfwGetKey(window, GLFW_KEY_U);
			int keyI = glfwGetKey(window, GLFW_KEY_I);
			if (keyU || keyI) {
				if (keyU)
					earthWrapPercent += animationStep;
				if (keyI)
					earthWrapPercent -= animationStep;

				if (earthWrapPercent > 1){
					earthWrapPercent = 1;
					earthWrapDir = -earthWrapDir;
				} else if (earthWrapPercent < 0) {
					earthWrapPercent = 0;
					earthWrapDir = -earthWrapDir;
				}
				sphere_update_model(earthSphere, earthWrapPercent);
				// printf("Progress: %.2f\n", earthWrapPercent);
			}

			//Toggle rotateAroundPaddle
			int paddleKey = GLFW_KEY_M;
			if (glfwGetKey(window, paddleKey)){
				if (!rotateAroundPaddleLast){
					rotateAroundPaddle = 1 - rotateAroundPaddle;
					rotateAroundPaddleLast = 1;
				}
			} else {
				rotateAroundPaddleLast = 0;
			}

			glfwGetCursorPos(window, &camx, &camy);
		}

		//------------- Calculate Matrices --------------------- //
		//Update normal of cameral to be perpendicular to the earth
		vec3 cam_n;
		vec3_sub(cam_n, cam->pos, earthSphere->pos);
		vec3_norm(cam_n, cam_n);
		// cam_n[0] = 1; cam_n[1] = 0; cam_n[2] = 0;
		// cameraSetUp(cam, cam_n);

		if (rotateAroundPaddle)
			cameraRotateAroundTarget(cam, camx-cam_prevx, camy-cam_prevy, dist_to_paddle);
		else
			cameraRotateFromPos(cam, camx-cam_prevx, camy-cam_prevy);

		cameraGenerateViewMatrix(cam);

		mat4x4 perspective, viewTimesPerspective;

		//Perspective
		float fov = 5*M_PI/12;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
			fov = 2*M_PI/24;
		if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
			fov = 2*M_PI/2.1;
		mat4x4_perspective(perspective, fov, (float)WIN_WIDTH/(float)WIN_HEIGHT, 0.1f, 1000.0f);

		//Product
		mat4x4_mul(viewTimesPerspective, perspective, cam->viewMatrix);

		// ------------------- Render Crate -------------------- //

		mat4x4 localIdentity;
		mat4x4_identity(localIdentity);

		mat4x4 crateLocal, crateModel;
		mat4x4_identity(crateModel);
		mat4x4_identity(crateLocal);//rot
		mat4x4_translate_in_place(crateLocal, x, -y, z); //translate
		mat4x4_rotate_Z(crateLocal, crateLocal, theta); //output, rot

		// while (0)
		{ // TEXTURED SPHERE RENDERING

			earthSphere->rot[1] += 0.001;

			glUseProgram(LTSS->ID);

			mat4x4 earthLocal;
			sphere_local_matrix(earthSphere, earthLocal);

			{  //Send matrix transformations to shader program
				glUniformMatrix4fv(LTSS_matrices[0], 1, GL_FALSE, (const GLfloat *)earthLocal);
				glUniformMatrix4fv(LTSS_matrices[1], 1, GL_FALSE, (const GLfloat *)localIdentity);
				glUniformMatrix4fv(LTSS_matrices[2], 1, GL_FALSE, (const GLfloat *)cam->viewMatrix);
				glUniformMatrix4fv(LTSS_matrices[3], 1, GL_FALSE, (const GLfloat *)perspective);

				glUniform3fv(LTSS_light3f, 1, (const GLfloat *)sunSphere->pos);
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, earthTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, earthNightTexture);

			glBindVertexArray(earthSphere->VAO);

			glDrawElements(GL_TRIANGLE_STRIP, earthSphere->ebo_indices_c, GL_UNSIGNED_INT, (void*)0);

			// /* Wireframe
				//Change Radius of sphere and update matrix
				float temp = earthSphere->r;
				earthSphere->r *= 1.001;
				glBindTexture(GL_TEXTURE_2D, 0);
				sphere_local_matrix(earthSphere, earthLocal);
				glUniformMatrix4fv(sphereShaderMatrices[0], 1, GL_FALSE, (const GLfloat *)earthLocal);
				glDrawElements(GL_LINE_STRIP, earthSphere->ebo_indices_c, GL_UNSIGNED_INT, 0);
				earthSphere->r = temp;
			// */
		} //Textured sphere rendering

		{ // TEXTURED SUN RENDERING

			sunSphere->pos[0] = 20*sinf(3*glfwGetTime());
			sunSphere->pos[1] = -35;
			sunSphere->pos[2] = 20*cosf(3*glfwGetTime());

			glUseProgram(texturedSphereShader->ID);

			mat4x4 sunLocal;
			sphere_local_matrix(sunSphere, sunLocal);

			{  //Send matrix transformations to shader program
				glUniformMatrix4fv(sphereShaderMatrices[0], 1, GL_FALSE, (const GLfloat *)sunLocal);
				glUniformMatrix4fv(sphereShaderMatrices[1], 1, GL_FALSE, (const GLfloat *)localIdentity);
				glUniformMatrix4fv(sphereShaderMatrices[2], 1, GL_FALSE, (const GLfloat *)cam->viewMatrix);
				glUniformMatrix4fv(sphereShaderMatrices[3], 1, GL_FALSE, (const GLfloat *)perspective);
			}

			glUniform3fv(LTSS_light3f, GL_FALSE, (const GLfloat *)sunSphere->pos);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, sunTexture);

			glBindVertexArray(sunSphere->VAO);

			glDrawElements(GL_TRIANGLE_STRIP, sunSphere->ebo_indices_c, GL_UNSIGNED_INT, (void*)0);

			// /* Wireframe
				//Change Radius of sphere and update matrix
				float temp = sunSphere->r;
				sunSphere->r *= 1.001;
				glBindTexture(GL_TEXTURE_2D, 0);
				sphere_local_matrix(sunSphere, sunLocal);
				glUniformMatrix4fv(sphereShaderMatrices[0], 1, GL_FALSE, (const GLfloat *)sunLocal);
				glDrawElements(GL_LINE_STRIP, sunSphere->ebo_indices_c, GL_UNSIGNED_INT, 0);
				sunSphere->r = temp;
			// */
		} //Textured sphere rendering

		glUseProgram(crateShader->ID);

		{  //Send matrix transformations to shader program
			glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)cam->viewMatrix);
			glUniformMatrix4fv(perspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);
		}

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


		//Bind building texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buildingTexture);

		//Replace crateLocal with neutral matrix
		glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *)localIdentity);

		int buildingc;
		for (buildingc=0; buildingc<10; buildingc++){
			//Bind buildingVAO and draw triangles
			glBindVertexArray(buildingVAOs[buildingc]);
			glUniform3fv(colorLoc, 1, colors[buildingc]);
			glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
		}

		//Render Sphere
		sphereShader->use(sphereShader);

		{ //Send matrix transformations to shader program
			glUniformMatrix4fv(sphereLocalLoc, 1, GL_FALSE, (const GLfloat *)localIdentity);
			glUniformMatrix4fv(sphereModelLoc, 1, GL_FALSE, (const GLfloat *)crateModel);
			glUniformMatrix4fv(sphereViewLoc, 1, GL_FALSE, (const GLfloat *)cam->viewMatrix);
			glUniformMatrix4fv(spherePerspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);
		}

		//Get target location (where the target sphere collides with other objects)
		vec3 start, direction;
		vec3_scale(start, cam->pos, 1);
		vec3_scale(direction, cam->dir, 1);

		float MAX_DISTANCE = 12000000;
		float dist = MAX_DISTANCE;
		int reset = 0;

		int di;
		for (di=0; di<sphere_c; di++){
			//dist should equal the distance from the camera to the target
			float R = spheres[di]->r + sphere->r; //1 == radius of targetting sphere
			vec3 object = {spheres[di]->x, spheres[di]->y, spheres[di]->z};

			vec3 rel_object;
			vec3_sub(rel_object, object, start);

			//Now we have the vector from the camera to the object (rel_object)
			//	and the vector of the camera direction (direction);

			// 		vx								 vy									vz
			float dx = direction[0], dy = direction[1], dz = direction[2];
			float ox = rel_object[0], oy = rel_object[1], oz = rel_object[2];

			/*	------------Solution for the above math---------------
					// Find the distance between the object (ox,oy,oz) and a moving camera
			(t*dx-ox)^2 + (t*dy-oy)^2 + (t*dz-oz)^2 = R^2
					// Expand the polynomial
			(t2*dx2 - 2*t*dx*ox + ox2) + (t2*dy2 - 2*t*dy*oy + oy2) + (t2*dz2 - 2*t*dz*oz + oz2) == R2
					// Order by terms of t
			t2*(dx2 + dy2 + dz2) - 2*t*(dxox + dyoy + dz*oz) + (ox2 + oy2 + oz2 - R2) = 0
					// Apply the quadratic equation x = -b += sqrt(pow(b,2) - 4*a*c)/(2*a)

			*/

			float a = pow(dx,2) + pow(dy,2) + pow(dz,2);
			float b = -2 * (dx*ox + dy*oy + dz*oz);
			float c = pow(ox,2) + pow(oy,2) + pow(oz,2) - pow(R,2);

			float x = (-b - sqrt(pow(b,2) - 4*a*c))/(2*a);
			// x1 = -b + sqrt(pow(b,2) - 4*a*c)/(2*a);

			if (x > 0 && x < dist)
				dist = x;

		}

		if (glfwGetKey(window, GLFW_KEY_C) && dist < MAX_DISTANCE){
			vec3 advance;
			vec3_scale(advance, direction, dist);
			vec3_add(targetPos, start, advance);
			mat4x4_translate(targetTransform, targetPos[0], targetPos[1], targetPos[2]);
			mat4x4_scale_aniso(targetTransform, targetTransform, (float)sphere->r, (float)sphere->r, (float)sphere->r);
		}

		//Draw the target sphere
		glUniform4fv(sphereColor, 1, sphere->rgba);
		glUniformMatrix4fv(sphereLocalLoc, 1, GL_FALSE, (const GLfloat *)targetTransform);
		glBindVertexArray(sphere->VAO);
		glDrawElements(GL_TRIANGLE_STRIP, sphere->ebo_indices_c, GL_UNSIGNED_INT, 0);

		mat4x4 sphere_local;
		int sphere_i;
		for (sphere_i=0; sphere_i<sphere_c; sphere_i++){
			//Update local matrix location and sphere color
			sphere_local_matrix(spheres[sphere_i], sphere_local);
			glUniformMatrix4fv(sphereLocalLoc, 1, GL_FALSE, (const GLfloat *)sphere_local);
			// glUniformMatrix4fv(sphereModelLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);
			glUniform4fv(sphereColor, 1, spheres[sphere_i]->rgba);
			//Bind Sphere VAO
			glBindVertexArray(spheres[sphere_i]->VAO);
			glDrawElements(GL_TRIANGLE_STRIP, spheres[sphere_i]->ebo_indices_c, GL_UNSIGNED_INT, 0);
			glDrawElements(GL_LINE_STRIP, spheres[sphere_i]->ebo_indices_c, GL_UNSIGNED_INT, 0);
		}

		//ddrop
		for (sphere_i=0; sphere_i<droplets_c; sphere_i++){
			// vec3 to_sphere;
			// vec3_sub(to_sphere, cam->pos, {droplets[sphere_i].x, droplets[sphere_i].y, droplets[sphere_i].z})

			//Update local matrix location and sphere color
			sphere_local_matrix(droplets[sphere_i], sphere_local);
			glUniformMatrix4fv(sphereLocalLoc, 1, GL_FALSE, (const GLfloat *)sphere_local);
			// glUniformMatrix4fv(sphereModelLoc, 1, GL_FALSE, (const GLfloat *)crateLocal);
			glUniform4fv(sphereColor, 1, droplets[sphere_i]->rgba);
			//Bind Sphere VAO
			glBindVertexArray(droplets[sphere_i]->VAO);
			glDrawElements(GL_TRIANGLE_STRIP, droplets[sphere_i]->ebo_indices_c, GL_UNSIGNED_INT, 0);
		}

		if (drawPaddle){
			vec4 darkGreen = {1, 1, 1, 1};
			//Bind and render paddle
			mat4x4 paddleLocal;
			mat4x4_identity(paddleLocal);
			mat4x4_translate_in_place(paddleLocal, paddle.pt[0], paddle.pt[1], paddle.pt[2]);
			mat4x4_rotate_Y(paddleLocal, paddleLocal, paddle.rt.yaw);
			mat4x4_rotate_X(paddleLocal, paddleLocal, paddle.rt.pitch);
			mat4x4_scale_aniso(paddleLocal, paddleLocal, paddle.r, 0.15, paddle.r);
			glUniform4fv(sphereColor, 1, darkGreen);
			glUniformMatrix4fv(sphereLocalLoc, 1, GL_FALSE, (const GLfloat *)paddleLocal);
			glBindVertexArray(paddle.vao);
			glDrawElements(GL_TRIANGLE_STRIP, paddle.ebo_c, GL_UNSIGNED_INT, 0);
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


		//Render the graph
		glDisable(GL_DEPTH_TEST);
		renderGraph(graphShader, graph);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);

		//GLFW Update Ticks
		glfwPollEvents();
		processInput(window);
		glfwSwapBuffers(window);

		cam_prevx = camx;
		cam_prevy = camy;

	}
	printVideo();
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


void updateDroplets(Sphere *droplets[], int droplets_c, vec3 cam){
	Sphere *o1, *o2;
	int i1, i2;
	for (i1=0; i1<droplets_c; i1++){
		o1 = droplets[i1];
		for (i2=i1+1; i2<droplets_c; i2++){
			o2 = droplets[i2];
			double R = o1->r + o2->r;
			vec3 p1 = {o1->x, o1->y, o1->z};
			vec3 p2 = {o2->x, o2->y, o2->z};
			vec3 ray;
			vec3_sub(ray, p1, p2);

			double d = vec3_len(ray);
			if (d > 9*R) continue;

			// force < 0 equals attraction
			double force = (d/R);

			int funcID = 2;
			double xfuncs[] = {
				3*M_PI/4 * pow(force,2),
				(1/1.15)*(3*M_PI/4)*pow(force+0.1,2),
				d
			};
			double E = 0.01, sigma = 4, rm = 1.6;
			double forces[] = {
				1.5 * pow(cosf(xfuncs[funcID]),2) - 0.75,
				1.5 * pow(cosf(xfuncs[funcID]),2) - 0.7 - 0.3,
				E * ( pow(rm/xfuncs[funcID], 12) - 20*pow(rm/xfuncs[funcID],6) )
			};

			force = forces[funcID];

			//Adjust force to appropriate units
			double scale = 0.00006;
			//Normalize direction vector from p1 to p2
			vec3_scale(ray, ray, force*scale/d);

			vec3_add(o1->vel, o1->vel, ray);
			vec3_sub(o2->vel, o2->vel, ray);

		}
		do { //Be nudged by cameraUp

			double R = o1->r;
			vec3 p1 = {o1->x, o1->y, o1->z};
			vec3 p2 = {cam[0], cam[1], cam[2]};
			vec3 ray;
			vec3_sub(ray, p1, p2);

			double d = vec3_len(ray);
			if (d > 9*R) break;

			// force < 0 equals attraction
			double force = (d/R);

			int funcID = 2;
			double xfuncs[] = {
				3*M_PI/4 * pow(force,2),
				(1/1.15)*(3*M_PI/4)*pow(force+0.1,2),
				force
			};
			double E = 0.01, sigma = 4, rm = 1.2;
			double forces[] = {
				1.5 * pow(cosf(xfuncs[funcID]),2) - 0.75,
				1.5 * pow(cosf(xfuncs[funcID]),2) - 0.7 - 0.3,
				E * ( pow(rm/xfuncs[funcID], 12) - 2*pow(rm/xfuncs[funcID],6) )
			};

			force = forces[funcID];

			//Adjust force to appropriate units
			double scale = 0.0001;
			//Normalize direction vector from p1 to p2
			vec3_scale(ray, ray, force*scale/d);

			vec3_add(o1->vel, o1->vel, ray);
		} while (0);


		o1->x += o1->vel[0];
		o1->y += o1->vel[1];
		o1->z += o1->vel[2];
	}

}//updateDroplets

float absf(float val){
	return val<0 ? -val: val;
}//absf

void updatePingBall(Sphere *ball, Object paddle){

	float dt = 1;

	vec3 sphere_pos = {ball->x, ball->y, ball->z};

	vec3 paddle_to_ball_pos;
	vec3_sub(paddle_to_ball_pos, sphere_pos, paddle.pt);

	//With a flat paddle, we only care about the vertical velocity
		//Thus, we transform all pieces to the plane of the paddle
	vec3 rotated_vel_vector;
	vec3_scale(rotated_vel_vector, ball->vel, 1);

	if (abs(ball->vel[1])<0.01 && abs(paddle_to_ball_pos[1])<0.01+0.15+ball->r);
	else
		ball->vel[1] += -0.02;

	do { // Check if the sphere will hit the paddle in the next Ticks
		float x = paddle_to_ball_pos[0];
		float y = paddle_to_ball_pos[1];
		float z = paddle_to_ball_pos[2];

		//Quick check athat the ball is near the paddle
		if (x*x + z*z > paddle.r*paddle.r) break;
		if (y < -0.1) break;

		if (y+ball->vel[1] < 0.15+ball->r){
			ball->vel[1] = 0.95*absf(ball->vel[1]);

			float rndX = rand()/(float)RAND_MAX-0.5,
						rndZ = rand()/(float)RAND_MAX-0.5;
			ball->vel[0] += rndX/1000;
			ball->vel[2] += rndZ/1000;
		}
			// ball->vel[1] = -abs(1.1*ball->vel[1]);

	} while (0);


	ball->x += ball->vel[0];
	ball->y += ball->vel[1];
	ball->z += ball->vel[2];

}//updatePingBall

void renderGraph(Shader *shader, Graph *graph){
	glUseProgram(shader->ID);
	setInt(shader, "index", graph->index);
	graph_render(graph);
}//renderGraph
