
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <linmath.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <camera.h>
#include <sphere.h>
#include <graph.h>
#include <rubiks.h>

#include "Shader.h"

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

int initGL();
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

struct crate_data {
	Shader *shader;
	unsigned int texture;

	unsigned int vao, ebo_c;
	unsigned int loc, mod, view, persp, paint;

	vec3 pos;
	float theta;
};

struct gnd_data {
	Shader *shader;
	unsigned int texture;

	unsigned int vao, ebo_c;
	unsigned int loc, vp;

	vec3 pos;
};

struct program_data {
	GLFWwindow *window;
	float zoom;
	int paddleGame, rubiks_grab;

	struct crate_data *crate;
	struct gnd_data *gnd;
};


void loadGnd();
void loadCrate();
void process_crate_input();
void crate_render(mat4x4 perspective, mat4x4 cam_view);
void paddle_render(Shader *shader, Object paddle);


void paddle_update(Object *paddle, Camera *cam);
void ball_reset(Sphere *sphere, vec3 targetPos, vec3 paddlePos);
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

void scrollCallback(GLFWwindow *win, double xoff, double yoff);

struct program_data *prog;

int main(){
	prog = malloc(sizeof(struct program_data));
	prog->paddleGame = 1;
	prog->rubiks_grab = 0;
	prog->zoom = 10;

	//Init graphics
	if (!initGL())
		return -1;
	glfwSetScrollCallback(prog->window, scrollCallback);

	loadGnd();
	loadCrate();


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
	int lats = 50, lons = 50;

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

	double cam_prevx = WIN_WIDTH/2;
	double cam_prevy = WIN_HEIGHT/2;
	glfwGetCursorPos(prog->window, &cam_prevx, &cam_prevy);
	double camx, camy;

	Camera *cam = cameraGetNew();
	cameraPosition3d(cam, 0, 1, 2);
	cameraRotate3d(cam, -0.2, 0, -1.5);
	int rotateAroundTarget = 1;
	int rotateAroundTargetLast = 0;

	// vec3 paddle
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

	//Create rubik's cube
	vec3 pos = {0, 10, 0};
	vec4 cubeColors[6] = {
		{1,0,0,1}, //Front
		{0,1,0,1}, //Right
		{1,1,1,1}, //Top
		{0,0,0,1}, //Bottom
		{1,1,0,1}, //Left
		{0,1,1,1}  //Back
	};
	rubiks *cube = rubiks_create(pos, cubeColors);

	//wstart
	glfwSetTime(0);
	glClearColor(0.2f, 0.3f, 0.3, 1.0);
	while (!glfwWindowShouldClose(prog->window)){
		//Enable transparency and cam-z calculations
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update water graphics
		int asd = 0;
		for (asd=0; asd<5; asd++)
			 updateDroplets(droplets, droplets_c, cam->pos);



		{  // ------------------ Process Input --------------------- //

			process_crate_input();

			GLFWwindow *win = prog->window;
			int keyR = glfwGetKey(win, GLFW_KEY_R);
			int keyW = glfwGetKey(win, GLFW_KEY_W);
			int keyA = glfwGetKey(win, GLFW_KEY_A);
			int keyS = glfwGetKey(win, GLFW_KEY_S);
			int keyD = glfwGetKey(win, GLFW_KEY_D);
			int keyLeftShift = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT);
			int keySpace = glfwGetKey(win, GLFW_KEY_SPACE);

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

			//Toggle rotateAroundPaddle
			int targetKey = GLFW_KEY_M;
			if (glfwGetKey(win, targetKey)){
				if (!rotateAroundTargetLast){
					rotateAroundTarget = 1 - rotateAroundTarget;
					rotateAroundTargetLast = 1;
				}
			} else {
				rotateAroundTargetLast = 0;
			}

			glfwGetCursorPos(win, &camx, &camy);
		}

		// -----------------  Paddle Game --------------------//
		if (glfwGetKey(prog->window, GLFW_KEY_P) == GLFW_PRESS)
			prog->paddleGame = 1 - prog->paddleGame;

		if (prog->paddleGame){
			paddle_update(&paddle, cam);

			if (glfwGetKey(prog->window, GLFW_KEY_B))
				ball_reset(sphere, targetPos, paddle.pt);

			updatePingBall(sphere, paddle);
			targetPos[0] = sphere->x, targetPos[1] = sphere->y, targetPos[2] = sphere->z;
			mat4x4_translate(targetTransform, targetPos[0], targetPos[1], targetPos[2]);
			mat4x4_scale_aniso(targetTransform, targetTransform, (float)sphere->r, (float)sphere->r, (float)sphere->r);

			graph_point(graph, targetPos[1]);

			// ---------------- Rendering ------------------- //
			glUseProgram(sphereShader->ID);
			paddle_render(sphereShader, paddle);

			//Render the graph
			glDisable(GL_DEPTH_TEST);
			renderGraph(graphShader, graph);
			glEnable(GL_DEPTH_TEST);

		}//paddleGame

		//------------- Calculate Matrices --------------------- //
		if (!prog->rubiks_grab){
			if (rotateAroundTarget)
				cameraRotateAroundTarget(cam, camx-cam_prevx, camy-cam_prevy, prog->zoom);
			else
				cameraRotateFromPos(cam, camx-cam_prevx, camy-cam_prevy);
		}

		cameraGenerateViewMatrix(cam);

		//Perspective
		mat4x4 perspective;
		mat4x4_perspective(perspective, 5*M_PI/12, (float)WIN_WIDTH/(float)WIN_HEIGHT, 0.01f, 1000.0f);

		//Camera view times Perspective
		mat4x4 viewTimesPerspective;
		mat4x4_mul(viewTimesPerspective, perspective, cam->viewMatrix);


		//------------------Render Objects -------------------------//
		crate_render(perspective, cam->viewMatrix);

		//Bind building texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buildingTexture);

		//Replace crateLocal with neutral matrix
		mat4x4 localIdentity;
		mat4x4_identity(localIdentity);
		glUseProgram(prog->crate->shader->ID);
		glUniformMatrix4fv(prog->crate->loc, 1, GL_FALSE, (const GLfloat *)localIdentity);

		int buildingc;
		for (buildingc=0; buildingc<10; buildingc++){
			//Bind buildingVAO and draw triangles
			glUniform3fv(prog->crate->paint, 1, colors[buildingc]);
			glBindVertexArray(buildingVAOs[buildingc]);
			glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
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

		if (glfwGetKey(prog->window, GLFW_KEY_C) && dist < MAX_DISTANCE){
			vec3 advance;
			vec3_scale(advance, direction, dist);
			vec3_add(targetPos, start, advance);
			mat4x4_translate(targetTransform, targetPos[0], targetPos[1], targetPos[2]);
			mat4x4_scale_aniso(targetTransform, targetTransform, (float)sphere->r, (float)sphere->r, (float)sphere->r);
		}


		//Render Sphere
		sphereShader->use(sphereShader);
		glUniformMatrix4fv(sphereLocalLoc, 1, GL_FALSE, (const GLfloat *)targetTransform);
		glUniformMatrix4fv(sphereModelLoc, 1, GL_FALSE, (const GLfloat *)localIdentity);
		glUniformMatrix4fv(sphereViewLoc, 1, GL_FALSE, (const GLfloat *)cam->viewMatrix);
		glUniformMatrix4fv(spherePerspectiveLoc, 1, GL_FALSE, (const GLfloat *)perspective);
		setInt(sphereShader, "damp_enabled", 1);

		//Draw the target sphere
		glUniform4fv(sphereColor, 1, sphere->rgba);
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

		// --------------------------Render Rubik's cube--------------------------//
		int piece, face;
		double distance_to_rubiks;

		if (!prog->rubiks_grab && glfwGetMouseButton(prog->window, GLFW_MOUSE_BUTTON_RIGHT)){
				distance_to_rubiks = rubiks_seek_face(cube, cam->pos, cam->dir, &piece, &face);
				prog->rubiks_grab = 1;
				rubiks_highlight(cube, piece, face);
		}
		if (prog->rubiks_grab){
			vec3 rot = {cam->pitch, cam->yaw, cam->roll};
			rubiks_rotate(cube, rot, camx-cam_prevx, camy-cam_prevy);


			if (!glfwGetMouseButton(prog->window, GLFW_MOUSE_BUTTON_RIGHT)){
				prog->rubiks_grab = 0;
			}
		}//rubiks grabbing

		setInt(sphereShader, "damp_enabled", 0);
		rubiks_render(cube, sphereShader->ID);
		setInt(sphereShader, "damp_enabled", 1);

		// ------------------- Render Paddle  -----------------------------------//

		// -------------    Render Tiles    -----------------------//
		// Bind Tile Shader Program
		glUseProgram(prog->gnd->shader->ID);

		unsigned int tile_vpLoc = glGetUniformLocation(prog->gnd->shader->ID, "vp");
		glUniformMatrix4fv(tile_vpLoc, 1, GL_FALSE, (const GLfloat *) viewTimesPerspective);

		//Bind tile texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prog->gnd->texture);

		mat4x4 local;
		mat4x4_identity(local);

		glBindVertexArray(prog->gnd->vao);
		int i,j;
		for (i=-10; i<10; i++){
			for (j=-10; j<10; j++){
				mat4x4_translate(local, 2*i, 0, 2*j);
				glUniformMatrix4fv(prog->gnd->loc, 1, GL_FALSE, (const GLfloat *)local);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}

		// ----------------------------------------------------//

		glBindVertexArray(0);

		//GLFW Update Ticks
		glfwPollEvents();
		processInput(prog->window);
		glfwSwapBuffers(prog->window);

		cam_prevx = camx;
		cam_prevy = camy;

	}

	glfwTerminate();
	return 0;

}//main

int initGL(){
	// glfw initialization
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	prog->window = glfwCreateWindow (WIN_WIDTH, WIN_HEIGHT, "OpenGL Window\n", NULL, NULL);
	if (prog->window == NULL){
		glfwTerminate();
		return 0;
	}
	glfwSetWindowPos(prog->window, WIN_X, WIN_Y);
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(prog->window, framebuffer_size_callback);
	glfwMakeContextCurrent(prog->window);

	// GLAD init
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		return 0;
	}

	setViewPort();

	glfwSetInputMode(prog->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return 1;
}//initGL
void scrollCallback(GLFWwindow *win, double xoff, double yoff){
	if (yoff > 0)
		prog->zoom /= 1.05;
	if (yoff < 0)
		prog->zoom *= 1.05;
}//scrollCallback

void loadGnd(){

	float vertices[] = {
		-1.0, -1.0,   0.5,0, //BL
		-1.0, 1.0,    0.5,1,   //TL
		1.0, 1.0,     1,1,   //TR

		1.0, 1.0,     1,1,   //TR
		1.0, -1.0,     1,0,   //BR
		-1.0, -1.0,   0.5,0 //BL
	};

	prog->gnd = malloc(sizeof(struct gnd_data));
	struct gnd_data *obj = prog->gnd;

	glGenVertexArrays(1, &obj->vao);
	glBindVertexArray(obj->vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*) (2*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	obj->shader = getShaderObject();
	loadShader(obj->shader, "res/tile_shader.vs", "res/tile_shader.fs");

	obj->loc = glGetUniformLocation(obj->shader->ID, "local");
	obj->vp = glGetUniformLocation(obj->shader->ID, "vp");
	obj->texture = loadTexture("textures/map.png");
}//loadGnd
void loadCrate(){

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

	prog->crate = malloc(sizeof(struct crate_data));
	struct crate_data *obj = prog->crate;
	obj->pos[0] = 0, obj->pos[1] = 0, obj->pos[2] = 0;
	obj->theta = M_PI/6;
	obj->texture = loadTexture("textures/map.png");
	// --- Vertex Array Buffers --- //
	glGenVertexArrays(1, &obj->vao);
	glBindVertexArray(obj->vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	obj->shader = getShaderObject();
	loadShader(obj->shader, "res/vertex_shader.vs.c", "res/fragment_shader.vs.c");

	obj->loc = glGetUniformLocation(obj->shader->ID, "local");
	obj->mod = glGetUniformLocation(obj->shader->ID, "model");
	obj->view = glGetUniformLocation(obj->shader->ID, "view");
	obj->persp = glGetUniformLocation(obj->shader->ID, "perspective");
	obj->paint = glGetUniformLocation(obj->shader->ID, "paint");

}//loadCrate

void process_crate_input(){
	if (glfwGetKey(prog->window, GLFW_KEY_LEFT)) prog->crate->theta += 0.03;
	if (glfwGetKey(prog->window, GLFW_KEY_RIGHT)) prog->crate->theta -= 0.03;

	vec3 crate_vel = {0,0,0};
	if (glfwGetKey(prog->window, GLFW_KEY_UP)){
		crate_vel[0] += cosf(-prog->crate->theta)/20;
		crate_vel[1] += sinf(-prog->crate->theta)/20;
	}
	if (glfwGetKey(prog->window, GLFW_KEY_DOWN)){
		crate_vel[0] -= cosf(-prog->crate->theta)/20;
		crate_vel[1] -= sinf(-prog->crate->theta)/20;
	}
	if (glfwGetKey(prog->window, GLFW_KEY_ENTER)){
		crate_vel[2] += 0.1f;
	}
	if (glfwGetKey(prog->window, GLFW_KEY_RIGHT_SHIFT)){
		crate_vel[2] -= 0.1f;
	}
	vec3_add(prog->crate->pos, prog->crate->pos, crate_vel);
}//process_crate_input
void crate_render(mat4x4 perspective, mat4x4 cam_view){

	mat4x4 local;
	vec3 pos;
	vec3_scale(pos, prog->crate->pos, 1);
	mat4x4_translate(local, pos[0], -pos[1], pos[2]); //translate
	mat4x4_rotate_Z(local, local, prog->crate->theta); //output, rot
	mat4x4 model;
	mat4x4_identity(model);

	//Load uniforms into crate shader
	glUseProgram(prog->crate->shader->ID);
	glUniformMatrix4fv(prog->crate->loc, 1, GL_FALSE, (const GLfloat *)local);
	glUniformMatrix4fv(prog->crate->mod, 1, GL_FALSE, (const GLfloat *)model);
	glUniformMatrix4fv(prog->crate->view, 1, GL_FALSE, (const GLfloat *)cam_view);
	glUniformMatrix4fv(prog->crate->persp, 1, GL_FALSE, (const GLfloat *)perspective);
	vec3 black = {0, 0, 0};
	glUniform3fv(prog->crate->paint, 1, black);

	//Bind crate texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, prog->crate->texture);

	//Bind vao and draw triangles
	glBindVertexArray(prog->crate->vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

}//crate_render
void paddle_render(Shader *shader, Object paddle){
	unsigned int color = glGetUniformLocation(shader->ID, "rgba"),
							 loc = glGetUniformLocation(shader->ID, "local"),
 							 mod = glGetUniformLocation(shader->ID, "model");
	vec3 darkGreen = {0, .9, .3};
	//Bind and render paddle
	mat4x4 identity;
	mat4x4_identity(identity);
	mat4x4 local;
	mat4x4_translate(local, paddle.pt[0], paddle.pt[1], paddle.pt[2]);
	mat4x4_rotate_Y(local, local, paddle.rt.yaw);
	mat4x4_rotate_X(local, local, paddle.rt.pitch);
	mat4x4_scale_aniso(local, local, paddle.r, 0.15, paddle.r);
	glUniform4fv(color, 1, darkGreen);
	glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat *)local);
	glUniformMatrix4fv(mod, 1, GL_FALSE, (const GLfloat *)identity);
	glBindVertexArray(paddle.vao);
	glDrawElements(GL_TRIANGLE_STRIP, paddle.ebo_c, GL_UNSIGNED_INT, 0);
}//paddle_render

void paddle_update(Object *paddle, Camera *cam){
	//Move the paddle to the right position
	int dist_to_paddle = 10;
	vec3 rel_dir;
	vec3_scale(rel_dir, cam->dir, prog->zoom);
	vec3_add(paddle->pt, cam->pos, rel_dir);
	paddle->rt.yaw = cam->yaw + M_PI/2;
	paddle->rt.pitch = cam->pitch + M_PI/12;
	paddle->rt.pitch = 0;
}//paddle_update

void ball_reset(Sphere *sphere, vec3 targetPos, vec3 paddlePos){
	vec3_scale(sphere->vel, targetPos, 0);//Clear sphere vel
	vec3_scale(targetPos, paddlePos, 1);
	targetPos[1] += 2;

	sphere->x = targetPos[0], sphere->y = targetPos[1], sphere->z = targetPos[2];
}//ball_reset




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
				force
			};
			double E = 0.01, sigma = 4, rm = 1.6;
			double forces[] = {
				1.5 * pow(cosf(xfuncs[funcID]),2) - 0.75,
				1.5 * pow(cosf(xfuncs[funcID]),2) - 0.7 - 0.3,
				E * ( pow(rm/xfuncs[funcID], 12) - 2*pow(rm/xfuncs[funcID],6) )
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
