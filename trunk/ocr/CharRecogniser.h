#ifndef _CHARRECOGNISER_H
#define _CHARRECOGNISER_H

#include "FontGen.h"

namespace recognise{

	class CharRecogniser
	{
	public:
		/** The entrance of getting an instance of the OCRToolkit */
		static CharRecogniser* getInstance(){
			if(s_instance == 0){
				s_instance = new CharRecogniser();
			}

			return s_instance;
		}
		static void  release(){
			if(s_instance != 0){
				delete s_instance;
				s_instance = 0;
			}
		}

		/** @return Reliably, from 0% to 100% */
		float recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res);

		void buildFeatureLib(generate::FontLib* fontLib, int size);

		void DEBUG_testDistorte(char** samples, char* prototype, int sampleSize){
			distorte(samples, prototype, sampleSize);
		}

	private:
		static CharRecogniser* s_instance;

		CharRecogniser(void);
		~CharRecogniser(void);

		void normalize(char* dst, const char* greys, int iWidth, int iHeight);
		
		/** used to generate many distorted samples for training */
		void distorte(char** samples, char* prototype, int sampleSize);

	};

}

#endif
