#include "DebugToolkit.h"
#include "OCRToolkit.h"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>

#include <cv.h>
#include <highgui.h>

using namespace std;

void DebugToolkit::displayImage(const IplImage* image, int wait){
	char winName[] = "_window";
	cvNamedWindow(winName);

	cvShowImage(winName, image);

	char key;
	do{
		key = cvWaitKey(wait);

		if(key == 19){	// Ctrl + S
			cvSaveImage("ctrl_s.bmp", image);
		}
	}while(key == 19);

	cvDestroyWindow(winName);
}

/**
  * This is a function illustrating how to display more than one image in a single window using Intel OpenCV
  *
  * @param char *title: Title of the window to be displayed
  * @param int nArgs:   Number of images to be displayed
  * @param ...:         IplImage*, which contains the images
  *
  * The method used is to set the ROIs of a Single Big image and then resizing 
  * and copying the input images on to the Single Big Image. This function does not 
  * stretch the image, and it resizes the image without modifying the width/height ratio..
  * 
  * This function can be called like this:
  * displayManyImages("Images", 2, img1, img2);
  * or
  * displayManyImages("Images", 5, img2, img2, img3, img4, img5);
  *
  * This function can display upto 12 images in a single window. It does not check whether 
  * the arguments are of type IplImage* or not. The maximum window size is 700 by 660 pixels.
  * Does not display anything if the number of arguments is less than one or greater than 12.
  * 
  * Take care of the number of arguments you pass, and the type of arguments, which should be 
  * of type IplImage* ONLY. If you pass a pointer that is not IplImage*, Error will occur.
  */

void DebugToolkit::displayManyImage(char* title, int nArgs, ...) {

	// img - Used for getting the arguments 
	IplImage *img;

	// DispImage - the image in which input images are to be copied
	IplImage *DispImage;

	int size;
	int i;
	int m, n;
	int x, y;

	// w - Maximum number of images in a row 
	// h - Maximum number of images in a column 
	int w, h;

	// scale - How much we have to resize the image
	float scale;
	int max;

	// If the number of arguments is lesser than 0 or greater than 12
	// return without displaying 
	if(nArgs <= 0) {
		printf("Number of arguments too small....\n");
		return;
	}
	else if(nArgs > 12) {
		printf("Number of arguments too large....\n");
		return;
	}
	// Determine the size of the image, 
	// and the number of rows/cols 
	// from number of arguments 
	else if (nArgs == 1) {
		w = h = 1;
		size = 300;
	}
	else if (nArgs == 2) {
		w = 2; h = 1;
		size = 300;
	}
	else if (nArgs == 3 || nArgs == 4) {
		w = 2; h = 2;
		size = 300;
	}
	else if (nArgs == 5 || nArgs == 6) {
		w = 3; h = 2;
		size = 200;
	}
	else if (nArgs == 7 || nArgs == 8) {
		w = 4; h = 2;
		size = 200;
	}
	else {
		w = 4; h = 3;
		size = 150;
	}

	// Create a new 3 channel image
	DispImage = cvCreateImage( cvSize(100 + size*w, 60 + size*h), 8, 3 );

	// Used to get the arguments passed
	va_list args;
	va_start(args, nArgs);

	// Loop for nArgs number of arguments
	for (i = 0, m = 20, n = 20; i < nArgs; i++, m += (20 + size)) {

		// Get the Pointer to the IplImage
		img = va_arg(args, IplImage*);

		// Check whether it is NULL or not
		// If it is NULL, release the image, and return
		if(img == 0) {
			printf("Invalid arguments");
			cvReleaseImage(&DispImage);
			return;
		}

		// Find the width and height of the image
		x = img->width;
		y = img->height;

		// Find whether height or width is greater in order to resize the image
		max = (x > y)? x: y;

		// Find the scaling factor to resize the image
		scale = (float) ( (float) max / size );

		// Used to Align the images
		if( i % w == 0 && m!= 20) {
			m = 20;
			n+= 20 + size;
		}

		// Set the image ROI to display the current image
		cvSetImageROI(DispImage, cvRect(m, n, (int)( x/scale ), (int)( y/scale )));

		// Resize the input image and copy the it to the Single Big Image
		cvResize(img, DispImage);

		// Reset the ROI in order to display the next image
		cvResetImageROI(DispImage);
	}

	// Create a new window, and show the Single Big Image
	cvNamedWindow( title, 1 );
	cvShowImage( title, DispImage);

	cvWaitKey();
	cvDestroyWindow(title);

	// End the number of arguments
	va_end(args);

	// Release the Image Memory
	cvReleaseImage(&DispImage);
}

