#include "Classifier.h"
#include "stdafx.h"
#include "FeatureExtracter.h"
#include "GlobalCofig.h"
#include "ConfigFile.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace recognise;
using namespace library;

SVMClassifier::SVMClassifier()
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	FILE *file = fopen(config->get("path.file.model.svm").c_str(), "r");

	if(file == NULL){
		m_model = NULL;
	}else{
		cout << "loading SVM model..." << endl;
		m_model = svm_load_model(config->get("path.file.model.svm").c_str());
		cout << "100% finished" << endl;

		fclose(file);
	}
}

SVMClassifier::~SVMClassifier(void)
{
	if(m_model != NULL){
		svm_destroy_model(m_model);
		m_model = NULL;
	}
}

void SVMClassifier::trainAndSaveClassifier(struct svm_problem *prob){
	if(prob == NULL){
		cout << "m_problem == NULL" << endl;

		exit(-1);
	}

	struct svm_parameter *param = new struct svm_parameter;


	// default values
	param->svm_type = C_SVC;
	param->kernel_type = RBF;
	param->degree = 3;
	param->gamma = 0;	// 1/k
	param->coef0 = 0;
	param->nu = 0.5;
	param->cache_size = 100;
	param->C = 1;
	param->eps = 1e-3;
	param->p = 0.1;
	param->shrinking = 1;
	param->probability = 0;
	param->nr_weight = 0;
	param->weight_label = NULL;
	param->weight = NULL;

	const char *error = svm_check_parameter(prob, param);
	if(error != NULL){
		cout << "ERROR: " << error << endl;

		exit(-1);
	}

	struct svm_model *model = svm_train(prob, param);

	svm_save_model(GlobalCofig::getConfigFile()->get("path.file.model.svm").c_str(), model);

	svm_destroy_model(model);

	delete param;
}

void SVMClassifier::buildFeatureLib(library::FontLib** fontLib, const int libSize)
{
	const int charCount = fontLib[0]->size();

#ifdef DEBUG
	for(int i = 1; i<libSize; i++){
		assert(charCount == fontLib[i]->size());
	}
#endif

	const int normSize = FeatureExtracter::s_NORMSIZE;
	const int featureSize = FeatureExtracter::s_FEATURESIZE;

	char** imageData = new char*[s_sampleSize];
	for(int i = 0; i<s_sampleSize; i++){
		imageData[i] = new char[normSize*normSize];
	}

	float *featureData = new float[charCount*libSize*s_sampleSize*featureSize], *tempData = featureData;
	FeatureExtracter* extracter = FeatureExtracter::getInstance();

	cout << "sample generating process:" << endl;
	for(int i = 0; i<charCount; i++){
		for(int j = 0; j<libSize; j++){
			distorteAndNorm(imageData, fontLib[j]->charArray()->at(i)->imageData(), fontLib[j]->typeface());

			for(int k = 0; k<s_sampleSize; k++, tempData += featureSize){
				extracter->extractFeature(tempData, imageData[k], true);
			}
		}

		if(i%100 == 0){
			cout << setprecision(2) << i*100*1.0/charCount << "% finished" << endl;
		}
	}
	cout << "100% finished" << endl;

	extracter->saveData();

	int count = charCount*libSize*s_sampleSize;
	struct svm_problem *problem = new struct svm_problem;
	problem->l = count;
	problem->y = new double[count];
	problem->x = new struct svm_node*[count];

	cout << "\nspace allocation process:" << endl;
	for(int i = 0; i<count; i++){
		problem->x[i] = new struct svm_node[featureSize + 1];

		if(i%4000 == 0){
			cout << setprecision(2) << i*100*1.0/count << "% finished" << endl;
		}
	}
	cout << "100% finished" << endl;

	cout << "\nproblem building process:" << endl;
	tempData = featureData;
	for(int i = 0; i<count; i++, tempData += featureSize){
		extracter->scaleFeature(tempData);

		problem->y[i] = fontLib[0]->charArray()->at(i/(libSize*s_sampleSize))->value();
		for(int j = 0; j<featureSize; j++){
			problem->x[i][j].index = j;
			problem->x[i][j].value= tempData[j]; 
		}

		problem->x[i][featureSize].index = -1;

		if(i%4000 == 0){
			cout << setprecision(2) << i*100*1.0/count << "% finished" << endl;
		}
	}
	cout << "100% finished\n" << endl;

#ifdef SAVE_PROBLEM

	FILE* file = fopen("data/classify/svm/problem", "w");
	assert(file != NULL);

	cout << "\nsaving problem process:" << endl;
	for(int i = 0; i<problem->l; i++){
		fprintf(file, "%d", (int)problem->y[i]);

		for(int j = 0; problem->x[i][j].index != -1; j++){
			fprintf(file, " %d:%lf", problem->x[i][j].index, problem->x[i][j].value);
		}

		fprintf(file, "\n");

		if(i%4000 == 0){
			cout << setprecision(2) << i*100*1.0/problem->l << "% finished" << endl;
		}
	}
	cout << "100% finished\n" << endl;

	fclose(file);

#endif

	trainAndSaveClassifier(problem);
	cout << "\nfinish svm training" << endl;

	for(int i = 0; i<count; i++){
		delete[] problem->x[i];
	}
	delete[] problem->x;
	delete[] problem->y;
	delete problem;

	for(int i = 0; i<s_sampleSize; i++){
		delete[] imageData[i];
	}
	delete[] imageData;

	delete[] featureData;
}

