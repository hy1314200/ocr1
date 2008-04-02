#include <iostream>
#include <string>

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

#include "OCRToolkit.h"
#include "CharDivider.h"
#include "CharRecogniser.h"
#include "OCRToolkit.h"
#include "FeatureExtracter.h"
#include "DebugToolkit.h"
#include "FontGen.h"

using namespace std;
using namespace recognise;
using namespace divide;
using namespace generate;

void testRecognise(char* path){
	const char* imagePath = path;
	IplImage* image = cvLoadImage(imagePath, CV_LOAD_IMAGE_GRAYSCALE);

	//DebugToolkit::displayImage(image);

	char* data = image->imageData;
	int width = image->width, height = image->height;

	// need modified DEBUG 
	cout << "约 " << cvRound(width*1.0f/height) << " 个字" << endl;

	// need modified DEBUG check if it's binarized
	for(int i = 0; i < width*height; i++){
		if(data[i] != 0 && data[i] != (char)255){
			cout << "\"" << imagePath << "\" is not binarized\n";

			system("pause");
			return;
		}
	}

	vector<wchar_t> wordList;
	int len = OCRToolkit::recognise(data, image->width, image->height, &wordList);
	
	cout << "识别出 " << len << " 个汉字\n";

	setlocale(LC_ALL, "");
	for(int i = 0; i<len; i++){
		wprintf(L"%c\n", wordList.at(i));
	}
	cout << endl;

	DebugToolkit::displayImage(image);
}

void binarize(const char* path){
	IplImage* image = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);

	char* data = image->imageData;
	int size = image->width*image->height;

	for(int i = 0; i<size; i++){
		data[i] = (data[i] >= 0)? 0: (char)255; 
	}

	cvSaveImage(path, image);
	cvReleaseImage(&image);
}

void testFeature1(char* path){
	IplImage* image = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);

	char* data = new char[image->width*image->height];
	for(int i = 0; i<image->height; i++){
		memcpy(data + image->width*i, image->imageData + image->widthStep*i, image->width);
	}

	CharRecogniser* rec = CharRecogniser::getInstance();
	rec->recogniseChar(data, image->width, image->height, NULL);

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
	double strokeProp[2][4][4];	// stroke proportion in two directions  
	int maxLocDirCtr[4][4][4];	// maximum local direction contributivity
	int totalBlackJump[2][8];	// black jump distribution in each balanced subvectors  
	double divBlackJump[2][8];	// black jump distribution in each balanced subvectors divided by the total 

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

void testFontGen(){
	FILE* file = fopen("data/font/songti.int", "r");
	assert(file != NULL);

	FontLib* lib = FontGen::genIntFontLib(file);

	fclose(file);
	DebugToolkit::displayGreyImage(lib->wideCharArray()->at(15)->imageData(), Char::s_CHARSIZE, Char::s_CHARSIZE);

	delete lib;
}

void testFontStore(){
	FILE* file = fopen("data/font/songti.ext", "r");
	assert(file != NULL);

	FontLib* lib = FontGen::genExtFontLib(file, SONGTI);

	fclose(file);

	lib->storeData("data/font/songti.int");
	DebugToolkit::displayGreyImage(lib->wideCharArray()->at(0)->imageData(), Char::s_CHARSIZE, Char::s_CHARSIZE);

	delete lib;
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

	CharRecogniser::getInstance()->TEST_distorteAndNorm(a, b, 16);

	cvReleaseImage(&image);

	for(int i = 0; i<16; i++){
		delete[] a[i];
	}
	delete[] a;
}

void unitTest(){
	char path[20];
	sprintf_s(path, "image/test/(%d).bmp", 8);

	//	testDistorte();
	//	testFontGen();
	//	testFontStore();
	//	testWChar();
	testFeature2();
	// 	testFilterNoise();

	// 	testRecognise(path);
	//	testFeature1(path);

	// 	for(int i = 1; i<=9; i++){
	// 		sprintf_s(path, "image/(%d).bmp", i);
	// 		cout << path << endl;
	// 		testRecognise(path);
	// 	}
}

void testApp(){
// 	FILE* file = fopen("data/font/st.c", "r");
// 	FontLib* lib = FontGen::genExtFontLib(file, SONGTI);
// 	//DebugToolkit::displayGreyImage(lib->wideCharArray()->at(0)->imageData(), Char::s_CHARSIZE, Char::s_CHARSIZE);
// 	fclose(file);
// 
// 	CharRecogniser::buildFeatureLib(lib, 1);
// 	delete lib;

	testRecognise("image/(9).bmp");
}

int main(int argc, char** argv){
	testApp();
//	unitTest();
	return 0;
}