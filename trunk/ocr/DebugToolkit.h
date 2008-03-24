#ifndef _DEBUGTOOLKIT_H
#define _DEBUGTOOLKIT_H

#include <cxcore.h>

class DebugToolkit
{
public:
	/** display the image and save it when press ctrl+s */
	static void displayImage(const IplImage* image, int wait = 0);

	/** display a region of the grey image and save it when press ctrl+s */
	static void displayGreyImage(const char* imageData, int iWidth, int x, int y, int pW, int pH, int wait = 0);

	/** display the grey image with width "width" and height "height" and save it when press ctrl+s */
	static void displayGreyImage(const char* imageData, int width, int height, int wait = 0){
		displayGreyImage(imageData, width, 0, 0, width, height, wait);
	}

	/** display the histogram recorded int the float array "data" with width "width" and save it when press ctrl+s */
	static void showHistogram(const float *data, int width, int wait = 0);

	static void printGreyImage(const char* data, int width, int height);


	static void saveGreyImage(const char* imageData, int width, int height, const char* filename);

	static bool isBinarized(const char* imageData, int width, int height);
	
	static bool isBinarized(IplImage* image);

	static void binarize(uchar* imageData, long size, uchar threshold);

	static void binarize(IplImage* image, uchar threshold);

	static void readGreyData(uchar* data, const char *name);

	static void writeGreyData(uchar* data, const char *name, int width, int height);

	static void normalizeAndShowHist(const int *data, int width, int wait = 0);

private:
	DebugToolkit(void){	}
	~DebugToolkit(void){ }
};

#endif