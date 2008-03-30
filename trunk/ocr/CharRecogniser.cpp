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

	normalize(normChar, greys, iWidth, 0, 0, iWidth, iHeight);

	delete[] normChar;
	return 0;
}

void CharRecogniser::normalize(char* res, const char* greys, int iWidth, int x, int y, int width, int height)
{
	int normSize = FeatureExtracter::s_NORMSIZE;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	float multiple;
	IplImage* dst = NULL;

	IplImage* src = cvCreateImage(cvSize(width, height), 8, 1);
	int widthStep = src->widthStep;
	for(int i = 0; i<height; i++){
		memcpy(src->imageData + widthStep*i, greys + iWidth*(i+y) + x, width);
	}

	if(width >= height){
		multiple = normSize*1.0f/width;
		int tempH = cvFloor(height*multiple);
		dst = cvCreateImage(cvSize(normSize, tempH), 8, 1);

		// need modified: CV_INTER_NN
		cvResize(src, dst);
		DebugToolkit::binarize(dst, 128);

		int count = (normSize - tempH)/2;
		for(int i = 0; i<count; i++){
			memset(res + normSize*i, backColor, normSize);
			memset(res + normSize*(normSize-i-1), backColor, normSize);
		}

		if(tempH + count*2 < normSize){
			memset(res + normSize*(normSize-count-1), backColor, normSize);
		}

		widthStep = dst->widthStep;
		for(int i = 0; i<tempH; i++){
			memcpy(res + normSize*(count+i), dst->imageData + widthStep*i, normSize);
		}
	}else{
		multiple = normSize*1.0f/height;
		int tempW = cvFloor(width*multiple);
		dst = cvCreateImage(cvSize(tempW, normSize), 8, 1);

		cvResize(src, dst);
		DebugToolkit::binarize(dst, 128);

		int count = (normSize - tempW)/2;
		for(int i = 0; i<normSize; i++){
			for(int j = 0; j<count; j++){
				*(res + normSize*i + j) = *(res + normSize*i + normSize-j-1) = backColor;
			}
		}

		if(tempW + count*2 < normSize){
			for(int i = 0; i<normSize; i++){
				*(res + normSize*i + normSize-count-1) = backColor;
			}
		}

		widthStep = dst->widthStep;
		for(int i = 0; i<normSize; i++){
			memcpy(res + normSize*i + count, dst->imageData + widthStep*i, tempW);
		}
	}

	cvReleaseImage(&src);
	cvReleaseImage(&dst);
}

void CharRecogniser::buildFeatureLib(generate::FontLib* fontLib, const int libSize)
{

	const int charCount = fontLib[0].size();

#ifdef DEBUG
	for(int i = 1; i<libSize; i++){
		assert(charCount == fontLib[i].size());
	}
#endif

	const int sampleSize = 16;
	const int normSize = FeatureExtracter::s_NORMSIZE;
	const int featureSize = FeatureExtracter::s_FEATURESIZE;

	char** imageData = new char*[libSize];
	for(int i = 0; i<charCount; i++){
		imageData[i] = new char[normSize*normSize];
	}

	CvMat* src = cvCreateMat(sampleSize*libSize, featureSize, CV_64FC1);
	CvMat* mi = cvCreateMat(charCount, featureSize, CV_64FC1);
	CvMat* header1 = cvCreateMatHeader(1, featureSize, CV_64FC1);
	CvMat* header2 = cvCreateMatHeader(1, featureSize, CV_64FC1);

	double* featureData = NULL;
	for(int i = 0; i<charCount; i++){
		for(int j = 0; j<libSize; j++){
			distorteAndNorm(imageData, fontLib[j].wideCharArray()->at(i)->imageData(), sampleSize);

			featureData = src->data.db + src->step*sampleSize*j;
			for(int k = 0; k<sampleSize; k++){
				FeatureExtracter::extractFeature(featureData + src->step*k, imageData[k]);
			}
		}

		cvReduce(src, cvGetRow(mi, header1, i), 0);
	}

	CvMat* res = cvCreateMat(featureSize, featureSize, CV_64FC1);
	CvMat* Si = cvCreateMat(featureSize, featureSize, CV_64FC1);
	cvSetZero(Si);

	CvMat* Sw = cvCloneMat(Si);
	CvMat* Sb = cvCloneMat(Si);

	int srcInter = sampleSize*libSize;
	for(int i = 0; i<charCount; i++){
		for(int j = 0; j<srcInter; j++){
			cvMulTransposed(cvGetRow(src, header1, srcInter*i + j), res, 1, cvGetRow(mi, header2, i));

			cvAdd(res, Si, Si);
		}

		cvAdd(Si, Sw, Sw);
	}

	CvMat* m = cvCreateMat(1, featureSize, CV_64FC1);
	cvSetZero(m);

	for(int i = 0; i<charCount; i++){
		cvAdd(m, cvGetRow(mi, header1, i), m);
	}
	
	CvMat* temp = cvCloneMat(m);
	cvSet(temp, cvScalar(1.0/charCount));

	cvMul(m, temp, m);

	for(int i = 0; i<charCount; i++){
		cvMulTransposed(cvGetRow(mi, header1, i), res, 1, m);

		cvAdd(res, Sb, Sb);
	}

	cvSet(temp, cvScalar(srcInter));

	cvMul(m, temp, m);

	cvReleaseMat(&temp);

	cvReleaseMat(&src);
	cvReleaseMat(&mi);
	cvReleaseMat(&m);
	cvReleaseMatHeader(&header1);
	cvReleaseMatHeader(&header2);
	cvReleaseMat(&res);
	cvReleaseMat(&Si);
	cvReleaseMat(&Sb);

	for(int i = 0; i<charCount; i++){
		delete[] imageData[i];
	}
	delete[] imageData;
}

