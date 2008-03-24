#include "OCRToolkit.h"
#include "CharDivider.h"
#include "CharRecogniser.h"
#include "DebugToolkit.h"

#include <iostream>
#include <algorithm>

const float OCRToolkit::s_SCALETHRESHOLD = 0.85f;

using namespace divide;
using namespace recognise;

int OCRToolkit::recognise(char* greys, int iWidth, int iHeight, wchar_t** res)
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

	*res = new wchar_t[len];

	float reliably = 0;
	int resIndex = 0;
	CharRecogniser* recogniser = CharRecogniser::getInstance();
	for(int offset = 0; offset<len; offset++){
		reliably = recogniser->recogniseChar(picList[offset], widthList[offset], heightList[offset], *res + resIndex);

		// DebugToolkit::saveGreyImage(picList[offset], widthList[offset], heightList[offset], "image/(16).bmp");

		if(reliably > s_SCALETHRESHOLD){
			resIndex++;
		}
	}

	if(resIndex != len){
		delete[] (*res + resIndex);
	}

	return resIndex;
}