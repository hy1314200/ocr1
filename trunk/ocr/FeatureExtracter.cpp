#include "FeatureExtracter.h"
#include "OCRToolkit.h"
#include "DebugToolkit.h"

#include <iostream>

using namespace recognise;

FeatureExtracter* FeatureExtracter::s_instance = NULL;
const char* FeatureExtracter::s_filepath = "data/classifier/feature";

FeatureExtracter::FeatureExtracter(FILE *file){
	if(file == NULL){
		for(int i = 0; i<s_FEATURESIZE; i++){
			m_max[i] = 0;
			m_min[i] = 1000; // max number
		}
	}else{
		for(int i = 0; i<s_FEATURESIZE; i++){
			fscanf(file, "%lf %lf", m_max+i, m_min+i);
		}
	}
}

FeatureExtracter* FeatureExtracter::getInstance(){
	if(s_instance == 0){
		FILE* file = fopen(s_filepath, "r");

		if(file != NULL){
			s_instance = new FeatureExtracter(file);

			fclose(file);
		}else{
			s_instance = new FeatureExtracter(NULL);
		}
	}

	return s_instance;
}

void FeatureExtracter::extractFeature(double* data, const char* imageData, bool updateMaxMin)
{
	// the feature describes a character image 
	int strokeWidth;								// stroke width
	int totalStrokeLen;								// total stroke length
	int projHist[2][s_NORMSIZE];					// projection histogram in two directions
	int transitions[2];								// number of transitions in two directions
	int strokeDensity[2][s_STRIPESIZE];				// stroke density in two directions
	int peripheral[4][s_STRIPESIZE][2];				// two peripheral features with four directions
	int locDir[s_GRIDSIZE][s_GRIDSIZE][4];			// local direction contributivity with four regions and four directions
	double strokeProp[2][s_GRIDSIZE][4];			// stroke proportion in two directions  
	int maxLocDirCtr[s_GRIDSIZE][s_GRIDSIZE][4];	// maximum local direction contributivity
	int totalBlackJump[2][s_SUBVCOUNT];				// black jump distribution in each balanced subvectors  
	double divBlackJump[2][s_SUBVCOUNT];			// black jump distribution in each balanced subvectors divided by the total 

	calcStrokeWidthAndLen(imageData, &strokeWidth, &totalStrokeLen);
	calcProjHist(imageData, projHist);
	calcTransDensAndPeri(imageData, transitions, strokeDensity, peripheral);
	calcLocDirPropAndMaxLocDir(imageData, locDir, strokeProp, maxLocDirCtr);
	calcBlackJump(imageData, totalBlackJump, divBlackJump);

	int count = 0, tempInt;
	double tempDouble;

	// int strokeWidth;
	//data[count++] = strokeWidth;	// ignore this feature

	//int totalStrokeLen;
	if(updateMaxMin){
		if(m_max[count] < totalStrokeLen){
			m_max[count] = totalStrokeLen;
		}
		if(m_min[count] > totalStrokeLen){
			m_min[count] = totalStrokeLen;
		}
	}
	data[count++] = totalStrokeLen;

	// int projHist[2][s_NORMSIZE];
	for(int i = 0; i<2; i++){
		for(int j = 0; j<s_NORMSIZE; j++){
			tempInt = projHist[i][j];

			if(updateMaxMin){
				if(m_max[count] < tempInt){
					m_max[count] = tempInt;
				}
				if(m_min[count] > tempInt){
					m_min[count] = tempInt;
				}
			}
			data[count++] = tempInt;
		}
	}

	// int transitions[2];
	if(updateMaxMin){
		if(m_max[count] < transitions[0]){
			m_max[count] = transitions[0];
		}
		if(m_min[count] > transitions[0]){
			m_min[count] = transitions[0];
		}
	}
	data[count++] = transitions[0];

	if(updateMaxMin){
		if(m_max[count] < transitions[1]){
			m_max[count] = transitions[1];
		}
		if(m_min[count] > transitions[1]){
			m_min[count] = transitions[1];
		}
	}
	data[count++] = transitions[1];

	// int strokeDensity[2][s_STRIPESIZE];
	for(int i = 0; i<2; i++){
		for(int j = 0; j<s_STRIPESIZE; j++){
			tempInt = strokeDensity[i][j];

			if(updateMaxMin){
				if(m_max[count] < tempInt){
					m_max[count] = tempInt;
				}
				if(m_min[count] > tempInt){
					m_min[count] = tempInt;
				}
			}
			data[count++] = tempInt;
		}
	}

	// int peripheral[4][s_STRIPESIZE][2];
	for(int i = 0; i<4; i++){
		for(int j = 0; j<s_STRIPESIZE; j++){
			for(int k = 0; k<2; k++){
				tempInt = peripheral[i][j][k];

				if(updateMaxMin){
					if(m_max[count] < tempInt){
						m_max[count] = tempInt;
					}
					if(m_min[count] > tempInt){
						m_min[count] = tempInt;
					}
				}
				data[count++] = tempInt;
			}
		}
	}

	// int locDir[s_GRIDSIZE][s_GRIDSIZE][4];
	for(int i = 0; i<s_GRIDSIZE; i++){
		for(int j = 0; j<s_GRIDSIZE; j++){
			for(int k = 0; k<4; k++){
				tempInt = locDir[i][j][k];

				if(updateMaxMin){
					if(m_max[count] < tempInt){
						m_max[count] = tempInt;
					}
					if(m_min[count] > tempInt){
						m_min[count] = tempInt;
					}
				}
				data[count++] = tempInt;
			}
		}
	}

	// double strokeProp[2][s_GRIDSIZE][4];
	for(int i = 0; i<2; i++){
		for(int j = 0; j<s_GRIDSIZE; j++){
			for(int k = 0; k<4; k++){
				tempDouble = strokeProp[i][j][k];

				if(updateMaxMin){
					if(m_max[count] < tempDouble){
						m_max[count] = tempDouble;
					}
					if(m_min[count] > tempDouble){
						m_min[count] = tempDouble;
					}
				}
				data[count++] = tempDouble;
			}
		}
	}

	// int maxLocDirCtr[s_GRIDSIZE][s_GRIDSIZE][4]
	for(int i = 0; i<s_GRIDSIZE; i++){
		for(int j = 0; j<s_GRIDSIZE; j++){
			for(int k = 0; k<4; k++){
				tempInt = maxLocDirCtr[i][j][k];

				if(updateMaxMin){
					if(m_max[count] < tempInt){
						m_max[count] = tempInt;
					}
					if(m_min[count] > tempInt){
						m_min[count] = tempInt;
					}
				}
				data[count++] =tempInt;
			}
		}
	}

	// int totalBlackJump[2][s_SUBVCOUNT];
	for(int i = 0; i<2; i++){
		for(int j = 0; j<s_SUBVCOUNT; j++){
			tempInt = totalBlackJump[i][j];

			if(updateMaxMin){
				if(m_max[count] < tempInt){
					m_max[count] = tempInt;
				}
				if(m_min[count] > tempInt){
					m_min[count] = tempInt;
				}
			}
			data[count++] = tempInt;
		}
	}

	// double divBlackJump[2][s_SUBVCOUNT];
	for(int i = 0; i<2; i++){
		for(int j = 0; j<s_SUBVCOUNT; j++){
			tempDouble = divBlackJump[i][j];

			if(updateMaxMin){
				if(m_max[count] < tempDouble){
					m_max[count] = tempDouble;
				}
				if(m_min[count] > tempDouble){
					m_min[count] = tempDouble;
				}
			}
			data[count++] = tempDouble;
		}
	}

	assert(count == s_FEATURESIZE);
}

