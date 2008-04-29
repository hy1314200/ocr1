#ifndef _FONTLIB_H
#define _FONTLIB_H

#include <stdio.h>
#include <vector>
#include <cxcore.h>

using namespace std;

namespace generate{

	class Char {
	public:
		static const char s_CHARACTERCOLOR = (char)255;
		static const char s_BACKGROUNDCOLOR = 0;

		static  const int s_CHARSIZE = 64;

		static Char* parseExtFile(FILE* file, int widthOfLine);

		static Char* parseIntFile(FILE* file);

		virtual ~Char(){
			delete m_imageData;
		}

		wchar_t value(){
			return m_value;
		}

		char* imageData(){
			return m_imageData;
		}

		void storeData(FILE* file);

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
		/** now, Song, Hei Ti, Fang Song, Kai Ti, Li Shu is supported */
		static const int TYPEKIND = 5;
		enum Typeface{
			SONGTI = 0,
			HEITI,
			FANGSONG,
			KAITI,
			LISHU
		};

		static FontLib* parseExtFile(FILE* file, Typeface typeface);

		static FontLib* parseIntFile(FILE* file);

		int size(){
			return m_charArray->size();
		}

		Typeface typeface(){
			return m_typeface;
		}

		vector<Char*>* charArray(){
			return m_charArray;
		}

		virtual ~FontLib();

		bool storeData(const char* filepath);

		int findCharIndex(wchar_t code);

		static bool isThinChar(wchar_t code){
			return code < 128;
		}

	private:
		Typeface m_typeface;

		/** wide font data: font data of multibytes chars */
		vector<Char*>* m_charArray;

		FontLib(Typeface typeface, vector<Char*>* charArray){
			m_typeface = typeface;
			m_charArray = charArray;
		}
	};

}

#endif