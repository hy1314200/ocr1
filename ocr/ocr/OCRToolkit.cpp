#include "OCRToolkit.h"
#include "CharDivider.h"
#include "DebugToolkit.h"

#include <iostream>
#include <algorithm>

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

const double OCRToolkit::s_SCALETHRESHOLD = 0.05;

using namespace divide;

void OCRToolkit::recognise(char* greys, int iWidth, int iHeight, vector<wchar_t> &res)
{
	vector<char*> picList;
	vector<int> widthList, heightList;

	if(!CharDivider::divideChar(greys, iWidth, iHeight, &picList, &widthList, &heightList)){
		return;
	}

	int len = picList.size();

	if(len == 0){
		return;
	}

	float reliably = 0;
	CharRecogniser* recogniser = CharRecogniser::buildInstance();

	if(!recogniser->isAvailable()){
		cout << "ERROR: Classifier data file is not exist, please train it before classifying!" << endl;

		return;
	}

	wchar_t temp;
	for(int offset = 0; offset<len; offset++){
		//DebugToolkit::displayGreyImage(picList[offset], widthList[offset], heightList[offset]);
		reliably = recogniser->recogniseChar(picList[offset], widthList[offset], heightList[offset], &temp);

		// DebugToolkit::saveGreyImage(picList[offset], widthList[offset], heightList[offset], "image/(16).bmp");

		if(reliably > s_SCALETHRESHOLD){
			res.push_back(temp);
		}
	}

	for(int i = 0; i<len; i++){
		delete[] picList.at(i);
	}

	return;
}

 bool OCRToolkit::recognise(const char *filePath, vector<wchar_t> &res)
{
	IplImage* image = cvLoadImage(filePath, CV_LOAD_IMAGE_GRAYSCALE);
	if(image == NULL){
		cout << "ERROR: can not find file \"" << filePath << "\"" << endl;
		return false;
	}

	if(!DebugToolkit::isBinarized(image)){
		cout << "ERROR: \"" << filePath << "\" is not binarized" << endl;

		cvReleaseImage(&image);
		return false;
	}

	int width = image->width, height = image->height;
	char *data = new char[width*height];

	for(int i = 0; i<height; i++){
		memcpy(data + width*i, image->imageData + image->widthStep*i, width);
	}

	recognise(data, image->width, image->height, res);

#ifdef DISPLAY_DIVIDED_CHAR_IN_TOTAL
	DebugToolkit::displayGreyImage(data, image->width, image->height);
#endif

#ifdef DISPLAY_ORIGIN_IMAGE
	DebugToolkit::displayImage(image);
#endif

	cvReleaseImage(&image);
	delete[] data;

	return true;
}