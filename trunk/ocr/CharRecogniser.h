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
		static CharRecogniser* buildInstance();
		static void  release(){
			if(s_instance != 0){
				delete s_instance;
				s_instance = 0;
			}
		}

		/** @return Reliably, from 0% to 100% */
		virtual double recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res) = 0;

		virtual void buildFeatureLib(generate::FontLib** fontLib, int size) = 0;

		virtual bool isAvailable() = 0;

#ifdef TEST
		void TEST_distorteAndNorm(char** samples, char* prototype, int sampleSize){
			distorteAndNorm(samples, prototype, sampleSize);
		}
#endif

	protected:
		static CharRecogniser* s_instance;

		// CvMat* W; used for MDA

		CharRecogniser(/*const char* modelFilepath*/){  };
		virtual ~CharRecogniser(void){  };

		static void normalize(char* res, const char* greys, int iWidth, int x, int y, int width, int height);
		
		/** used to generate many distorted samples for training */
		static void distorteAndNorm(char** samples, const char* prototype, int sampleSize);

		static void findXYWH(char* data, int* x, int* y, int* width, int* height);

		static void reduceResolution(IplImage* image, int scale, int type = CV_INTER_NN, int threshold = 128);

	private:
		static const char* s_CONFPATH;

	};

	class SVMClassifier: public CharRecogniser
	{
	public:
		void buildFeatureLib(generate::FontLib** fontLib, int size);

		SVMClassifier();
		~SVMClassifier();

		double recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res);

		bool isAvailable(){
			return m_model != NULL;
		}

	private:
		static const char* s_MODELPATH;

		struct svm_model* m_model;

		void trainAndSaveClassifier(struct svm_problem *prob);

	};

	class MNNClassifier: public CharRecogniser
	{
	public:
		void buildFeatureLib(generate::FontLib** fontLib, int size);

		MNNClassifier();
		~MNNClassifier();

		double recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res);

		bool isAvailable(){
			return true;
		};

	private:
		static const char* s_MODELPATH;
	};

}

#endif
