
#ifndef GRAPH_H
#define GRAPH_H

#include <GLFW/glfw3.h>
#include <linmath.h>

#include <stdlib.h>
#include <stdio.h>

#define GRAPH_MIN -2000000000;
#define GRAPH_MAX 2000000000;

typedef struct graph_object {

  vec3 pos;
  float width, height;

  int index, len;
  float *data, *rel_data;

  float min, max;
  int recalculate_bounds;

  unsigned int vao, vbo, vbo_c;
  unsigned int ebo, ebo_c;

} Graph;


Graph *graph_create(vec3 top_left, float width, float height, int length){

  Graph *graph = malloc(sizeof(Graph));

  vec3_scale(graph->pos, top_left, 1);
  graph->width = width;
  graph->height = height;

  graph->index = 0;
  graph->len = length;
  graph->rel_data = malloc( length * sizeof(float));
  graph->data = malloc(  (length) * sizeof(float)  );
  {
    int i=0;
    for (i=0; i<length; i++)
      graph->data[i] = 0,
      graph->rel_data[i] = 0;
  }

  graph->min = GRAPH_MAX;
  graph->max = GRAPH_MIN;
  graph->recalculate_bounds = 1;

  glGenVertexArrays(1, &graph->vao);
  glBindVertexArray(graph->vao);

  glGenBuffers(1, &graph->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, graph->vbo);
  glBufferData(GL_ARRAY_BUFFER, graph->len*sizeof(float), graph->rel_data, GL_DYNAMIC_DRAW);

  glBindVertexArray(0);

  return graph;

}//graph_create

void graph_point(Graph *graph, float value){

  int ind = graph->index;
  float old = graph->data[ind];
  graph->data[graph->index++] = value;

  if (value > graph->max)
    graph->max = value;
  if (value < graph->min)
    graph->min = value;

  if (old == graph->max || old == graph->min)
    graph->recalculate_bounds = 1;
  else
    graph->rel_data[ind] = (graph->data[ind]-graph->min) / (graph->max-graph->min);

  if (graph->index == graph->len)
    graph->index = 0;

}//graph_point
int print = 0;
void graph_render(Graph *graph){
  //Assume that x,y,z and w,h are already written to the shader's uniforms.

  graph->min = GRAPH_MAX;
  graph->max = GRAPH_MIN;
  int i;
  for (i=0; i<graph->len; i++){
    if (graph->data[i] < graph->min)
      graph->min = graph->data[i];
    else if (graph->data[i] > graph->max)
      graph->max = graph->data[i];
  }
  for (i=0; i<graph->len; i++)
    graph->rel_data[i] = (graph->data[i]-graph->min)/(graph->max-graph->min);

  graph->recalculate_bounds = 0;


  glBindVertexArray(graph->vao);

  glBindBuffer(GL_ARRAY_BUFFER, graph->vbo);
  glBufferData(GL_ARRAY_BUFFER, graph->len*sizeof(float), graph->rel_data, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glDrawArrays(GL_LINE_STRIP, 0, graph->len);

  glBindVertexArray(0);

}//graph_render

#endif
