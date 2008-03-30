#ifndef _FEATUREEXTRACTER_H
#define _FEATUREEXTRACTER_H

#include <cxcore.h>

namespace recognise{

	class FeatureExtracter
	{
	public:
		static const int s_NORMSIZE = 64;

		static const int s_FEATURESIZE = 404;

		static void extractFeature(double* data, const char* imageData);

	private:
		static const int s_STRIPESIZE = 8;

		static const int s_GRIDSIZE = 4;

		static const int s_SUBVCOUNT = 8;

		enum Direction{ NORTH = 0, SOUTH, WEST, EAST };

		enum Line{ LDOWN_RUP = 0, LUP_RDOWN, LEFT_RIGHT, UP_DOWN };

		enum BlackJumpType{ SUB_TOTAL = 0, SUB_DIVIDED };

		enum HV{ HORIZONTAL = 0, VERTICAL, SLANTING = VERTICAL };

		FeatureExtracter(void){ };
		~FeatureExtracter(void){ };

		static void calcStrokeWidthAndLen(const char* imageData, int* strokeWidth, int* totalStrokeLen);

		static void calcProjHist(const char* imageData, int projHist[][s_NORMSIZE]);

		static void calcTransDensAndPeri(const char* imageData, int* transitions, int strokeDensity[][s_STRIPESIZE], int peripheral[][s_STRIPESIZE][2]);
	
		static void calcLocDirPropAndMaxLocDir(const char* imageData, int locDir[][s_GRIDSIZE][4], double strokeProp[][s_GRIDSIZE][4], int maxLocDirCtr[][s_GRIDSIZE][4]);
	
		static void calcBlackJump(const char* imageData, int totalBlackJump[][s_SUBVCOUNT], double divBlackJump[][s_SUBVCOUNT]);
	};

}

#endif