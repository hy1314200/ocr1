#include "CharDivider.h"
#include "DebugToolkit.h"
#include "OCRToolkit.h"

#include <iostream>
#include <algorithm>

#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace divide;

bool CharDivider::divideChar(char* greys, int iWidth, int iHeight, vector<char*>* picList, vector<int>* widthList, vector<int>* heightList)
{
	char charColor = OCRToolkit::s_CHARACTERCOLOR;

	int upside, downside;
	if(preProcess(greys, iWidth, iHeight, &upside, &downside) == false){
		return false;
	}

#ifdef FILTER_NOISE
	//filterNoise(greys, iWidth, upside, downside, true);
	filterNoise(greys, iWidth, upside, downside, false);
#endif

	int cWidth, cHeight;
	cHeight = downside - upside + 1;

	// horizontal process
	int* histgram = new int[iWidth];

	for(int i = 0; i<iWidth; i++){
		histgram[i] = 0;
	}

	for(int i = upside; i<=downside; i++){
		for(int j = 0; j<iWidth; j++){
			if(*(greys + iWidth*i + j) == charColor){
				histgram[j]++;
			}
		}
	}

	int dWidth;
	vector<int> cWlist, dWlist, offList;
	// need modified: do we need dWidth?
	if(divideHelp(histgram, iWidth, &cWidth, &dWidth, &offList, &cWlist, &dWlist) == false){
		return false;
	}
	assert(offList.size() == cWlist.size());

	if(PROPER != scale(cWidth, cHeight, LOOSE)){
		cWidth = cHeight;
	}

	int size = offList.size(), x, y, cWt, cHt;
	Scale sc;
	bool needBreak = false;
	char* alloc = 0;
	for(int count = 0; count<size; count++){
		cWt = cWlist[count];

		if(cWt < 1){	// ignore block only take one pixel wide
			continue;
		}

		x = offList[count];
		y = upside;
		cHt = cHeight;

		needBreak = false;
		while(!needBreak){
			findUpAndDown(greys, iWidth, &x, &y, &cWt, &cHt);
			sc = scale(cWt, cWidth, CHINESE);	// need modofied: only consider Chinese here
			switch(sc){
			case PROPER:
				alloc = new char[cWt*cHt];
				copyArea(alloc, greys, iWidth, x, y, cWt, cHt);

				(*picList).push_back(alloc);
				(*widthList).push_back(cWt);
				(*heightList).push_back(cHt);

				needBreak = true;

				DEBUG_markChar(greys, iWidth, iHeight, x, y, cWt, cHt);

#ifdef DISPLAY_DIVIDED_CHAR
				DebugToolkit::displayGreyImage(alloc, cWt, cHt);
#endif

				break;

			case LEFT_TOOBIG:
				if(cWt < 2*cWidth){
					int* temp = new int[cWt];
					for(int i = 0; i<cWt; i++){
						temp[0] = 0;
					}

					for(int i = 0; i<cHt; i++){
						for(int j = 0; j<cWt; j++){
							if(*(greys + iWidth*(y + i) + x + j) == charColor){
								temp[j]++;
							}
						}
					}

					int offset = findTroughFromSide(temp, cWt);

					if(offset <= (cWt + 1)/2){
						x = x + offset + 1;
					}
					cWt = cWt - offset - 1;

					delete[] temp;
				}else{
					// 按照 x 到 x + cWidth 和 x + cWt - cWidth 到 x + cWt 提取单字，即取可能的前后部分
					alloc = new char[cWidth*cHt];
					copyArea(alloc, greys, iWidth, x, y, cWidth, cHt);

					(*picList).push_back(alloc);
					(*widthList).push_back(cWidth);
					(*heightList).push_back(cHt);

					DEBUG_markChar(greys, iWidth, iHeight, x, y, cWidth, cHt);

#ifdef DISPLAY_DIVIDED_CHAR
					DebugToolkit::displayGreyImage(alloc, cWidth, cHt);
#endif

					alloc = new char[cWidth*cHt];
					copyArea(alloc, greys, iWidth, x + cWt - cWidth, y, cWidth, cHt);

					(*picList).push_back(alloc);
					(*widthList).push_back(cWidth);
					(*heightList).push_back(cHt);

					DEBUG_markChar(greys, iWidth, iHeight, x + cWt - cWidth, y, cWidth, cHt);

#ifdef DISPLAY_DIVIDED_CHAR
					DebugToolkit::displayGreyImage(alloc, cWidth, cHt);
#endif

					needBreak = true;
				}

				break;

			case LEFT_TOOSMALL:
				if(!isSign(greys, x, y, cWt, cHt)){
					if(x + cWidth < iWidth && histgram[x + cWidth] == 0){
						int i = 1;
						while(histgram[x + cWidth - i] == 0){ 
							i++;
						}
						cWt = cWidth-i+1;

						int tempX, tempY, tempW, tempH;
						while(count + 1 < size && offList[count + 1] < x + cWt){
							tempX = offList[count + 1];
							tempY = upside;
							tempW = cWlist[count + 1];
							tempH = cHeight;

							findUpAndDown(greys, iWidth, &tempX, &tempY, &tempW, &tempH);
							if(cHt < tempH){
								y = tempY;
								cHt = tempH;
							}

							count++;
						}

						alloc = new char[cWt*cHt];
						copyArea(alloc, greys, iWidth, x, y, cWt, cHt);

						(*picList).push_back(alloc);
						(*widthList).push_back(cWt);
						(*heightList).push_back(cHt);

						DEBUG_markChar(greys, iWidth, iHeight, x, y, cWt, cHt);

#ifdef DISPLAY_DIVIDED_CHAR
						DebugToolkit::displayGreyImage(alloc, cWt, cHt);
#endif

					}else{
						// 想想怎么处理
					}
				}else{
					// 处理符号

					DEBUG_markChar(greys, iWidth, iHeight, x, y, cWt, cHt);
				}
				needBreak = true;

				break;
			}
		}
	}

	delete[] histgram;

	return true;
}

