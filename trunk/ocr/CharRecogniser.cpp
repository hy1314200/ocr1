#include "CharRecogniser.h"
#include "FeatureExtracter.h"
#include "DebugToolkit.h"
#include "OCRToolkit.h"

#include <iostream>

#include <cxcore.h>
#include <cv.h>

using namespace recognise;

CharRecogniser* CharRecogniser::s_instance = 0;

CharRecogniser::CharRecogniser(void)
{
	// load the character library
}

CharRecogniser::~CharRecogniser(void)
{
	// release the character library
}

float CharRecogniser::recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res)
{
	char* normChar = normalize(greys, iWidth, iHeight);

	FeatureExtracter::temp(normChar);

	delete[] normChar;
	return 0;
}

char* CharRecogniser::normalize(const char* greys, int iWidth, int iHeight)
{
	int normSize = FeatureExtracter::s_NORMSIZE;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;
	float multiple;
	IplImage* dst = 0;

	IplImage* src = cvCreateImage(cvSize(iWidth, iHeight), 8, 1);
	int widthStep = src->widthStep;
	for(int i = 0; i<iHeight; i++){
		memcpy(src->imageData + widthStep*i, greys + iWidth*i, iWidth);
	}

	char* retData = new char[normSize*normSize];
	if(iWidth >= iHeight){
		multiple = normSize*1.0f/iWidth;
		int tempH = cvFloor(iHeight*multiple);
		dst = cvCreateImage(cvSize(normSize, tempH), 8, 1);

		// need modified: CV_INTER_NN
		cvResize(src, dst);
		DebugToolkit::binarize(dst, 128);

		int count = (normSize - tempH)/2;
		for(int i = 0; i<count; i++){
			memset(retData + normSize*i, backColor, normSize);
			memset(retData + normSize*(normSize-i-1), backColor, normSize);
		}

		if(tempH + count*2 < normSize){
			memset(retData + normSize*(normSize-count-1), backColor, normSize);
		}

		widthStep = dst->widthStep;
		for(int i = 0; i<tempH; i++){
			memcpy(retData + normSize*(count+i), dst->imageData + widthStep*i, normSize);
		}
	}else{
		multiple = normSize*1.0f/iHeight;
		int tempW = cvFloor(iWidth*multiple);
		dst = cvCreateImage(cvSize(tempW, normSize), 8, 1);

		cvResize(src, dst);
		DebugToolkit::binarize(dst, 128);

		int count = (normSize - tempW)/2;
		for(int i = 0; i<normSize; i++){
			for(int j = 0; j<count; j++){
				*(retData + normSize*i + j) = *(retData + normSize*i + normSize-j-1) = backColor;
			}
		}

		if(tempW + count*2 < normSize){
			for(int i = 0; i<normSize; i++){
				*(retData + normSize*i + normSize-count-1) = backColor;
			}
		}

		widthStep = dst->widthStep;
		for(int i = 0; i<normSize; i++){
			memcpy(retData + normSize*i + count, dst->imageData + widthStep*i, tempW);
		}
	}

	cvReleaseImage(&src);

	DebugToolkit::displayGreyImage(retData, normSize, normSize);

	cvReleaseImage(&dst);

	return retData;
}