
#ifndef GRABIMAGE_H
#define GRABIMAGE_H

struct VideoCapture;
struct Mat;

typedef struct VideoCapture VideoCapture;
typedef struct Mat Mat;

VideoCapture *openVideo(char file[]);   //Open video (.avi/.mp4/etc.) files
void closeVideo(VideoCapture *video);   //Release resources for VideoCapture

Mat *grabFrame  (VideoCapture *video);  //Load the new video frame into mat
void releaseMat (Mat *mat);

// Data member access functions for Mat objects
int matRows(Mat *mat);          // Height of image
int matCols(Mat *mat);          // Width of image
int matChannels(Mat *mat);      // RGB / RGBA etc.
unsigned char *matData(Mat *mat);//Pointer to raw matrix data

#endif
