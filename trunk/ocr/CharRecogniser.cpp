#include "CharRecogniser.h"
#include "FeatureExtracter.h"
#include "DebugToolkit.h"
#include "OCRToolkit.h"
#include "FontGen.h"

#include <iostream>
#include <cxcore.h>

using namespace recognise;

CharRecogniser* CharRecogniser::s_instance = 0;
const char* CharRecogniser::s_CONFPATH = "data/classify/config";

CharRecogniser* CharRecogniser::buildInstance(){
	if(s_instance == 0){
		FILE* file = fopen(s_CONFPATH, "r");
		if(file == NULL){
			return NULL;
		}

		char type[10];
		fscanf(file, "%s", type);

		if(strcmp(type, "svm") == 0){
			s_instance = new SVMClassifier();
		}else if(strcmp(type, "mnn") == 0){
			s_instance = new MNNClassifier();
		}else{
			assert(false);
		}

		fclose(file);
	}

	return s_instance;
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

const char* SVMClassifier::s_MODELPATH = "data/classify/svm/svm.model";

SVMClassifier::SVMClassifier()
{
	FILE *file = fopen(s_MODELPATH, "r");

	if(file == NULL){
		m_model = NULL;
	}else{
		m_model = svm_load_model(s_MODELPATH);

		fclose(file);
	}
}

SVMClassifier::~SVMClassifier(void)
{
	if(m_model != NULL){
		svm_destroy_model(m_model);
		m_model = NULL;
	}
}

void SVMClassifier::trainAndSaveClassifier(struct svm_problem *prob){
	if(prob == NULL){
		cout << "m_problem == NULL" << endl;

		exit(-1);
	}

	struct svm_parameter *param = new struct svm_parameter;


	// default values
	param->svm_type = C_SVC;
	param->kernel_type = RBF;
	param->degree = 3;
	param->gamma = 0;	// 1/k
	param->coef0 = 0;
	param->nu = 0.5;
	param->cache_size = 100;
	param->C = 1;
	param->eps = 1e-3;
	param->p = 0.1;
	param->shrinking = 1;
	param->probability = 0;
	param->nr_weight = 0;
	param->weight_label = NULL;
	param->weight = NULL;

	const char *error = svm_check_parameter(prob, param);
	if(error != NULL){
		cerr << "ERROR: " << error << endl;

		exit(-1);
	}

	struct svm_model *model = svm_train(prob, param);

	svm_save_model(s_MODELPATH, model);

	svm_destroy_model(model);

	delete param;
}

void SVMClassifier::buildFeatureLib(generate::FontLib** fontLib, const int libSize)
{
	const int charCount = fontLib[0]->size();

#ifdef DEBUG
	for(int i = 1; i<libSize; i++){
		assert(charCount == fontLib[i]->size());
	}
#endif

	const int sampleSize = 16;
	const int normSize = FeatureExtracter::s_NORMSIZE;
	const int featureSize = FeatureExtracter::s_FEATURESIZE;

	char** imageData = new char*[sampleSize];
	for(int i = 0; i<sampleSize; i++){
		imageData[i] = new char[normSize*normSize];
	}

	float *featureData = new float[charCount*libSize*sampleSize*featureSize], *tempData = featureData;
	FeatureExtracter* extracter = FeatureExtracter::getInstance();

	printf("sample generating process:\n");
	for(int i = 0; i<charCount; i++){
		for(int j = 0; j<libSize; j++){
			distorteAndNorm(imageData, fontLib[j]->wideCharArray()->at(i)->imageData(), sampleSize);

			for(int k = 0; k<sampleSize; k++, tempData += featureSize){
				extracter->extractFeature(tempData, imageData[k], true);
			}
		}

		if(i%100 == 0){
			printf("%.2f%% finished\n", i*100*1.0/charCount);
		}
	}
	printf("100%% finished\n");

	extracter->saveData();

	int count = charCount*libSize*sampleSize;
	struct svm_problem *problem = new struct svm_problem;
	problem->l = count;
	problem->y = new double[count];
	problem->x = new struct svm_node*[count];

	printf("\nspace allocation process:\n");
	for(int i = 0; i<count; i++){
		problem->x[i] = new struct svm_node[featureSize + 1];

		if(i%4000 == 0){
			printf("%.2f%% finished\n", i*100*1.0/count);
		}
	}
	printf("100%% finished\n");

	printf("\nproblem building process:\n");
	tempData = featureData;
	for(int i = 0; i<count; i++, tempData += featureSize){
		extracter->scaleFeature(tempData);

		problem->y[i] = fontLib[0]->wideCharArray()->at(i/(libSize*sampleSize))->value();
		for(int j = 0; j<featureSize; j++){
			problem->x[i][j].index = j;
			problem->x[i][j].value= tempData[j]; 
		}

		problem->x[i][featureSize].index = -1;

		if(i%4000 == 0){
			printf("%.2f%% finished\n", i*100*1.0/count);
		}
	}
	printf("100%% finished\n");

#ifdef SAVE_PROBLEM

	FILE* file = fopen("data/classify/svm/problem", "w");
	assert(file != NULL);

	printf("\nsaving problem process:\n");
	for(int i = 0; i<problem->l; i++){
		fprintf(file, "%d", (int)problem->y[i]);

		for(int j = 0; problem->x[i][j].index != -1; j++){
			fprintf(file, " %d:%lf", problem->x[i][j].index, problem->x[i][j].value);
		}

		fprintf(file, "\n");

		if(i%4000 == 0){
			printf("%.2f%% finished\n", i*100*1.0/problem->l);
		}
	}
	printf("100%% finished\n");

	fclose(file);

#endif

	printf("problem saved\n");
	//trainAndSaveClassifier(problem);

	for(int i = 0; i<count; i++){
		delete[] problem->x[i];
	}
	delete[] problem->x;
	delete[] problem->y;
	delete problem;

	for(int i = 0; i<sampleSize; i++){
		delete[] imageData[i];
	}
	delete[] imageData;

	delete[] featureData;
}

double SVMClassifier::recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res)
{
	int normSize = FeatureExtracter::s_NORMSIZE;
	int featureSize = FeatureExtracter::s_FEATURESIZE;
	char* normChar = new char[normSize*normSize];

	normalize(normChar, greys, iWidth, 0, 0, iWidth, iHeight);

	float *scaledFeature = new float[featureSize];
	FeatureExtracter::getInstance()->extractScaledFeature(scaledFeature, normChar);

	struct svm_node *node = new struct svm_node[featureSize+1];
	for(int i = 0; i<featureSize; i++){
		node[i].index = i;
		node[i].value = scaledFeature[i];
	}
	node[featureSize].index = -1;

	int size = svm_get_nr_class(m_model);
	double *probability = new double[size];

	double maxProb;
	if(svm_check_probability_model(m_model)){
		*res = (wchar_t)svm_predict_probability(m_model, node, probability);

		maxProb = 0;
		for(int i = 0; i<size; i++){
			if(maxProb < probability[i]){
				maxProb = probability[i];
			}
		}
	}else{
		*res = (wchar_t)svm_predict(m_model, node);

		maxProb = 1;
	}

	delete[] normChar;
	delete[] scaledFeature;
	delete[] node;
	delete[] probability;

	return maxProb;
}

const char* MNNClassifier::s_MODELPATH = "data/classify/mnn/mnn.model";

MNNClassifier::MNNClassifier()
{
	FILE *file = fopen(s_MODELPATH, "r");

	if(file == NULL){

	}else{

		fclose(file);		
	}
}

MNNClassifier::~MNNClassifier(void)
{

}

void MNNClassifier::buildFeatureLib(generate::FontLib** fontLib, const int libSize)
{

}

double MNNClassifier::recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res)
{
	return 0;
}