#include "CharRecogniser.h"
#include "stdafx.h"
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
	int normSize = FeatureExtracter::s_NORMSIZE;
	char* normChar = new char[normSize*normSize];

	normalize(normChar, greys, iWidth, iHeight);

	delete[] normChar;
	return 0;
}

void CharRecogniser::normalize(char* dst, const char* greys, int iWidth, int iHeight)
{
	int normSize = FeatureExtracter::s_NORMSIZE;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;
	float multiple;
	IplImage* tdst = NULL;

	IplImage* src = cvCreateImage(cvSize(iWidth, iHeight), 8, 1);
	int widthStep = src->widthStep;
	for(int i = 0; i<iHeight; i++){
		memcpy(src->imageData + widthStep*i, greys + iWidth*i, iWidth);
	}

	if(iWidth >= iHeight){
		multiple = normSize*1.0f/iWidth;
		int tempH = cvFloor(iHeight*multiple);
		tdst = cvCreateImage(cvSize(normSize, tempH), 8, 1);

		// need modified: CV_INTER_NN
		cvResize(src, tdst);
		DebugToolkit::binarize(tdst, 128);

		int count = (normSize - tempH)/2;
		for(int i = 0; i<count; i++){
			memset(dst + normSize*i, backColor, normSize);
			memset(dst + normSize*(normSize-i-1), backColor, normSize);
		}

		if(tempH + count*2 < normSize){
			memset(dst + normSize*(normSize-count-1), backColor, normSize);
		}

		widthStep = tdst->widthStep;
		for(int i = 0; i<tempH; i++){
			memcpy(dst + normSize*(count+i), tdst->imageData + widthStep*i, normSize);
		}
	}else{
		multiple = normSize*1.0f/iHeight;
		int tempW = cvFloor(iWidth*multiple);
		tdst = cvCreateImage(cvSize(tempW, normSize), 8, 1);

		cvResize(src, tdst);
		DebugToolkit::binarize(tdst, 128);

		int count = (normSize - tempW)/2;
		for(int i = 0; i<normSize; i++){
			for(int j = 0; j<count; j++){
				*(dst + normSize*i + j) = *(dst + normSize*i + normSize-j-1) = backColor;
			}
		}

		if(tempW + count*2 < normSize){
			for(int i = 0; i<normSize; i++){
				*(dst + normSize*i + normSize-count-1) = backColor;
			}
		}

		widthStep = tdst->widthStep;
		for(int i = 0; i<normSize; i++){
			memcpy(dst + normSize*i + count, tdst->imageData + widthStep*i, tempW);
		}
	}

	cvReleaseImage(&src);

	DebugToolkit::displayGreyImage(dst, normSize, normSize);

	cvReleaseImage(&tdst);
}

void CharRecogniser::buildFeatureLib(generate::FontLib* fontLib, int size)
{

	int count = fontLib[0].size();

#ifdef DEBUG
	for(int i = 1; i<size; i++){
		assert(count == fontLib[i].size());
		count = fontLib[i].size();
	}
#endif

	const int SAMPLESIZE = 100;
	char** imageData = new char*[size];
	for(int i = 0; i<count; i++){
		imageData[i] = new char[FeatureExtracter::s_NORMSIZE*FeatureExtracter::s_NORMSIZE];
	}

	CvMat* mat = cvCreateMat(1, FeatureExtracter::s_FEATURESIZE, CV_64FC1);
	for(int i = 0; i<count; i++){
		for(int j = 0; j<size; j++){
			distorte(imageData, fontLib[j].wideCharArray()->at(i)->imageData(), SAMPLESIZE);
		}
	}

	for(int i = 0; i<count; i++){
		delete[] imageData[i];
	}
	delete[] imageData;
}

void CharRecogniser::distorte(char** samples, char* prototype, int sampleSize)
{
	int normSize = FeatureExtracter::s_NORMSIZE, count = 0;

	IplImage* src = cvCreateImageHeader(cvSize(normSize, normSize), 8, 1);
	IplImage* dst = cvCreateImageHeader(cvSize(normSize, normSize), 8, 1);
	cvSetData(src, prototype, normSize);

	IplImage* temp = cvCreateImage(cvSize(normSize, normSize), 8, 1);
	
	cvSetData(dst, samples[count++], normSize);

	/*
	CV_SHAPE_RECT, a rectangular element; 
	CV_SHAPE_CROSS, a cross-shaped element; 
	CV_SHAPE_ELLIPSE, an elliptic element;
	*/
	IplConvKernel* element = cvCreateStructuringElementEx( 2, 2, 0, 0, CV_SHAPE_ELLIPSE, 0 );
	cvErode(src, dst, element, 1);
	cvReleaseStructuringElement(&element);

	DebugToolkit::displayImage(src);
	DebugToolkit::displayImage(dst);

	cvDilate(src, dst);

	DebugToolkit::displayImage(src);
	DebugToolkit::displayImage(dst);


	cvReleaseImageHeader(&src);
	cvReleaseImageHeader(&dst);
	cvReleaseImageHeader(&temp);
}