void FeatureExtracter::extractScaledFeature(double* data, const char* imageData){
	extractFeature(data, imageData);
	scaleFeature(data);
}

void FeatureExtracter::scaleFeature(double* feature){
	for(int i = 0; i<s_FEATURESIZE; i++){
		feature[i] = (feature[i] - m_min[i]) / (m_max[i] - m_min[i]);
	}
}

void FeatureExtracter::calcStrokeWidthAndLen(const char* imageData, int* strokeWidth, int* totalStrokeLen)
{
	int charPixCount, hit;
	int charColor = OCRToolkit::s_CHARACTERCOLOR;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	charPixCount = hit = 0;

	for(int i = 0; i<s_NORMSIZE - 1; i++){
		for(int j = 0; j<s_NORMSIZE - 1; j++){
			if(*(imageData + s_NORMSIZE*i + j) == charColor){
				charPixCount++;

				if(*(imageData + s_NORMSIZE*(i+1) + j) == charColor){
					if(j>0 && *(imageData + s_NORMSIZE*(i+1) + j-1) == charColor){
						if(*(imageData + s_NORMSIZE*i + j+1) == charColor){
							hit++;
						}
					}else if(*(imageData + s_NORMSIZE*i + j+1) == charColor && *(imageData + s_NORMSIZE*(i+1) + j+1) == charColor){
						hit++;
					}						
				}else if(j<s_NORMSIZE-2 && *(imageData + s_NORMSIZE*i + j+1) == charColor && *(imageData + s_NORMSIZE*(i+1) + j+1) == charColor && *(imageData + s_NORMSIZE*(i+1) + j+2) == charColor){
					hit++;
				}
			}else if(*(imageData + s_NORMSIZE*i + j) != backColor){ // need modified
				std::cout << "FeatureExtracter::calcStrokeWidthAndLen:\n\t*(imageData + size*i + j) == " << (int)*(imageData + s_NORMSIZE*i + j) << std::endl; 
			}
		}
	}

	for(int i = 0; i<s_NORMSIZE-1; i++){
		if(*(imageData + s_NORMSIZE*i + s_NORMSIZE-1) == charColor){
			charPixCount++;
		}

		if(*(imageData + s_NORMSIZE*(s_NORMSIZE-1) + i) == charColor){
			charPixCount++;
		}
	}

	if(*(imageData + s_NORMSIZE*s_NORMSIZE - 1) == charColor){	// *(imageData + size*(size - 1) + (size-1))
		charPixCount++;
	}

	*strokeWidth = (hit == 0)? 1: cvRound(charPixCount*1.0f/(charPixCount - hit));
	*totalStrokeLen = cvRound(charPixCount*1.0f / *strokeWidth);
}