bool CharDivider::preProcess(const char* greys, int iWidth, int iHeight, int* upside, int* downside)
{
	int threshold = cvCeil(iWidth/iHeight*1.3);

	if(horizontalRejectCalc(greys, iWidth, iHeight, upside, downside, threshold) == false){
		return false;
	}

	char* newGreys = removeBigConnectedComp(greys, iWidth, *upside, *downside);
	if(newGreys != NULL){
		int tempUp, tempDown;

		// need modified: the threshold related parameter 1.1
		if(horizontalRejectCalc(newGreys, iWidth, *downside - *upside + 1, &tempUp, &tempDown, cvCeil(iWidth/iHeight*1.1)) == false){
			return false;
		}

		*upside += tempUp;
		*downside = *upside + tempDown - tempUp;

		delete[] newGreys;
	}

	return true;
}

bool CharDivider::horizontalRejectCalc(const char* greys, int iWidth, int iHeight, int* upside, int* downside, int threshold){
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	
	int* histagram = new int[iHeight];
	for(int i = 0; i<iHeight; i++){
		histagram[i] = 0;
	}

	for(int i = 0; i<iHeight; i++){
		for(int j = 0; j<iWidth; j++){
			if(*(greys + iWidth*i + j) == charColor){
				histagram[i]++;
			}
		}
	}

	// need modified DEBUG
// 	cout << "horizontal projection: ";
// 	for(int i = 0; i<iHeight; i++){
// 		cout << histagram[i] << " ";
// 	}
// 	cout << endl;

	int up, down, center = iHeight/2;
	up = down = center;	// need modified 默认图像水平中心线必穿过字符
	while(up >= 0 && histagram[up] >= threshold){
		up--;
	}

	down++;
	while(down < iHeight && histagram[down] >= threshold){
		down++;
	}

	if(down - up <= 2){
		return false;
	}

	*upside = up + 1;
	*downside = down - 1;

	delete[] histagram;

	return true;
}

