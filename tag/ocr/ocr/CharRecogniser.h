#ifndef _CHARRECOGNISER_H
#define _CHARRECOGNISER_H

#include "stdafx.h"
#include "FontLib.h"

#include <cxcore.h>
#include <cv.h>

namespace recognise{

	class CharRecogniser
	{
	public:
		/** The entrance of getting an instance of the OCRToolkit */
		static CharRecogniser* buildInstance();
		static void  release(){
			if(s_instance != 0){
				delete s_instance;
				s_instance = 0;
			}
		}

		/** @return Reliably, from 0% to 100% */
		double recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res);

		void trainClassifier();

		virtual bool isAvailable() = 0;

#ifdef TEST
		void TEST_distorteAndNorm(char** samples, char* prototype){
			distorteAndNorm(samples, prototype);
		}
#endif

	protected:
		static const int s_sampleSize = 13;

		static CharRecogniser* s_instance;

		// CvMat* W; used for MDA

		CharRecogniser(/*const char* modelFilepath*/){  };
		virtual ~CharRecogniser(void){  };

		virtual void buildFeatureLib(library::FontLib** fontLib, int size) = 0;

		virtual double classify(const float *feature, wchar_t *res) = 0;

		static void normalize(char* res, const char* greys, int iWidth, int x, int y, int width, int height);
		
		/** used to generate many distorted samples for training */
		static void distorteAndNorm(char** samples, const char* prototype);

		static void findXYWH(char* data, int* x, int* y, int* width, int* height);

		static void reduceResolution(IplImage* image, int scale, int type = CV_INTER_NN, int threshold = 128);

	};

}

#endif