void CharRecogniser::distorteAndNorm(char** samples, char* prototype, int sampleSize)
{
	int normSize = FeatureExtracter::s_NORMSIZE, count = 0;
	int x, y, width, height;
	char* data = new char[normSize*normSize];

	IplImage* src = cvCreateImageHeader(cvSize(normSize, normSize), 8, 1);
	IplImage* dst = cvCreateImageHeader(cvSize(normSize, normSize), 8, 1);

	IplImage* scale[4];
	scale[0] = cvCreateImage(cvSize(48, 48), 8, 1);
	scale[1] = cvCreateImage(cvSize(32, 32), 8, 1);
	scale[2] = cvCreateImage(cvSize(24, 24), 8, 1);
	scale[3] = cvCreateImage(cvSize(16, 16), 8, 1);

	cvSetData(src, prototype, normSize);	
	cvSetData(dst, data, normSize);

	DebugToolkit::displayImage(src);

	/*
	CV_SHAPE_RECT, a rectangular element; 
	CV_SHAPE_CROSS, a cross-shaped element; 
	CV_SHAPE_ELLIPSE, an elliptic element;
	*/
	IplConvKernel* element = cvCreateStructuringElementEx( 2, 2, 0, 0, CV_SHAPE_ELLIPSE, 0 );

	cvErode(src, dst, element, 1);

	//findXYWH(data, &x, &y, &width, &height);
	//normalize(samples[count++], data, normSize, x, y, width, height);

	//DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);


	cvDilate(src, dst, element, 1);

	cvResize(src, scale[0]);
	cvResize(scale[0], src, CV_INTER_NN);
	DebugToolkit::binarize(src, 128);
	//DebugToolkit::displayImage(scale[0]);
	DebugToolkit::displayImage(src);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

	cvReleaseStructuringElement(&element);

	//DebugToolkit::displayImage(src);
	//DebugToolkit::displayImage(dst);

	cvDilate(src, dst);

	//findXYWH(data, &x, &y, &width, &height);
	//normalize(samples[count++], data, normSize, x, y, width, height);

	//DebugToolkit::displayImage(dst);

	//findXYWH(data, &x, &y, &width, &height);
	//normalize(samples[count++], data, normSize, x, y, width, height);
// 	int multiple = 4;
// 	char value;
// 	for(int i = 0; i<64/multiple; i++){
// 		for(int j = 0;j<64/multiple; j++){
// 			value = *(temp->imageData + temp->widthStep*(multiple*i) + multiple*j); 
// 
// 			for(int k = 0; k<multiple; k++){
// 				for(int l = 0; l<multiple; l++){
// 					*(temp->imageData + temp->widthStep*(multiple*i+l) + multiple*j+k) = value; 
// 				}
// 			}
// 		}
// 	}


	cvReleaseImageHeader(&src);
	cvReleaseImageHeader(&dst);
	for(int i = 0; i<4; i++){
		cvReleaseImageHeader(&scale[i]);
	}
	

	//assert(count == sampleSize+1);
}

void CharRecogniser::findXYWH(char* data, int* x, int* y, int* width, int* height){
	int normSize = FeatureExtracter::s_NORMSIZE;
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	bool needBreak = false;

	for(int i = 0; i<normSize; i++){
		for(int j = 0; j<normSize; j++){
			if(*(data + normSize*i + j) == charColor){
				*y = i;

				needBreak = true;
				break;
			}
		}

		if(needBreak){
			break;
		}
	}

	needBreak = false;
	for(int i = normSize-1; i>=0; i--){
		for(int j = 0; j<normSize; j++){
			if(*(data + normSize*i + j) == charColor){
				*height = i - *y + 1;

				needBreak = true;
				break;
			}
		}

		if(needBreak){
			break;
		}
	}

	needBreak = false;
	for(int j = 0; j<normSize; j++){
		for(int i = 0; i<normSize; i++){
			if(*(data + normSize*i + j) == charColor){
				*x = j;

				needBreak = true;
				break;
			}
		}

		if(needBreak){
			break;
		}
	}

	needBreak = false;
	for(int j = normSize-1; j>=0; j--){
		for(int i = 0; i<normSize; i++){
			if(*(data + normSize*i + j) == charColor){
				*width = j - *x + 1;

				needBreak = true;
				break;
			}
		}

		if(needBreak){
			break;
		}
	}
}