void DebugToolkit::displayGreyImage(const char* iData, int iWidth, int x, int y, int pW, int pH, int wait){
	IplImage* image = cvCreateImage(cvSize(pW, pH), 8, 1);

	int widthStep = image->widthStep;
	for(int i = 0; i<pH; i++){
		memcpy(image->imageData + widthStep*i, iData + iWidth*(y+i) + x, pW);
	}

	displayImage(image, wait);
	cvReleaseImage(&image);
}

void DebugToolkit::saveGreyImage(const char* imageData, int width, int height, const char* filename){
	IplImage* image = cvCreateImage(cvSize(width, height), 8, 1);

	int widthStep = image->widthStep;
	for(int i = 0; i<height; i++){
		memcpy(image->imageData + widthStep*i, imageData + width*i, width);
	}

	cvSaveImage(filename, image);
	cvReleaseImage(&image);
}

bool DebugToolkit::isBinarized(const char* imageData, int width, int height){
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;
	bool isBinarized = true;

	for(int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			if(*(imageData + width*i + j) != backColor && *(imageData + width*i + j) != charColor){
				std::cout << "(" << j << ", " << i << ") = " << (int)(uchar)*(imageData + width*i + j) << " not binarized\n";

				isBinarized = false;
			}
		}
	}

	return isBinarized;
}

bool DebugToolkit::isBinarized(const IplImage* image){
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	int width = image->width, height = image->height;
	char* data = image->imageData;

	bool isBinarized = true;
	for(int i = 0; i<height; i++){
		data = image->imageData + i*image->widthStep;

		for(int j = 0; j<width; j++){
			if(*(data + j) != backColor && *(data + j) != charColor){
				std::cout << "(" << j << ", " << i << ") = " << (int)(uchar)*(data + j) << " is not binarized\n";

				isBinarized = false;
			}
		}
	}

	return isBinarized;
}

void DebugToolkit::binarize(uchar* imageData, long size, uchar threshold){
	for(int i = 0; i<size; i++){
		if(*(imageData + i) < threshold){
			*(imageData + i) = 0;
		}else{
			*(imageData + i) = 255;
		}
	}
}

void DebugToolkit::binarize(IplImage* image, uchar threshold){
	int width = image->width, height = image->height;
	uchar* data;

	for(int i = 0; i<height; i++){
		data = (uchar*)(image->imageData + i*image->widthStep);

		for(int j = 0; j<width; j++){
			if(*(data + j) < threshold){
				*(data + j) = 0;
			}else{
				*(data + j) = 255;
			}
		}
	}
}

void DebugToolkit::readGreyData(uchar* data, const char *name){
	int width, height;

	FILE *file = fopen(name, "r");
	assert(file != NULL);

	fscanf(file, "%d %d", &width, &height);
	
	int temp;
	for(int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			fscanf(file, "%d", &temp);

			*(data + i*width + j) = (uchar)temp;
		}
	}

	fclose(file);
}

void DebugToolkit::writeGreyData(uchar* data, const char *name, int width, int height){
	FILE *file = fopen(name, "w");
	assert(file != NULL);

	fprintf(file, "%d %d\n", width, height);
	for(int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			fprintf(file, "%d\t", *(data + i*width + j));
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

void DebugToolkit::showNormHistogram(const float *fdata, int width, int wait){
	int height = width * 2 / 3; 

	IplImage *image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	uchar *imageData = (uchar*)image->imageData;

	float max = 0;
	float* times = new float[width];
	for(int i = 0; i<width; i++){
		times[i] = fdata[i] * height;
		max = (max > times[i])? max: times[i];
	}

	float count = height * 4 / (5 * max);
	int widthStep = image->widthStep;
	for(int i = 0; i<width; i++){
		int j = 0;
		int temp = cvRound(times[i]*count);
		for( ; j<temp; j++){
			*(imageData + (height - j - 1) * widthStep + i) = 0;
		}
		for( ; j<height; j++){
			*(imageData + (height - j - 1) * widthStep + i) = 255;
		}
	}

	displayImage(image, wait);

	delete[] times;
	cvReleaseImage(&image);
}

void DebugToolkit::normalizeAndShowHist(const int *data, int width, int wait){
	float* fdata = new float[width];

	int count = 0;
	for(int i = 0; i<width; i++){
		count += data[i];
	}

	for(int i = 0; i<width; i++){
		fdata[i] = data[i]*0.1f / count;
	}

	showNormHistogram(fdata, width, wait);

	delete[] fdata;
}

void DebugToolkit::printGreyImage(const char* data, int width, int height){
	for(int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			cout.width(4);
			cout << (int)(uchar)*(data + width*i + j);
		}

		cout << endl;
	}
}