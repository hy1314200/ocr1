#ifndef _FEATUREEXTRACTER_H
#define _FEATUREEXTRACTER_H

namespace recognise{

	class FeatureExtracter
	{
	public:
		static const int s_NORMSIZE = 64;

		static void temp(const char* imageData){
			extractFeature(imageData);
		}

	// need modifid: private:
	private:
		static const int s_STRIPESIZE = 8;

		static const int s_GRIDSIZE = 4;

		static const int s_SUBVCOUNT = 8;

		enum Direction{ NORTH = 0, SOUTH, WEST, EAST };

		enum Line{ LDOWN_RUP = 0, LUP_RDOWN, LEFT_RIGHT, UP_DOWN };

		enum BlackJumpType{ SUB_TOTAL = 0, SUB_DIVIDED };

		enum HV{ HORIZONTAL = 0, VERTICAL, SLANTING = VERTICAL };

		// the feature describes a character image 
		typedef struct feature{
			int strokeWidth;								// stroke width
			int totalStrokeLen;								// total stroke length
			int projHist[2][s_NORMSIZE];				// projection histogram in two directions
			int transitions[2];								// number of transitions in two directions
			int strokeDensity[2][s_STRIPESIZE];			// stroke density in two directions
			int peripheral[4][s_STRIPESIZE][2];			// two peripheral features with four directions
			int locDir[s_GRIDSIZE][s_GRIDSIZE][4];			// local direction contributivity with four regions and four directions
			float strokeProp[2][s_GRIDSIZE][4];			// stroke proportion in two directions  
			int maxLocDirCtr[s_GRIDSIZE][s_GRIDSIZE][4];	// maximum local direction contributivity
			int totalBlackJump[2][s_SUBVCOUNT];				// black jump distribution in each balanced subvectors  
			float divBlackJump[2][s_SUBVCOUNT];				// black jump distribution in each balanced subvectors divided by the total 
		}Feature;

		FeatureExtracter(void){ };
		~FeatureExtracter(void){ };

		static Feature* extractFeature(const char* imageData);

		static void calcStrokeWidthAndLen(const char* imageData, int* strokeWidth, int* totalStrokeLen);

		static void calcProjHist(const char* imageData, int projHist[][s_NORMSIZE]);

		static void calcTransDensAndPeri(const char* imageData, int* transitions, int strokeDensity[][s_STRIPESIZE], int peripheral[][s_STRIPESIZE][2]);
	
		static void calcLocDirPropAndMaxLocDir(const char* imageData, int locDir[][s_GRIDSIZE][4], float strokeProp[][s_GRIDSIZE][4], int maxLocDirCtr[][s_GRIDSIZE][4]);
	
		static void calcBlackJump(const char* imageData, int totalBlackJump[][s_SUBVCOUNT], float divBlackJump[][s_SUBVCOUNT]);
	};

}

#endif