void FeatureExtracter::calcProjHist(const char* imageData, int projHist[][s_NORMSIZE])
{
	int charColor = OCRToolkit::s_CHARACTERCOLOR;

	memset(projHist, 0, 2*s_NORMSIZE*sizeof(int));

	for(int i = 0; i<s_NORMSIZE; i++){
		for(int j = 0; j<s_NORMSIZE; j++){
			if(*(imageData + s_NORMSIZE*i + j) == charColor){
				projHist[HORIZONTAL][j]++;
				projHist[VERTICAL][i]++;
			}
		}
	}
}

void FeatureExtracter::calcTransDensAndPeri(const char* imageData, int* transitions, int strokeDensity[][s_STRIPESIZE], int peripheral[][s_STRIPESIZE][2])
{
	int charColor = OCRToolkit::s_CHARACTERCOLOR;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;
	bool isFirst[4][s_NORMSIZE];

	enum Order{ FIRST = 0, SECOND };

	transitions[HORIZONTAL] = transitions[VERTICAL] = 0;

	memset(strokeDensity, 0, 2*s_STRIPESIZE*sizeof(int));

	for(int i = 0; i<s_STRIPESIZE; i++){
		peripheral[EAST][i][FIRST] = peripheral[SOUTH][i][FIRST] = peripheral[WEST][i][FIRST] = peripheral[NORTH][i][FIRST] = s_NORMSIZE;
		peripheral[EAST][i][SECOND] = peripheral[SOUTH][i][SECOND] = peripheral[WEST][i][SECOND] = peripheral[NORTH][i][SECOND] = s_NORMSIZE;
	}

	for(int i = 0; i<s_NORMSIZE; i++){
		isFirst[EAST][i] = isFirst[SOUTH][i] = isFirst[WEST][i] = isFirst[NORTH][i] = true;
	}

	int multiple = s_NORMSIZE/s_STRIPESIZE;
	for(int i = 0; i<s_NORMSIZE; i++){
		for(int j = 0; j<s_NORMSIZE; j++){
			if(*(imageData + s_NORMSIZE*i + j) == charColor){
				if(j > 0 && *(imageData + s_NORMSIZE*i + j-1) == backColor || j == 0){
					transitions[HORIZONTAL]++;

					strokeDensity[HORIZONTAL][i/multiple]++;

					if(isFirst[WEST][i]){
						isFirst[WEST][i] = false;
						if(j < peripheral[WEST][i/multiple][FIRST]){
							peripheral[WEST][i/multiple][FIRST] = j;
						}
					}else{ // need modified: 考虑第三次遇到transition的情况
						if(j < peripheral[WEST][i/multiple][SECOND]){
							peripheral[WEST][i/multiple][SECOND] = j;
						}
					}
				}

				if(i > 0 && *(imageData + s_NORMSIZE*(i-1) + j) == backColor || i == 0){
					transitions[VERTICAL]++;

					strokeDensity[VERTICAL][j/multiple]++;

					if(isFirst[NORTH][j]){
						isFirst[NORTH][j] = false;
						if(i < peripheral[NORTH][j/multiple][FIRST]){
							peripheral[NORTH][j/multiple][FIRST] = i;
						}
					}else{
						if(i < peripheral[NORTH][j/multiple][SECOND]){
							peripheral[NORTH][j/multiple][SECOND] = i;
						}
					}
				}
			}
		}
	}

	for(int i = s_NORMSIZE - 1; i >= 0; i--){
		for(int j = s_NORMSIZE - 1; j >= 0; j--){
			if(*(imageData + s_NORMSIZE*i + j) == charColor){
				if(j < s_NORMSIZE - 1 && *(imageData + s_NORMSIZE*i + j+1) == backColor || j == s_NORMSIZE - 1){
					if(isFirst[EAST][i]){
						isFirst[EAST][i] = false;

						if(s_NORMSIZE-1-j < peripheral[EAST][i/multiple][FIRST]){
							peripheral[EAST][i/multiple][FIRST] = s_NORMSIZE-1-j;
						}
					}else{
						if(s_NORMSIZE-1-j < peripheral[EAST][i/multiple][SECOND]){
							peripheral[EAST][i/multiple][SECOND] = s_NORMSIZE-1-j;
						}
					}
				}

				if(i < s_NORMSIZE - 1 && *(imageData + s_NORMSIZE*(i+1) + j) == backColor || i == s_NORMSIZE - 1){
					if(isFirst[SOUTH][j]){
						isFirst[SOUTH][j] = false;

						if(s_NORMSIZE-1-i < peripheral[SOUTH][j/multiple][FIRST]){
							peripheral[SOUTH][j/multiple][FIRST] = s_NORMSIZE-1-i;
						}
					}else{
						if(s_NORMSIZE-1-i < peripheral[SOUTH][j/multiple][SECOND]){
							peripheral[SOUTH][j/multiple][SECOND] = s_NORMSIZE-1-i;
						}
					}
				}
			}
		}
	}
}