char* CharDivider::removeBigConnectedComp(const char* greys, int iWidth, int upside, int downside){
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	int cH = downside - upside + 1;
	IplImage* imageErode = cvCreateImage(cvSize(iWidth, cH), 8, 1);
	int widthStep = imageErode->widthStep;

	for(int i = 0; i<cH; i++){
		memcpy(imageErode->imageData + widthStep*i, greys + iWidth*(upside + i), iWidth);
	}

	IplImage* imageConnected = cvCloneImage(imageErode);

	// need modified: change parameter, looser condition allowed
	int anchor = (cH + 1)/7;
/*	cout << "cH: " << cH << " anchor: " << anchor << endl;*/

	anchor = (anchor < 2)? 2: anchor;

	//IplConvKernel* element = cvCreateStructuringElementEx( 3, 3, 1, 1, CV_SHAPE_RECT, 0 );
	//IplConvKernel* element = cvCreateStructuringElementEx( 5, 5, 2, 2, CV_SHAPE_RECT, 0 );
	//IplConvKernel* element = cvCreateStructuringElementEx( 5, 5, 2, 2, CV_SHAPE_ELLIPSE, 0 );
	//IplConvKernel* element = cvCreateStructuringElementEx( 7, 7, 3, 3, CV_SHAPE_ELLIPSE, 0 );

	IplConvKernel* element = cvCreateStructuringElementEx( anchor*2+1, anchor*2+1, anchor, anchor, CV_SHAPE_ELLIPSE, 0 );

	cvErode(imageErode, imageErode, element, 1);

	//DebugToolkit::displayImage(imageErode);

	char* dataErode = imageErode->imageData;
	char* dataConn = imageConnected->imageData;
	bool found = true;
	for(int i = 0; i<cH ; i++){
		for(int j = 0; j<iWidth; j++){
			if(*(dataErode + iWidth*i + j) == charColor){

				cvFloodFill(imageErode, cvPoint(j, i), cvScalarAll(backColor));
				if(*(dataConn + iWidth*i + j) == charColor){
					cvFloodFill(imageConnected, cvPoint(j, i), cvScalarAll(backColor));
				}

				found = true;
			}
		}
	}

#ifdef DISPLAY_CONNECTED_IMAGE
	DebugToolkit::displayImage(imageConnected);
#endif

	if(!found){
		return NULL;
	}

	char* retData = new char[cH * iWidth];
	for(int i = 0; i<cH; i++){
		memcpy(retData + iWidth*i, imageConnected->imageData + widthStep*i, iWidth);
	}

	cvReleaseImage(&imageConnected);
	cvReleaseImage(&imageErode);

	return retData;
}

CharDivider::Scale CharDivider::scale(int fist, int second, InputType type)
{
	float scale = fist * 1.0f / second;
	Scale res = PROPER;

	switch (type){
	case CHINESE:
		if(scale > 1.17){
			res = LEFT_TOOBIG;
		}else if(scale < 0.733){
			res = LEFT_TOOSMALL;
		}

		break;

	case ENGLISH:
		if(scale > 1){
			res = LEFT_TOOBIG;
		}else if(scale < 0.1){
			res = LEFT_TOOSMALL;
		}

		break;

// 	case STRICT:
// 		if(scale > 1.1){
// 			res = LEFT_TOOBIG;
// 		}else if(scale < 0.9){
// 			res = LEFT_TOOSMALL;
// 		}
// 
// 		break;

	case LOOSE:
		if(scale > 1.3){
			res = LEFT_TOOBIG;
		}else if(scale < 0.7){
			res = LEFT_TOOSMALL;
		}

		break;

	default:
		assert(false);
	}

	return res;
}

void CharDivider::findUpAndDown(const char* greys, int iWidth, int* X, int* Y, int* cWidth, int* cHeight)
{
	char charColor = OCRToolkit::s_CHARACTERCOLOR;

	int xoff = *X, yoff = *Y, i;
	bool isBreak = false;
	for(i = 0; i < *cHeight; i++){
		for(int j = 0; j < *cWidth; j++){
			if(*(greys + iWidth*(i + yoff) + j + xoff) == charColor){
				isBreak = true;

				break;
			}
		}

		if(isBreak){
			break;
		}
	}
	*Y += i;

	isBreak = false;
	for(i = *cHeight - 1; i>=0; i--){
		for(int j = 0; j < *cWidth; j++){
			if(*(greys + iWidth*(i + yoff) + j + xoff) == charColor){
				isBreak = true;

				break;
			}
		}

		if(isBreak){
			break;
		}
	}
	*cHeight = i - *Y + yoff + 1;
}

