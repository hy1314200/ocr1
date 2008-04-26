#ifndef _FONTGEN_H
#define _FONTGEN_H

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
		int size(){
			return m_charArray->size();
		}

		Typeface typeface(){
			return m_typeface;
		}

		vector<Char *> *charArray(){
			return m_charArray;
		}
		
		static FontLib *genCurrFontLib(Typeface typeface);

		static FontLib *genSubFontLib(Typeface typeface, vector<wchar_t> *subList);

		virtual ~FontLib();

	private:
		
		class _Char: public Char
		{
		public:
			_Char(wchar_t value, char *imageData): Char(value, imageData){  };

			static Char *parseIntFile(FILE *file);

		};

		static const char *s_maxLibDirPath;
		static const char *s_currLibDirPath;

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