void FeatureExtracter::calcLocDirPropAndMaxLocDir(const char* imageData, int locDir[][s_GRIDSIZE][4], double strokeProp[][s_GRIDSIZE][4], int maxLocDirCtr[][s_GRIDSIZE][4])
{
	int charColor = OCRToolkit::s_CHARACTERCOLOR;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;
	int count = 0, record[s_NORMSIZE][s_NORMSIZE][4];

	memset(record, 0, 4*s_NORMSIZE*s_NORMSIZE*sizeof(int));

	for(int i = 0; i<s_NORMSIZE; i++){
		for(int j = 0; j<s_NORMSIZE; j++){
			if(*(imageData + s_NORMSIZE*i + j) == charColor){
				if(record[i][j][LEFT_RIGHT] == 0){
					count = 1;

					while(j+count < s_NORMSIZE && *(imageData + s_NORMSIZE*i + j+count) == charColor){
						count++;
					}

					for(int k = 0; k < count; k++){
						record[i][j+k][LEFT_RIGHT] = count;
					}
				}

				if(record[i][j][UP_DOWN] == 0){
					count = 1;

					while(i+count < s_NORMSIZE && *(imageData + s_NORMSIZE*(i+count) + j) == charColor){
						count++;
					}

					for(int k = 0; k < count; k++){
						record[i+k][j][UP_DOWN] = count;
					}
				}

				if(record[i][j][LUP_RDOWN] == 0){
					count = 1;

					while(j+count < s_NORMSIZE  && i+count < s_NORMSIZE && *(imageData + s_NORMSIZE*(i+count) + j+count) == charColor){
						count++;
					}

					for(int k = 0; k < count; k++){
						record[i+k][j+k][LUP_RDOWN] = count;
					}
				}

				if(record[i][j][LDOWN_RUP] == 0){
					count = 1;

					while(i+count < s_NORMSIZE && j >= count && *(imageData + s_NORMSIZE*(i+count) + j-count) == charColor){
						count++;
					}

					for(int k = 0; k < count; k++){
						record[i+k][j-k][LDOWN_RUP] = count;
					}
				}
			}
		}
	}

	int total[s_GRIDSIZE][s_GRIDSIZE][4], pixs[s_GRIDSIZE][s_GRIDSIZE];
	int multiple = s_NORMSIZE/s_GRIDSIZE;

	memset(total, 0, 4*s_GRIDSIZE*s_GRIDSIZE*sizeof(int));
	memset(maxLocDirCtr, 0, 4*s_GRIDSIZE*s_GRIDSIZE*sizeof(int));
	memset(pixs, 0, s_GRIDSIZE*s_GRIDSIZE*sizeof(int));

	bool tag;
	for(int i = 0; i<s_NORMSIZE; i++){
		for(int j = 0; j<s_NORMSIZE; j++){
			tag = false;

			if(record[i][j][UP_DOWN] > 0){
				tag = true;

				total[i/multiple][j/multiple][UP_DOWN] += record[i][j][UP_DOWN];
			}

			if(record[i][j][LEFT_RIGHT] > 0){
				tag = true;

				total[i/multiple][j/multiple][LEFT_RIGHT] += record[i][j][LEFT_RIGHT];
			}

			if(record[i][j][LUP_RDOWN] > 0){
				tag = true;

				total[i/multiple][j/multiple][LUP_RDOWN] += record[i][j][LUP_RDOWN];
			}

			if(record[i][j][LDOWN_RUP] > 0){
				tag = true;

				total[i/multiple][j/multiple][LDOWN_RUP] += record[i][j][LDOWN_RUP];
			}

			if(tag){
				pixs[i/multiple][j/multiple]++;

				int m_max = 0, offset = 0;
				for(int k = 0; k<4; k++){
					if(m_max < record[i][j][k]){
						m_max = record[i][j][k];

						offset = k;
					}
				}
				maxLocDirCtr[i/multiple][j/multiple][offset]++;
			}
		}
	}

	int temp;
	int *p1= NULL, *p2 = NULL;
	for(int i = 0; i<s_GRIDSIZE; i++){
		for(int j = 0; j<s_GRIDSIZE; j++){
			temp = pixs[i][j];

			p1 = &(locDir[i][j][0]);
			p2 = &(total[i][j][0]);
			p1[LEFT_RIGHT] = (temp == 0)? 0: p2[LEFT_RIGHT] / temp;
			p1[UP_DOWN] = (temp == 0)? 0: p2[UP_DOWN] / temp;
			p1[LUP_RDOWN] = (temp == 0)? 0: p2[LUP_RDOWN] / temp;
			p1[LDOWN_RUP] = (temp == 0)? 0: p2[LDOWN_RUP] / temp;

			temp = maxLocDirCtr[i][j][LEFT_RIGHT];
			strokeProp[HORIZONTAL][i][LEFT_RIGHT] += temp;
			strokeProp[VERTICAL][j][LEFT_RIGHT] += temp;

			temp = maxLocDirCtr[i][j][UP_DOWN];
			strokeProp[HORIZONTAL][i][UP_DOWN] += temp;
			strokeProp[VERTICAL][j][UP_DOWN] += temp;

			temp = maxLocDirCtr[i][j][LUP_RDOWN];
			strokeProp[HORIZONTAL][i][LUP_RDOWN] += temp;
			strokeProp[VERTICAL][j][LUP_RDOWN] += temp;

			temp = maxLocDirCtr[i][j][LDOWN_RUP];
			strokeProp[HORIZONTAL][i][LDOWN_RUP] += temp;
			strokeProp[VERTICAL][j][LDOWN_RUP] += temp;
		}
	}

	double *fp = NULL;
	for(int i = 0; i<s_GRIDSIZE; i++){
		temp = pixs[i][0] + pixs[i][1] + pixs[i][2] + pixs[i][3];
		fp = &(strokeProp[HORIZONTAL][i][0]);
		fp[LEFT_RIGHT] = (temp == 0)? 0: fp[LEFT_RIGHT]/temp;
		fp[UP_DOWN] = (temp == 0)? 0: fp[UP_DOWN]/temp;
		fp[LUP_RDOWN] = (temp == 0)? 0: fp[LUP_RDOWN]/temp;
		fp[LDOWN_RUP] = (temp == 0)? 0: fp[LDOWN_RUP]/temp;

		temp = pixs[0][i] + pixs[1][i] + pixs[2][i] + pixs[3][i];
		fp = &(strokeProp[VERTICAL][i][0]);
		fp[LEFT_RIGHT] = (temp == 0)? 0: fp[LEFT_RIGHT]/temp;
		fp[UP_DOWN] = (temp == 0)? 0: fp[UP_DOWN]/temp;
		fp[LUP_RDOWN] = (temp == 0)? 0: fp[LUP_RDOWN]/temp;
		fp[LDOWN_RUP] = (temp == 0)? 0: fp[LDOWN_RUP]/temp;
	}
}

