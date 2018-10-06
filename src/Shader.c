
#include "Shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *readFile (char *path);

void use (Shader *shader){
  glUseProgram(shader->ID);
}

void loadShader (Shader *shader, char *vertPath, char *fragPath){

  const char *vertexSource = readFile(vertPath);
  const char *fragmentSource = readFile(fragPath);

  unsigned int vertexShader, fragmentShader;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	//    ------  Error Checking ------ //
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if(!success)
	{
	    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
	}

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if(!success)
	{
	    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
	}

	// --- Shader Linking --- //

	glAttachShader(shader->ID, vertexShader);
	glAttachShader(shader->ID, fragmentShader);
	glLinkProgram(shader->ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
	if(!success) {
	    glGetProgramInfoLog(shader->ID, 512, NULL, infoLog);
			printf("EROOR::SHADER::LINKER::COMPILATION_FAILED\n%s", infoLog);
	}
}

void setInt (Shader *shader, char *key, int value){
  unsigned int vertexLocation = glGetUniformLocation(shader->ID, key);
  glUniform1i(vertexLocation, value);
}//setInt

void setBool (Shader *shader, char *key, bool value){
  unsigned int vertexLocation = glGetUniformLocation(shader->ID, key);
  glUniform1i(vertexLocation, value);
}//setInt

void setFloat (Shader *shader, char *key, float value){
  unsigned int vertexLocation = glGetUniformLocation(shader->ID, key);
  glUniform1f(vertexLocation, value);
}//setInt

Shader* getShaderObject(){
  Shader *shader = malloc(sizeof(Shader));
  shader->ID = glCreateProgram();

  shader->use = use;
  shader->loadShader = loadShader;

  shader->setInt = setInt;
  shader->setBool = setBool;
  shader->setFloat = setFloat;

  return shader;
}

// ----  ReadFile ---- //
// #define DEBUG
#define BLOCK_SIZE 512

struct file_block {
  char data[BLOCK_SIZE];
  unsigned int size;
  struct file_block *next;
};

char *readFile(char *path){

  #ifdef DEBUG
  printf("Reading from: %s\n", path);
  #endif

  struct file_block *listStart = malloc(sizeof(struct file_block));
  listStart->size = 0;
  listStart->next = NULL;

  struct file_block *listPtr = listStart;

  FILE *fin = fopen(path, "r");

  while (1){
    unsigned int room = BLOCK_SIZE-listPtr->size;
    int read = fread(listPtr->data, sizeof(char), room, fin);

    listPtr->size += read;

    #ifdef DEBUG
    printf("Read %d bytes\n", read);
    #endif

    //File could not fill buffer, file must have terminated
    if (listPtr->size < BLOCK_SIZE){
      //Add terminating character to <BLOCK_SIZE buffers
      listPtr->data[listPtr->size] = '\0';
      break;
    } else {
      struct file_block *listNext = malloc(sizeof(struct file_block));
      listNext->size = 0;
      listNext->next = NULL;

      listPtr->next = listNext; //Append new list item
      listPtr = listPtr->next; //Advance LinkedList pointer
    }

  }//read the entire file into a LinkedList of file_blocks

  fclose(fin);

  #ifdef DEBUG
  printf("Finished reading file data\n");
  #endif

  int size = 0;

  listPtr = listStart; //Reset ListPtr to start of list
  while (listPtr){ //Return false when listPtr points to end of list
    size += listPtr->size;
    listPtr = listPtr->next; //Advance LinkedList pointer
  }

  char *fileText = malloc(size*sizeof(char)+1); //Allocate permanent memory
  fileText[0] = '\0';

  #ifdef DEBUG
  printf("To load %d bytes\n", size);
  #endif

  listPtr = listStart; //Reset listPtr
  while (listPtr != NULL){ //Loop through all list items
    strncat(fileText, listPtr->data, listPtr->size); //Read up to BLOCK_SIZE bytes from buffer

    #ifdef DEBUG
    printf("Loaded data: %s\n", listPtr->data);
    #endif

    struct file_block *next = listPtr->next;//Save address of next list item
    free(listPtr); //Free memory block of current list item
    // listPtr = 0;
    listPtr = next; //Advance LinkedList pointer
  }

  if (strlen(fileText) != size){
    printf("READFILE::WARNING: Loaded %zu/%d bytes\n", strlen(fileText), size);
    printf("File:\n%s", fileText);
  }
  #ifdef DEBUG
  else
    printf("Successfully loaded all data\n");
  #endif

  return fileText;
}//readFile
