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
	int resIndex = 0;
	CharRecogniser* recogniser = CharRecogniser::getInstance();

	if(recogniser == NULL){
		cerr << "Fail to initialize CharRecogniser: Can not find classifier data file!" << endl;

		return 0;
	}

	wchar_t temp;
	for(int offset = 0; offset<len; offset++){
		reliably = recogniser->recogniseChar(picList[offset], widthList[offset], heightList[offset], &temp);

		// DebugToolkit::saveGreyImage(picList[offset], widthList[offset], heightList[offset], "image/(16).bmp");

		if(reliably > s_SCALETHRESHOLD){
			resIndex++;

			res->push_back(temp);
		}
	}

	return resIndex;
}