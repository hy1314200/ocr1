#ifndef _CHARRECOGNISER_H
#define _CHARRECOGNISER_H

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

		/** @return Reliably, from 0% to 100% */
		float recogniseChar(const char* greys, int iWidth, int iHeight, wchar_t* res);

	private:
		static CharRecogniser* s_instance;

		CharRecogniser(void);
		~CharRecogniser(void);

		char* normalize(const char* greys, int iWidth, int iHeight);
	};

}

#endif
