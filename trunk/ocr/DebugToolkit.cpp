#include "DebugToolkit.h"
#include "CharDivider.h"

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

void DebugToolkit::displayGreyImage(const char* iData, int iWidth, int x, int y, int pW, int pH, int wait){
	IplImage* image = cvCreateImage(cvSize(pW, pH), 8, 1);

	int widthStep = image->widthStep;
	for(int i = 0; i<pH; i++){
		memcpy(image->imageData + widthStep*i, iData + iWidth*(y+i) + x, pW);
	}

	displayImage(image, wait);
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
	char charColor = divide::CharDivider::s_CHARACTERCOLOR;
	char backColor = divide::CharDivider::s_BACKGROUNDCOLOR;
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

bool DebugToolkit::isBinarized(IplImage* image){
	char charColor = divide::CharDivider::s_CHARACTERCOLOR;
	char backColor = divide::CharDivider::s_BACKGROUNDCOLOR;

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

	fscanf(file, "%d %d", &width, &height);
		
	for(int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			fscanf(file, "%d", data + i*width + j);
		}
	}

	fclose(file);
}

void DebugToolkit::writeGreyData(uchar* data, const char *name, int width, int height){
	FILE *file = fopen(name, "w");

	fprintf(file, "%d %d\n", width, height);
	for(int i = 0; i<height; i++){
		for(int j = 0; j<width; j++){
			fprintf(file, "%d\t", *(data + i*width + j));
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

void DebugToolkit::showHistogram(const float *fdata, int width, int wait){
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
	for(int i = 0; i<width; i++){
		int j = 0;
		int temp = cvRound(times[i]*count);
		for( ; j<temp; j++){
			*(imageData + (height - j - 1) * width + i) = 0;
		}
		for( ; j<height; j++){
			*(imageData + (height - j - 1) * width + i) = 255;
		}
	}

	displayImage(image, wait);

	delete[] times;
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

	showHistogram(fdata, width, wait);

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