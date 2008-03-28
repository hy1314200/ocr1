#ifndef _FONTGEN_H
#define _FONTGEN_H

#include <stdio.h>
#include <vector>
#include <cxcore.h>

using namespace std;

namespace generate{

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
		static  const int s_CHARSIZE = 64;

		wchar_t value(){
			return m_value;
		}

		char* imageData(){
			return m_imageData;
		}

		virtual void storeData(FILE* file) = 0;

		virtual ~Char(){
			delete m_imageData;
		}

	protected:
		/** the unicode value of this item */
		wchar_t m_value;

		/** the binary grey data of this item */
		char* m_imageData;

		Char(wchar_t value, char* imageData){
			m_value = value;
			m_imageData = imageData;
		};
	};

	class FontLib {
	public:
		int size(){
			return m_thinCharArray->size() + m_wideCharArray->size();
		}

		Typeface typeface(){
			return m_typeface;
		}

		vector<Char*>* thinCharArray(){
			return m_thinCharArray;
		}

		vector<Char*>* wideCharArray(){
			return m_wideCharArray;
		}

		virtual bool storeData(const char* filepath) = 0;

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
			_Char(wchar_t value, char* imageData): Char(value, imageData){  };

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