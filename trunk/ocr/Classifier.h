#ifndef _CLASSIFIER_H
#define _CLASSIFIER_H

#include "stdafx.h"
#include "FontGen.h"
#include "CharRecogniser.h"
#include "libsvm-2.85/svm.h"

#include <stdio.h>

namespace recognise
{

	class SVMClassifier: public CharRecogniser
	{
	public:
		void buildFeatureLib(generate::FontLib** fontLib, int size);

		SVMClassifier();
		~SVMClassifier();

		double classify(const float *scaledFeature, wchar_t *res);

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

		double classify(const float *scaledFeature, wchar_t *res);

		bool isAvailable(){
			return m_lib.size() > 0;
		};

	private:
		typedef struct prototype{
			wchar_t label;
			float *data;
		} Prototype;

		static const char* s_MODELPATH;

		vector<Prototype*> m_lib;

		static bool compare(const Prototype* x,const Prototype* y);

		void loadFile(FILE *file);
		void storeFile();
	};

}

#endif