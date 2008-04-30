#include "CharRecogniser.h"
#include "DebugToolkit.h"
#include "OCRToolkit.h"
#include "Classifier.h"
#include "FeatureExtracter.h"
#include "GlobalCofig.h"
#include "ConfigFile.h"
#include "FontLib.h"

using namespace recognise;
using namespace util;
using namespace library;

CharRecogniser* CharRecogniser::s_instance = NULL;

CharRecogniser* CharRecogniser::buildInstance(){
	if(s_instance == 0){
		ConfigFile *config = GlobalCofig::getConfigFile();

		string classifier = config->get("classifier");

		if(classifier == "svm"){
			s_instance = new SVMClassifier();
		}else if(classifier == "mnn"){
			s_instance = new MNNClassifier();
		}else{
			assert(false);
		}
	}

	return s_instance;
}

void CharRecogniser::trainClassifier()
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	FontLib *lib[FontLib::s_TYPEKIND];

	int offset = 0;
	if(config->getBool("typeface.songti")){
		lib[offset++] = FontLib::genFontLib(FontLib::SONGTI);
	}
	if(config->getBool("typeface.heiti")){
		lib[offset++] = FontLib::genFontLib(FontLib::HEITI);
	}
	if(config->getBool("typeface.fangsong")){
		lib[offset++] = FontLib::genFontLib(FontLib::FANGSONG);
	}
	if(config->getBool("typeface.kaiti")){
		lib[offset++] = FontLib::genFontLib(FontLib::KAITI);
	}
	if(config->getBool("typeface.lishu")){
		lib[offset++] = FontLib::genFontLib(FontLib::LISHU);
	}

	buildFeatureLib(lib, offset);

	// check in case that library typeface changed in the future
	assert(FontLib::s_TYPEKIND == 5);
}

double CharRecogniser::recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res)
{
	int normSize = FeatureExtracter::s_NORMSIZE;
	int featureSize = FeatureExtracter::s_FEATURESIZE;
	char* normChar = new char[normSize*normSize];

	normalize(normChar, greys, iWidth, 0, 0, iWidth, iHeight);

#ifdef DISPLAY_NORM_CHAR
	DebugToolkit::displayGreyImage(normChar, normSize, normSize);
#endif

	float *scaledFeature = new float[featureSize];
	FeatureExtracter::getInstance()->extractScaledFeature(scaledFeature, normChar);

// 	for(int i = 0; i<featureSize; i++){
// 		std::cout << scaledFeature[i] << std::endl;
// 	}

	double ret = s_instance->classify(scaledFeature, res);

	delete[] normChar;
	delete[] scaledFeature;

	return ret;
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

void CharRecogniser::distorteAndNorm(char** samples, const char* prototypeOrigin)
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
	/************************************************************************
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
#endif*/

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

	assert(count == s_sampleSize);
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