#ifndef _CHARDIVIDER_H
#define _CHARDIVIDER_H

#include "stdafx.h"

#include <vector>
#include <cxcore.h>

using namespace std;

namespace divide{

	class CharDivider
	{
	public:
		/** return false is character image has more than one line characters or has no one, else true */
		static bool divideChar(char* greys, int iWidth, int iHeight, vector<char*>* picList, vector<int>* widthList, vector<int>* heightList);

		/** filter the noise in the picture. if strict, remove all likely noise */
		static void filterNoise(char* greys, int width, int upside, int downside, bool strict = true);

	private:
		static const float s_SCALETHRESHOLD;

		struct CRect
		{
			int x;
			int y;
			int width;
			int height;
			int rightSpace;
		}CRect;

		enum InputType{
			CHINESE = 10,
			ENGLISH,
			STRICT,
			LOOSE
		};

		enum Scale{
			LEFT_TOOBIG = 100,
			LEFT_TOOSMALL,
			PROPER
		};

		CharDivider(void){ };
		~CharDivider(void){ };

		/** 
		* Remove the background in advance
		* @return false is character image has more than one line characters or has no one, else true 
		*/
		static bool preProcess(const char* greys, int iWidth, int iHeight, int* upside, int* downside);

		static bool horizontalRejectCalc(const char* greys, int iWidth, int iHeight, int* upside, int* downside, int threshold);

		static char* removeBigConnectedComp(const char* greys, int iWidth, int upside, int downside);

		/** According to the CharacterType, return TOOWIDTH if too wide, TOOHIGH if too high, APPROPRIATE if appropriate */
		static Scale scale(int width, int height, InputType type);

		/** Find the upside and downside of a character */
		static void findUpAndDown(const char* greys, int iWidth, int* X, int* Y, int* cWidth, int* cHeight);

		/** 
		* According to the histgram, calculate the character width(cW) and the proper distance width(dW) 
		* @return false is character image has more than one line characters or has no one, else true
		*/
		static bool divideHelp(const int* histagram, int iWidth, int* cW, int* dW, vector<int>* offList, vector<int>* cWlist, vector<int>* dWlist);

		static int calcMaxFreqValue(vector<int>* v);

		/** Check if it's a sign */
		static bool isSign(const char* greys, int x, int y, int width, int height);

		static int findTroughFromSide(const int* histgram, int len);

		static void copyArea(char* dest, const char* src, int iWidth, int x, int y, int cW, int cH);

#ifdef DEBUG
		static void DEBUG_markChar(char* greys, int iWidth, int iHeight, int x, int y, int w, int h);
#endif

	};

}

#endif
