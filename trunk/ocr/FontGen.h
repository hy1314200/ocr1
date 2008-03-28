#ifndef _FONTGEN_H
#define _FONTGEN_H

#include <stdio.h>
#include <vector>
#include <cxcore.h>

using namespace std;

namespace generate{

	const int CHARSIZE = 64;

	/** now, Fang Song, Song, Kai Ti, Hei Ti, Li Shu is supported */
	static const int TYPEKIND = 5;
	enum Typeface{
		SONGTI = 0,
		FANGSONG,
		KAITI,
		HEITI,
		LISHU
	};

	class Char {
	public:
		wchar_t value(){
			return m_value;
		}

		IplImage* image(){
			return m_image;
		}

		virtual void storeData(FILE* file) = 0;

		virtual ~Char(){
			if(m_image != NULL){
				cvReleaseImage(&m_image);
			}
		}

	protected:
		/** the unicode value of this item */
		wchar_t m_value;

		/** the binary grey data of this item */
		IplImage* m_image;

		Char(wchar_t value, IplImage* image){
			m_value = value;
			m_image = image;
		};
	};

	class FontLib {
	public:
		Typeface typeface(){
			return m_typeface;
		}

		vector<Char*>* thinCharArray(){
			return m_thinCharArray;
		}

		vector<Char*>* wideCharArray(){
			return m_wideCharArray;
		}

		virtual void storeData(const char* filepath) = 0;

		virtual ~FontLib();

	protected:
		Typeface m_typeface;

		/** thin font data: font data of ASCII chars */
		vector<Char*>* m_thinCharArray;

		/** wide font data: font data of multibytes chars */
		vector<Char*>* m_wideCharArray;

		FontLib(Typeface typeface, vector<Char*>* thinCharArray, vector<Char*>* wideCharArray){
			m_typeface = typeface;
			m_thinCharArray = thinCharArray;
			m_wideCharArray = wideCharArray;
		}
	};

	class FontGen
	{
	public:
		class _Char: public Char
		{
		public:
			static _Char* parseExtFile(FILE* file, int widthOfLine);

			static _Char* parseIntFile(FILE* file);

			void storeData(FILE* file);

		private:
			_Char(wchar_t value, IplImage* image): Char(value, image){  };

		};

		class _FontLib: public FontLib
		{
		public:
			static _FontLib* parseExtFile(FILE* file, Typeface typeface);

			static _FontLib* parseIntFile(FILE* file);

			bool storeData(const char* filepath);

		private:
			_FontLib(Typeface typeface, vector<Char*>* thinCharArray, vector<Char*>* wideCharArray): FontLib(typeface, thinCharArray, wideCharArray){  };

		};

		static FontLib* genExtFontLib(FILE* file, Typeface typeface);

		static FontLib* genIntFontLib(FILE* file);

		static bool storeFontLib(const char* path, const FontLib* fontLib);

	private:
		FontGen(void){ };
		~FontGen(void){ };

		static bool isThinChar(wchar_t code){
			return code < 128;
		}

		static int findCharIndex(vector<Char*>* charArray, wchar_t code);
	};

}

#endif