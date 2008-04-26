#ifndef _FONTLIB_H
#define _FONTLIB_H

#include <stdio.h>
#include <vector>
#include <cxcore.h>

using namespace std;

namespace library{

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

		char *imageData(){
			return m_imageData;
		}

		virtual ~Char(){
			delete m_imageData;
		}

	protected:
		/** the unicode value of this item */
		wchar_t m_value;

		/** the binary grey data of this item */
		char *m_imageData;

		Char(wchar_t value, char *imageData){
			m_value = value;
			m_imageData = imageData;
		};
	};

	class FontLib {
	public:
		static const char *s_maxLibDirPath;
		static const char *s_maxLibFilePath;
		static const char *s_currLibDirPath;
		static const char *s_currLibFilePath;

		int size(){
			return m_charArray->size();
		}

		Typeface typeface(){
			return m_typeface;
		}

		vector<Char *> *charArray(){
			return m_charArray;
		}

		static void genCurrFontLib();
		
		static FontLib *genCurrFontLib(Typeface typeface);

		virtual ~FontLib();

	private:
		
		class _Char: public Char
		{
		public:
			_Char(wchar_t value, char *imageData): Char(value, imageData){  };

			static Char *parseIntFile(FILE *file);

		};

		Typeface m_typeface;

		/** wide font data: font data of multibytes chars */
		vector<Char *> *m_charArray;

		FontLib(Typeface typeface, vector<Char*> *charArray){
			m_typeface = typeface;
			m_charArray = charArray;
		}

		static FontLib *parseIntFile(FILE *file);
	};

}

#endif