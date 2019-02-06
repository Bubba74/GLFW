
#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

extern "C" {
	// C-API Interface
	
	VideoCapture *openVideo(char file[]);	//Open video (.avi/.mp4/etc.) files
	void closeVideo(VideoCapture *video);	//Release resources for VideoCapture

	Mat *grabFrame  (VideoCapture *video);	//Load the new video frame into mat
	void releaseMat (Mat *mat);

	// Data member access functions for Mat objects
	int matRows(Mat *mat);		// Height of image
	int matCols(Mat *mat);		// Width of image
	int matChannels(Mat *mat);	// RGB / RGBA etc.
	unsigned char *matData (Mat *mat); //Pointer to raw matrix data
}

VideoCapture *openVideo (char file[]){
	return new VideoCapture(file);
} // openVideo

void closeVideo (VideoCapture *video) {
	video->release();
} // closeVideo

Mat *grabFrame (VideoCapture *video) {
	//TODO Figure out how these resources are managed
	Mat *mat = new Mat();
	*video >> *mat;
	return mat;
} // grabFrame

void releaseMat (Mat *mat) {
	delete mat;
} // releaseMat

int matRows (Mat *mat) { return mat->rows; }
int matCols (Mat *mat) { return mat->cols; }
int matChannels (Mat *mat) { return mat->channels(); }
unsigned char *matData (Mat *mat) {
	return mat->data;
}

int debug () {
	VideoCapture cap("RollerCoaster360.mp4");

	Mat img;

	cap >> img;

	waitKey(10);
	imshow("frame", img);

	cout << "Dims: " << img.dims << endl;
	cout << "Rows: " << img.rows << endl;
	cout << "Cols: " << img.cols << endl;

	cout << "Flag: " << img.flags << endl;
	cout << "Dpth: " << img.depth() << endl;
	cout << "Channels: " << img.channels() << endl;
}
	
