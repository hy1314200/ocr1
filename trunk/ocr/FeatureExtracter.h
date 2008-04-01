#ifndef _FEATUREEXTRACTER_H
#define _FEATUREEXTRACTER_H

#include <stdio.h>
#include <cxcore.h>

namespace recognise{

	class FeatureExtracter
	{
	public:
		static const int s_NORMSIZE = 64;

		static const int s_FEATURESIZE = 403;

		static FeatureExtracter* getInstance();

		/** this method can update the max/min value of each dim in feature */
		void extractFeature(double* data, const char* imageData, bool updateMaxMin = false);

		void extractScaledFeature(double* data, const char* imageData);

		void scaleFeature(double* feature);

		void saveData();

	private:
		static const char* s_filepath;

		enum Direction{ NORTH = 0, SOUTH, WEST, EAST };

		enum Line{ LDOWN_RUP = 0, LUP_RDOWN, LEFT_RIGHT, UP_DOWN };

		enum BlackJumpType{ SUB_TOTAL = 0, SUB_DIVIDED };

		enum HV{ HORIZONTAL = 0, VERTICAL, SLANTING = VERTICAL };

		static const int s_STRIPESIZE = 8;

		static const int s_GRIDSIZE = 4;

		static const int s_SUBVCOUNT = 8;

		static FeatureExtracter* s_instance;

		double m_max[s_FEATURESIZE];
		double m_min[s_FEATURESIZE];

		FeatureExtracter(FILE *file);
		~FeatureExtracter(void){  };

		void calcStrokeWidthAndLen(const char* imageData, int* strokeWidth, int* totalStrokeLen);

		void calcProjHist(const char* imageData, int projHist[][s_NORMSIZE]);

		void calcTransDensAndPeri(const char* imageData, int* transitions, int strokeDensity[][s_STRIPESIZE], int peripheral[][s_STRIPESIZE][2]);
	
		void calcLocDirPropAndMaxLocDir(const char* imageData, int locDir[][s_GRIDSIZE][4], double strokeProp[][s_GRIDSIZE][4], int maxLocDirCtr[][s_GRIDSIZE][4]);
	
		void calcBlackJump(const char* imageData, int totalBlackJump[][s_SUBVCOUNT], double divBlackJump[][s_SUBVCOUNT]);

	};

}

#endif