void FeatureExtracter::calcBlackJump(const char* imageData, int totalBlackJump[][s_SUBVCOUNT], double divBlackJump[][s_SUBVCOUNT]){
	char slantingData[s_NORMSIZE*s_NORMSIZE];
	int sum, temp;

	for(int i = 0; i<s_NORMSIZE; i++){
		for(int j = 0; j<s_NORMSIZE; j++){
			temp = i + j;
			if(temp < s_NORMSIZE){
				sum = temp*(temp + 1)/2;
				slantingData[sum + j] = *(imageData + s_NORMSIZE*i +j);
			}else{
				temp = 2*s_NORMSIZE - 2 - temp;

				sum = temp*(temp + 1)/2;
				slantingData[s_NORMSIZE*s_NORMSIZE - sum - s_NORMSIZE + j] = *(imageData + s_NORMSIZE*i +j); // s_NORMSIZE*s_NORMSIZE - 1 - sum - (s_NORMSIZE - 1 - j)
			}
		}
	}

	int charColor = OCRToolkit::s_CHARACTERCOLOR;
	int backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	memset(totalBlackJump, 0, 2*s_SUBVCOUNT*sizeof(int));

	int totalH = 0, totalS = 0, interval = s_NORMSIZE*s_NORMSIZE/s_SUBVCOUNT;
	for(int i = 0; i<s_SUBVCOUNT; i++){
		for(int j = 0; j<interval; j++){
			if(*(imageData + interval*i + j) == charColor && (i == 0 && j == 0 || *(imageData + interval*i + j-1) == backColor)){
				totalBlackJump[HORIZONTAL][i]++;

				totalH++;
			}

			if(*(slantingData + interval*i + j) == charColor && (i == 0 && j == 0 || *(slantingData + interval*i + j-1) == backColor)){
				totalBlackJump[SLANTING][i]++;

				totalS++;
			}
		}
	}

	for(int i = 0; i<s_SUBVCOUNT; i++){
		divBlackJump[HORIZONTAL][i] = totalBlackJump[HORIZONTAL][i]*1.0 / totalH;

		divBlackJump[SLANTING][i] = totalBlackJump[SLANTING][i]*1.0 / totalS;
	}
}

void FeatureExtracter::saveData(){
	FILE* file = fopen(s_filepath, "w");

	for(int i = 0; i < s_FEATURESIZE; i++){
		fprintf(file, "%lf %lf\n", m_max[i], m_min[i]);
	}

	fclose(file);
}