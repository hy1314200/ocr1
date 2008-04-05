#ifndef _CHARRECOGNISER_H
#define _CHARRECOGNISER_H

#include "FontGen.h"
#include "stdafx.h"
#include "libsvm-2.85/svm.h"

#include <stdio.h>
#include <cv.h>

namespace recognise{

	class CharRecogniser
	{
	public:
		/** The entrance of getting an instance of the OCRToolkit */
		static CharRecogniser* getInstance();
		static void  release(){
			if(s_instance != 0){
				delete s_instance;
				s_instance = 0;
			}
		}

		/** @return Reliably, from 0% to 100% */
		double recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res);

		static void buildFeatureLib(generate::FontLib** fontLib, int size);

#ifdef TEST
		void TEST_distorteAndNorm(char** samples, char* prototype, int sampleSize){
			distorteAndNorm(samples, prototype, sampleSize);
		}
#endif

	private:
		static CharRecogniser* s_instance;

		static const char* s_FILEPATH;

		// CvMat* W; used for MDA

		struct svm_model* m_model;

		CharRecogniser(const char* modelFilepath);
		~CharRecogniser(void);

		static void normalize(char* res, const char* greys, int iWidth, int x, int y, int width, int height);
		
		/** used to generate many distorted samples for training */
		static void distorteAndNorm(char** samples, const char* prototype, int sampleSize);

		static void findXYWH(char* data, int* x, int* y, int* width, int* height);

		static void reduceResolution(IplImage* image, int scale, int type = CV_INTER_NN, int threshold = 128);

		static void trainAndSaveClassifier(struct svm_problem *prob);

	};

}

#endif