bool CharDivider::divideHelp(const int* histagram, int iWidth, int* cW, int* dW, vector<int>* offList, vector<int>* cWlist, vector<int>* dWlist)
{
	// need modified DEBUG
// 	cout << "vertical projection: ";
// 	for(int j = 0; j<iWidth; j++){
// 		cout << histagram[j] << " ";
// 	}
// 	cout << endl;

	int i = 0, temp;
	while(i<iWidth && histagram[i] == 0){ 
		i++; 
	}

	while(true){
		temp = i++;	
		while(i<iWidth && histagram[i] > 0){
			i++; 
		}
		if(i == iWidth){ 
			break; 
		}
		offList->push_back(temp);
		cWlist->push_back(i - temp);

		temp = i++;
		while(i<iWidth && histagram[i] == 0){ 
			i++; 
		}
		if(i == iWidth){
			break;
		}
		dWlist->push_back(i - temp);
	}

	if(offList->size() == 0){
		return false;
	}

	*cW = calcMaxFreqValue(cWlist);

	if(dWlist->size() > 0){
		*dW = calcMaxFreqValue(dWlist);	
	}

	return true;
}
int CharDivider::calcMaxFreqValue(vector<int>* v)
{
	assert(v->size() > 0);

	vector<int> temp(*v);
	sort(temp.begin(), temp.end());

	int value, freq, t, size;
	size = temp.size();
	t = 0;
	freq = 1;
	value = temp[0];
	for(int i = 1; i<size; i++){
		if(temp[i] != temp[i - 1]){
			if(i - t >= freq){
				value = temp[i - 1];
				freq = i - t;
				t = i;
			}
		}
	}
	if(size - t > freq){
		value = temp[size - 1];
	}

	return value;
}

bool CharDivider::isSign(const char* greys, int x, int y, int width, int height)
{
	return false;
}

int CharDivider::findTroughFromSide(const int* histgram, int len)
{
	int head = histgram[0], tail = histgram[len-1], temp, offset;

	offset = len;	// any number big enough
	for(int i = 1; i<(len - 1)/2; i++){
		temp = histgram[i];
		if(temp <= histgram[i-1] && temp <= histgram[i+1]){
			offset = i;
			break;
		}
	}

	for(int i = 1; i<(len - 1)/2; i++){
		temp = histgram[len - i - 1];
		if(temp <= histgram[len-i-2] && temp <= histgram[len-i]){
			if(i < offset){
				offset = len - i - 1;
			}
			break;
		}
	}

	return offset;
}

void CharDivider::copyArea(char* dest, const char* src, int iWidth, int x, int y, int cW, int cH)
{
	for(int i = 0; i<cH; i++){
		memcpy(dest + cW*i, src + iWidth*(y+i) + x, cW);
	}
}

#ifdef DEBUG
void CharDivider::DEBUG_markChar(char* greys, int iWidth, int iHeight, int x, int y, int w, int h){
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	for(int i = 0; i<w; i++){
		*(greys + iWidth*y + x + i) = charColor;
		*(greys + iWidth*(y+h-1) + x + i) = charColor;
	}
	for(int i = 0; i<h; i++){
		*(greys + iWidth*(y+i) + x) = charColor;
		*(greys + iWidth*(y+i) + x + w - 1) = charColor;
	}
}
#endif