double SVMClassifier::classify(const float *scaledFeature, wchar_t *res)
{
	int featureSize = FeatureExtracter::s_FEATURESIZE;

	struct svm_node *node = new struct svm_node[featureSize+1];
	for(int i = 0; i<featureSize; i++){
		node[i].index = i;
		node[i].value = scaledFeature[i];
	}
	node[featureSize].index = -1;

	int size = svm_get_nr_class(m_model);
	double *probability = new double[size];

	double maxProb;
	if(svm_check_probability_model(m_model)){
		*res = (wchar_t)svm_predict_probability(m_model, node, probability);

		maxProb = 0;
		for(int i = 0; i<size; i++){
			if(maxProb < probability[i]){
				maxProb = probability[i];
			}
		}
	}else{
		*res = (wchar_t)svm_predict(m_model, node);

		maxProb = 1;
	}
	delete[] node;
	delete[] probability;

	return maxProb;
}

KNNClassifier::KNNClassifier()
{
	FILE *file = fopen(GlobalCofig::getConfigFile()->get("path.file.model.mnn").c_str(), "r");

	if(file != NULL){
		loadFile(file);

		fclose(file);		
	}
}

KNNClassifier::~KNNClassifier(void)
{
	int size = m_lib.size();
	for(int i = 0; i<size; i++){
		delete[] m_lib.at(i)->data;

		delete m_lib.at(i);
	}
}

void KNNClassifier::loadFile(FILE *file)
{
	int dim = FeatureExtracter::s_FEATURESIZE;
	Prototype* proto = NULL;
	
	int size;
	fscanf(file, "%d", &size);

	cout << "KNN model loading process:" << endl;
	for(int i = 0; i<size; i++){
		proto = new Prototype;
		proto->data = new float[dim];

		fscanf(file, "%d", &(proto->label));

		for(int j = 0; j<dim; j++){
			fscanf(file, "%f", proto->data + j);
		}

		m_lib.push_back(proto);

		if(i%10000 == 0){
			cout << setprecision(2) << i*100*1.0/size << "% finished" << endl;
		}
	}
	cout << "100% finished" << endl;
}

