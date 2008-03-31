#include "CharRecogniser.h"
#include "stdafx.h"
#include "FeatureExtracter.h"
#include "DebugToolkit.h"
#include "OCRToolkit.h"
#include "FontGen.h"

#include <iostream>
#include <cxcore.h>

using namespace recognise;

CharRecogniser* CharRecogniser::s_instance = 0;
const char CharRecogniser::s_SVMDATAPATH[20] = "data/classifier/svm";

CharRecogniser* CharRecogniser::getInstance(){
	if(s_instance == 0){
		FILE* file = fopen(s_SVMDATAPATH, "r");

		if(file != NULL){
			s_instance = new CharRecogniser(file);
		}else{
			return NULL;
		}
	}

	return s_instance;
}

CharRecogniser::CharRecogniser(FILE* file)
{
	
}

CharRecogniser::~CharRecogniser(void)
{
	// release the character library
}

float CharRecogniser::recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res)
{
	int normSize = FeatureExtracter::s_NORMSIZE;
	int featureSize = FeatureExtracter::s_FEATURESIZE;
	char* normChar = new char[normSize*normSize];

	normalize(normChar, greys, iWidth, 0, 0, iWidth, iHeight);

	double *feature = new double[featureSize];
	FeatureExtracter::extractFeature(feature, normChar);

	CvMat* header = cvCreateMatHeader(1, featureSize, CV_64FC1);
	cvSetData(header, feature, featureSize);

	CvMat* y = cvCreateMat(1, W->cols, CV_64FC1);

	cvGEMM(W, header, 1, NULL, 0, y, CV_GEMM_B_T);

	int index = 0;// 对y进行分类

	*res = m_indexMapping.at(index);

	cvReleaseMatHeader(&header);
	cvReleaseMat(&y);
	delete[] normChar;
	delete[] feature;

	return 1;
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

	int size = fontLib[0].size();
	for(int i = 0; i<size; i++){
		m_indexMapping.push_back(fontLib->wideCharArray()->at(i)->value());
	}

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

void CharRecogniser::distorteAndNorm(char** samples, const char* prototypeOrigin, int sampleSize)
{
	int normSize = FeatureExtracter::s_NORMSIZE, count = 0;
	int x, y, width, height;
	char* data = new char[normSize*normSize], *prototype = new char[normSize*normSize];
	memcpy(prototype, prototypeOrigin, normSize*normSize);

	IplImage* src = cvCreateImageHeader(cvSize(normSize, normSize), 8, 1);
	IplImage* dst = cvCreateImageHeader(cvSize(normSize, normSize), 8, 1);
	cvSetData(src, prototype, normSize);
	cvSetData(dst, data, normSize);

	IplImage* temp = cvCreateImage(cvSize(normSize, normSize), 8, 1);

	/*
	CV_SHAPE_RECT, a rectangular element; 
	CV_SHAPE_CROSS, a cross-shaped element; 
	CV_SHAPE_ELLIPSE, an elliptic element;
	*/
	IplConvKernel* element = cvCreateStructuringElementEx( 2, 2, 0, 0, CV_SHAPE_ELLIPSE, 0 );

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayImage(src);
#endif

	/************************************************************************/
	/* the first group                                                      */
	/************************************************************************/
	cvErode(src, dst, element, 1);
	cvCopyImage(dst, temp);

	// normal
	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 1, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 1);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 2, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 2);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	/************************************************************************/
	/* the second group                                                     */
	/************************************************************************/
	cvDilate(src, dst, element, 1);
	cvCopyImage(dst, temp);
	
	// normal
	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 1, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 1);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 2, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 2);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 3, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 3);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 4, CV_INTER_LINEAR, threshold 110
	cvCopyImage(temp, dst);
	reduceResolution(dst, 4, CV_INTER_LINEAR, 110);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	/************************************************************************/
	/* the third group                                                      */
	/************************************************************************/
	cvDilate(src, dst);
	cvCopyImage(dst, temp);

	// normal
	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 2, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 2);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 3, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 3);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 4, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 4, CV_INTER_LINEAR);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	/************************************************************************/
	/* the fourth group                                                     */
	/************************************************************************/
	cvCopyImage(src, temp);

	// normal
	findXYWH(prototype, &x, &y, &width, &height);
	normalize(samples[count++], prototype, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 1, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 1);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 2, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 2);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	// scale 3, CV_INTER_NN, threshold 128
	cvCopyImage(temp, dst);
	reduceResolution(dst, 3, CV_INTER_LINEAR, 100);

	findXYWH(data, &x, &y, &width, &height);
	normalize(samples[count++], data, normSize, x, y, width, height);

#ifdef DISPLAY_DISTORTION
	DebugToolkit::displayGreyImage(samples[count-1], normSize, normSize);
#endif

	cvReleaseImageHeader(&src);
	cvReleaseImageHeader(&dst);
	cvReleaseImage(&temp);

	cvReleaseStructuringElement(&element);

	delete[] data;	
	delete[] prototype;

	assert(count == sampleSize);
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

void CharRecogniser::reduceResolution(IplImage* image, int scale, int type, int threshold){
	IplImage* temp;

	switch(scale){
	case 1:
		temp = cvCreateImage(cvSize(48, 48), 8, 1);
		break;

	case 2:
		temp = cvCreateImage(cvSize(32, 32), 8, 1);
		break;

	case 3:
		temp = cvCreateImage(cvSize(24, 24), 8, 1);
		break;

	case 4:
		temp = cvCreateImage(cvSize(16, 16), 8, 1);
		break;

	default:
		assert(false);

	}

	cvResize(image, temp);
	cvResize(temp, image, type);
	DebugToolkit::binarize(image, threshold);

	cvReleaseImage(&temp);
}