void divide::CharDivider::filterNoise(char* greys, int iWidth, int upside, int downside, bool strict)
{
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	int tw = iWidth + 4, iHeight = downside-upside+1, th = iHeight + 4;
	char *pix = NULL, *buff = new char[tw*th];

	for(int i = 0; i<2; i++){
		memset(buff + tw*i, backColor, tw);
		memset(buff + tw*(th-i-1), backColor, tw);
	}
	for(int j = 0; j<2; j++){
		for(int i = 0; i<iHeight; i++){
			*(buff + tw*(i+2) + j) = backColor;
			*(buff + tw*(i+2) + tw-j-1) = backColor;
		}
	}

	for(int i = 0; i<iHeight; i++){
		memcpy(buff + tw*(i+2) + 2, greys + iWidth*(i+upside), iWidth);
	}

	int count, noiseCount = 0;
	for(int i = 0; i<iHeight; i++){
		for(int j = 0; j<iWidth; j++){
			count = 0;
			pix = buff + tw*(i+2) + j+2;

			if(*pix == charColor){
				// check four corners
				if(*(pix-1-tw) == charColor){
					count++;
				}
				if(*(pix+1-tw) == charColor){
					count++;
				}
				if(*(pix+1+tw) == charColor){
					count++;
				}
				if(*(pix-1+tw) == charColor){
					count++;
				}

				if(count > 2){
					continue;
				}

				if(count < 2){
					// * * *
					// * 1 *
					// * * *
					if(*(pix-tw) == charColor){
						count++;
					}
					if(*(pix-1) == charColor){
						count++;
					}
					if(*(pix+tw) == charColor){
						count++;
					}
					if(*(pix+1) == charColor){
						count++;
					}

					if(count == 0){
						*(greys + iWidth*(i+upside) + j) = *pix = backColor;
						noiseCount++;

						continue;
					}else if(count > 1 || !strict){
						continue;
					}else{

						// * * * *
						// * * 1 *
						// * * * *
						count = 0;

						for(int k = 0; k<4; k++){
							if(*(pix-2-tw+k) == charColor){
								count++;
							}
							if(*(pix-2+tw+k) == charColor){
								count++;
							}
						}
						if((*(pix-2) == charColor)){
							count++;
						}
						if(*(pix+1) == charColor){
							count++;
						}

						if(count > 1){
							continue;
						}

						// * * *
						// * 1 *
						// * * *
						// * * *
						count = 0;

						for(int k = 0; k<4; k++){
							if(*(pix-1+tw*(k-1)) == charColor){
								count++;
							}
							if(*(pix+1+tw*(k-1)) == charColor){
								count++;
							}
						}
						if((*(pix-tw) == charColor)){
							count++;
						}
						if(*(pix+tw*2) == charColor){
							count++;
						}

						if(count > 1){
							continue;
						}


						// * * * *
						// * 1 * *
						// * * * *
						count = 0;

						for(int k = 0; k<4; k++){
							if(*(pix-1-tw+k) == charColor){
								count++;
							}
							if(*(pix-1+tw+k) == charColor){
								count++;
							}
						}
						if((*(pix-1) == charColor)){
							count++;
						}
						if(*(pix+2) == charColor){
							count++;
						}

						if(count > 1){
							continue;
						}
					}
				}

				// * * * *
				// * 1 * *
				// * * * *
				// * * * *
				count = 0;

				for(int k = 0; k<4; k++){
					if(*(pix-1-tw+k) == charColor){
						count++;
					}
					if(*(pix-1+tw*2+k) == charColor){
						count++;
					}
				}

				if(count > 1){
					continue;
				}

				for(int k = 0; k<2; k++){
					if(*(pix-1+tw*k) == charColor){
						count++;
					}
					if(*(pix+2+tw*k) == charColor){
						count++;
					}
				}

				if(count > 1){
					continue;
				}

				// * * * *
				// * * 1 *
				// * * * *
				// * * * *
				count = 0;

				for(int k = 0; k<4; k++){
					if(*(pix-2-tw+k) == charColor){
						count++;
					}
					if(*(pix-2+tw*2+k) == charColor){
						count++;
					}
				}

				if(count > 1){
					continue;
				}

				for(int k = 0; k<2; k++){
					if(*(pix-2+tw*k) == charColor){
						count++;
					}
					if(*(pix+1+tw*k) == charColor){
						count++;
					}
				}

				if(count < 2){
					*(greys + iWidth*(i+upside) + j) = *pix = backColor;
					noiseCount++;

					if(*(pix+1) == charColor){
						*(greys + iWidth*(i+upside) + j+1) = *(pix+1) = backColor;
						noiseCount++;

						continue;
					}
					if(*(pix-1+tw) == charColor){
						*(greys + iWidth*(i+1+upside) + j-1) = *(pix-1+tw) = backColor;
						noiseCount++;

						continue;
					}
					if(*(pix+tw) == charColor){
						*(greys + iWidth*(i+1+upside) + j) = *(pix+tw) = backColor;
						noiseCount++;
					}
				}
			}
		}
	}
	
	//cout << "noise count: " << noiseCount << endl;

	delete[] buff;
}