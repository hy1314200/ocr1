#include "OCRToolkit.h"
#include "CharDivider.h"
#include "CharRecogniser.h"
#include "OCRToolkit.h"
#include "FeatureExtracter.h"
#include "DebugToolkit.h"
#include "FontLib.h"
#include "LibManager.h"
#include "ConfigFile.h"
#include "GlobalCofig.h"

#include <iostream>
#include <string>
#include <cstring>

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace recognise;
using namespace divide;
using namespace library;
using namespace util;

void testRecognise(char* path){
	vector<wchar_t> wordList;
	OCRToolkit::recognise(path, wordList);
	
	int len = wordList.size();
	printf("识别出 %d 个汉字\n", len);

	wcout.imbue(locale("chs"));
	for(int i = 0; i<len; i++){
		wcout << wordList.at(i);
	}

	IplImage *image = cvLoadImage(path);
	DebugToolkit::displayImage(image);
}

void testFeature1(char* path){
	IplImage* image = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);

	char* data = new char[image->width*image->height];
	for(int i = 0; i<image->height; i++){
		memcpy(data + image->width*i, image->imageData + image->widthStep*i, image->width);
	}

	vector<wchar_t> wordList;
	OCRToolkit::recognise(data, image->width, image->height, wordList);

	cvReleaseImage(&image);
	delete[] data;
}

void testFeature2(){
	const int dim = 16;

	char* data = new char[dim*dim];

	DebugToolkit::readGreyData((uchar*)data, "data/test/image2.data");
	
	// the feature describes a character image 
	int strokeWidth;			// stroke width
	int totalStrokeLen;			// total stroke length
	int projHist[2][dim];		// projection histogram in two directions
	int transitions[2];			// number of transitions in two directions
	int strokeDensity[2][8];	// stroke density in two directions
	int peripheral[4][8][2];	// two peripheral features with four directions
	int locDir[4][4][4];		// local direction contributivity with four regions and four directions
	float strokeProp[2][4][4];	// stroke proportion in two directions  
	int maxLocDirCtr[4][4][4];	// maximum local direction contributivity
	int totalBlackJump[2][8];	// black jump distribution in each balanced subvectors  
	float divBlackJump[2][8];	// black jump distribution in each balanced subvectors divided by the total 

	FeatureExtracter* ext = FeatureExtracter::getInstance();
	//ext->TEST_calcStrokeWidthAndLen(data, &strokeWidth, &totalStrokeLen);
	//ext->TEST_calcTransDensAndPeri(data, transitions, strokeDensity, peripheral);
	ext->TEST_calcLocDirPropAndMaxLocDir(data, locDir, strokeProp, maxLocDirCtr);

	//cout << transitions[0] << " " << transitions[1] << endl;
	cout << strokeProp[1][2][3] << " ";
//  	for(int i = 0; i<2; i++){
// 		for(int j = 0; j<4; j++){
// 			for(int k = 0; k<4; k++){
// 				cout << strokeProp[i][j][k] << " ";
// 			}
// 			cout << endl;
// 		}
// 		cout << endl;
//  	}

//	FeatureExtracter::TEST_calcBlackJump(data, &totalBlackJump, &divBlackJump);

//  	for(int i = 0; i<8; i++){
//  		cout << peripheral[2][i][1] << " ";
//  	}

	//DebugToolkit::normalizeAndShowHist(f.strokeDensity[0], 8);

	delete[] data;
}

void testFilterNoise(){
	char* data = new char[64];

	DebugToolkit::readGreyData((uchar*)data, "image.data");
	CharDivider::filterNoise(data, 8, 1, 6);
	DebugToolkit::printGreyImage(data, 8, 8);
}

void testWChar(){
	setlocale(LC_ALL, "");
	wchar_t str[10];
	str[0] = L'中';
	str[1] = L'国';
	str[2] = 0x0032;
	str[3] = 0x0030;
	str[4] = 0x0030;
	str[5] = 0x0038;
	str[6] = L'\0';

	wprintf(L"%s\n", str);
}

void testDistorte(){
	IplImage* image = cvLoadImage("ctrl_s.bmp", 0);

	char b[64*64], **a = new char*[16];

	for(int i = 0; i<16; i++){
		a[i] = new char[64*64];
	}

	for(int i = 0; i<64; i++){
		memcpy(b+64*i, image->imageData + image->widthStep*i, 64);
	}

	CharRecogniser::buildInstance()->TEST_distorteAndNorm(a, b);

	cvReleaseImage(&image);

	for(int i = 0; i<16; i++){
		delete[] a[i];
	}
	delete[] a;
}

void testApp(){
	char str[40];
	for(int i = 1; i<=9; i++){ 
		sprintf(str, "image/(%d).bmp", i);

		testRecognise(str);
		cout << "\n" << endl;
	}

	/*
	for(int i = 1; i<=18; i++){
		sprintf(str, "image/test/(%d).bmp", i);

		testRecognise(str);
		cout << "\n" << endl;
	}*/
}

void unitTest(){

	try
	{
		ConfigFile *config = ConfigFile::parseConfig("haha.txt");
		config->store("heihei.txt");
	}
	catch (string &e)
	{
		cout << e << endl;
	}

// 	char path[20];
// 	sprintf_s(path, "image/test/(%d).bmp", 8);

// 	testDistorte();
 //	testFontGen();
// 	testWChar();
// 	testFeature2();
// 	testFilterNoise();
// 
// 	testRecognise(path);
// 	testFeature1(path);
// 
// 	for(int i = 1; i<=9; i++){
// 		sprintf_s(path, "image/(%d).bmp", i);
// 		cout << path << endl;
// 		testRecognise(path);
// 	}
}

int main(int argc, char** argv){
	//unitTest();
#if 1
	const char *usage = 
"usage:character -train|(-a filepath)|(-r filepath)|(-b filepath)\n\
	-train train classifier from current library\n\
	-a append library characters\n\
	-r recognise character image\n\
	-b binarize input image\n";

	if(argc < 2){
		cout << usage;
		return 1;
	}

	if(strcmp(argv[1], "-train") == 0){

		OCRToolkit::trainClassifier();
	}else if(strcmp(argv[1], "-a") == 0){
		if(argc < 3){
			cout << usage;
			return 1;
		}

		LibManager::appendChars(argv[2]);
	}else if(strcmp(argv[1], "-r") == 0){
		if(argc < 3){
			cout << usage;
			return 1;
		}

		vector<wchar_t> res;
		wcout.imbue(locale("chs"));
		bool success;

		for(int i = 2; i<argc; i++){
			res.clear();

			success = OCRToolkit::recognise(argv[i], res);
			if(!success){
				continue;
			}

			int len = res.size();
			cout << "识别出" << len << "个汉字\n";
			for(int i = 0; i<len; i++){
				wcout << res.at(i);
			}
			cout << "\n" << endl;
		}
	}else if(strcmp(argv[1], "-b") == 0){
		if(argc < 3){
			cout << usage;
			return 1;
		}

		IplImage *image = cvLoadImage(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
		DebugToolkit::binarize(image, 128);

		cvSaveImage(argv[2], image);

		cvReleaseImage(&image);
	}
#endif

	//testApp();
	return 0;
}