#include <iostream>
#include <string>

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

#include "OCRToolkit.h"
#include "CharDivider.h"
#include "CharRecogniser.h"
#include "FeatureExtracter.h"
#include "DebugToolkit.h"

using namespace std;

#include "OCRToolkit.h"

void testRecognise(char* path){
	const char* imagePath = path;
	IplImage* image = cvLoadImage(imagePath, CV_LOAD_IMAGE_GRAYSCALE);

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

	wchar_t* wordList;
	int len = OCRToolkit::recognise(data, image->width, image->height, &wordList);
	if(len == 0){
		cout << "识别出 0 个汉字\n";
	}

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

	using namespace recognise;

	CharRecogniser* rec = CharRecogniser::getInstance();
	rec->recogniseChar(data, image->width, image->height, NULL);

	cvReleaseImage(&image);
	delete[] data;
}

void testFeature2(){
	char* data = new char[64];

	DebugToolkit::readGreyData((uchar*)data, "image.data");

	using namespace recognise;
	//FeatureExtracter::Feature f;
	//FeatureExtracter::calcBlackJump(data, f.totalBlackJump, f.divBlackJump);

// 	for(int i = 0; i<8; i++){
// 		cout << f.peripheral[2][i][1] << " ";
// 	}

	//DebugToolkit::normalizeAndShowHist(f.strokeDensity[0], 8);

	delete[] data;
}

void testFilterNoise(){
	char* data = new char[64];

	DebugToolkit::readGreyData((uchar*)data, "image.data");
	using namespace divide;
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

int main(int argc, char** argv){
	char path[20];
  	sprintf_s(path, "image/(%d).bmp", 3);
	testWChar();
//	testFeature2();
// 	testFilterNoise();

//	testRecognise(path);
//	testFeature1(path);

// 	for(int i = 1; i<=9; i++){
// 		sprintf_s(path, "image/(%d).bmp", i);
// 		cout << path << endl;
// 		testRecognise(path);
// 	}

	return 0;
}