void KNNClassifier::storeFile()
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	FILE *file = fopen(config->get("path.file.model.mnn").c_str(), "w");
	assert(file);

	int dim = FeatureExtracter::s_FEATURESIZE;
	int size = m_lib.size();
	fprintf(file, "%d\n", size);

	for(int i = 0; i<size; i++){
		fprintf(file, "%d", (int)m_lib.at(i)->label);

		for(int j = 0; j<dim; j++){
			fprintf(file, " %f", m_lib.at(i)->data[j]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

void KNNClassifier::buildFeatureLib(library::FontLib** fontLib, const int libSize)
{
	const int charCount = fontLib[0]->size();

#ifdef DEBUG
	for(int i = 1; i<libSize; i++){
		assert(charCount == fontLib[i]->size());
	}
#endif

	const int normSize = FeatureExtracter::s_NORMSIZE;
	const int featureSize = FeatureExtracter::s_FEATURESIZE;

	char** imageData = new char*[s_sampleSize];
	for(int i = 0; i<s_sampleSize; i++){
		imageData[i] = new char[normSize*normSize];
	}

	FeatureExtracter* extracter = FeatureExtracter::getInstance();

	Prototype *proto = NULL;
	cout << "sample generating process:" << endl;
	for(int i = 0; i<charCount; i++){
		for(int j = 0; j<libSize; j++){
			distorteAndNorm(imageData, fontLib[j]->charArray()->at(i)->imageData(), fontLib[j]->typeface());

			for(int k = 0; k<s_sampleSize; k++){
				proto = new Prototype;
				proto->label = fontLib[j]->charArray()->at(i)->value();
				proto->data = new float[featureSize];

				extracter->extractFeature(proto->data, imageData[k], true);
				m_lib.push_back(proto);
			}
		}

		if(i%100 == 0){
			cout << setprecision(2) << i*100*1.0/charCount << "% finished" << endl;
		}
	}
	cout << "100% finished" << endl;

	extracter->saveData();

	sort(m_lib.begin(), m_lib.end(), compare);

	int size = m_lib.size();
	for(int i = 0; i<size; i++){
		extracter->scaleFeature(m_lib.at(i)->data);
	}

	storeFile();
	cout << "mnn model saved" << endl;

	for(int i = 0; i<s_sampleSize; i++){
		delete[] imageData[i];
	}
	delete[] imageData;
}

double KNNClassifier::classify(const float *scaledFeature, wchar_t *res)
{
	int size = m_lib.size();
	int dim = FeatureExtracter::s_FEATURESIZE;

	int index = 0;
//  	while(m_lib.at(index)->data[0] < scaledFeature[0]){
//  		++index;	
//  	}

	const int K = 4;

	float distance2;
	float nn[K], tempNN;
	for(int i = 0; i<K; i++){
		nn[i] = 999999;	// max float
	}
	int recordOff[K], tempROff;

	float *data = NULL;
	for( ; index<size; index++){
		distance2 = 0;

		data = m_lib.at(index)->data;
		for(int i = 0; i<dim; i++){
			distance2 += (data[i] - scaledFeature[i])*(data[i] - scaledFeature[i]);

			if(distance2 > nn[K-1]){
				break;
			}
		}

		if(distance2 < nn[K-1]){
			nn[K-1] = distance2;
			recordOff[K-1] = index;

			for(int i = 1; i<K; i++){
				if(nn[K-i] < nn[K-i-1]){
					tempNN = nn[K-i];
					tempROff = recordOff[K-i];
					nn[K-i] = nn[K-i-1];
					recordOff[K-i] = recordOff[K-i-1];
					nn[K-i-1] = tempNN;
					recordOff[K-i-1] = tempROff;
				}else{
					break;
				}
			}
		}
	}

	tempROff = recordOff[0];	// record the most nearest

#ifdef DISPLAY_KNN_CHAR

	setlocale(LC_ALL, "");
	for(int i = 0; i<K; i++){
		cout << m_lib.at(recordOff[i])->label;
		wprintf(L"%c ", m_lib.at(recordOff[i])->label);
	}
	cout << endl;

#endif

	// bubble sort
	for(int i = 0; i<K-1; i++){
		for(int j = 0; j<K-i-1; j++){
			if(m_lib.at(recordOff[j])->label > m_lib.at(recordOff[j+1])->label){
				tempROff = recordOff[j];
				recordOff[j] = recordOff[j+1];
				recordOff[j+1] = tempROff;
			}
		}
	}

	int offset, recodeCount = 0, count = 1;
	for(int i = 1; i<K; i++){
		if(m_lib.at(recordOff[i])->label == m_lib.at(recordOff[i-1])->label){
			++count;
		}else{
			if(count > recodeCount || count == recodeCount && m_lib.at(recordOff[i-1])->label == m_lib.at(tempROff)->label){
				recodeCount = count;
				offset = recordOff[i-1];
			}

			count = 1;
		}
	}
	if(count > recodeCount || count == recodeCount && m_lib.at(recordOff[K-1])->label == m_lib.at(tempROff)->label){
		recodeCount = count;
		offset = recordOff[K-1];
	}

	*res = m_lib.at(offset)->label;

	return 1;
}

bool KNNClassifier::compare(const Prototype* x,const Prototype* y)
{
	return x->data[0] < y->data[0];
}