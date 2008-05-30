#ifndef _CLASSIFIER_H
#define _CLASSIFIER_H

#include "FontLib.h"
#include "CharRecogniser.h"
#include "libsvm-2.85/svm.h"

namespace recognise
{

	class SVMClassifier: public CharRecogniser
	{
	public:
		void buildFeatureLib(library::FontLib** fontLib, int size);

		SVMClassifier();
		~SVMClassifier();

		double classify(const float *scaledFeature, wchar_t *res);

		bool isAvailable(){
			return m_model != NULL;
		}

	private:
		struct svm_model* m_model;

		void trainAndSaveClassifier(struct svm_problem *prob);

	};

	class KNNClassifier: public CharRecogniser
	{
	public:
		void buildFeatureLib(library::FontLib** fontLib, int size);

		KNNClassifier();
		~KNNClassifier();

		double classify(const float *scaledFeature, wchar_t *res);

		bool isAvailable(){
			return m_lib.size() > 0;
		};

	private:
		typedef struct prototype{
			wchar_t label;
			float *data;
		} Prototype;

		vector<Prototype*> m_lib;

		static bool compare(const Prototype* x,const Prototype* y);

		void loadFile(FILE *file);
		void storeFile();
	};

}

#endif