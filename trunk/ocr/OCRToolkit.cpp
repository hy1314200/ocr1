#include "OCRToolkit.h"
#include "CharDivider.h"
#include "CharRecogniser.h"
#include "DebugToolkit.h"

#include <iostream>
#include <algorithm>

const double OCRToolkit::s_SCALETHRESHOLD = 0.05;

using namespace divide;
using namespace recognise;

int OCRToolkit::recognise(char* greys, int iWidth, int iHeight, vector<wchar_t>* res)
{
	vector<char*> picList;
	vector<int> widthList, heightList;

	if(!CharDivider::divideChar(greys, iWidth, iHeight, &picList, &widthList, &heightList)){
		return 0;
	}

	int len = picList.size();

	if(len == 0){
		return 0;
	}

	float reliably = 0;
	int resCount = 0;
	CharRecogniser* recogniser = CharRecogniser::buildInstance();

	if(!recogniser->isAvailable()){
		cerr << "ERROR: Classifier data file is not exist, please train it before classifying!" << endl;

		return 0;
	}

	wchar_t temp;
	for(int offset = 0; offset<len; offset++){
		//DebugToolkit::displayGreyImage(picList[offset], widthList[offset], heightList[offset]);
		reliably = recogniser->recogniseChar(picList[offset], widthList[offset], heightList[offset], &temp);

		// DebugToolkit::saveGreyImage(picList[offset], widthList[offset], heightList[offset], "image/(16).bmp");

		if(reliably > s_SCALETHRESHOLD){
			resCount++;

			res->push_back(temp);
		}
	}

	for(int i = 0; i<len; i++){
		delete[] picList.at(i);
	}

	return resCount;
}