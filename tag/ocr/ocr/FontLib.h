#ifndef _FONTLIB_H
#define _FONTLIB_H

#include <stdio.h>
#include <vector>
#include <cxcore.h>

using namespace std;

namespace library{

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

		Char *clone()
		{
			char *imageData = new char[s_CHARSIZE*s_CHARSIZE];
			memcpy(imageData, m_imageData, s_CHARSIZE*s_CHARSIZE);

			return new Char(m_value, imageData);
		}

		void storeData(FILE* file);

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
		/** now, Song, Hei Ti, Fang Song, Kai Ti, Li Shu is supported */
		static const int s_TYPEKIND = 5;
		enum Typeface{
			SONGTI = 0,
			HEITI,
			FANGSONG,
			KAITI,
			LISHU
		};

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
		
		static FontLib *genFontLib(Typeface typeface);

		virtual ~FontLib();

		// store to currLibDir
		bool storeData();
		bool storeData(const char* filepath);

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

		static FontLib *parseIntFile(FILE *file);

		static void buildSubset(FILE *file, wstring subset);

		static void retrieveFilePath(Typeface typeface, const char *dir, string &filePath);

		FontLib(Typeface typeface)
		{
			m_typeface = typeface;
			m_charArray = new vector<Char *>;
		}

		FontLib(Typeface typeface, vector<Char*> *charArray){
			m_typeface = typeface;
			m_charArray = charArray;
		}
	};

}

#endif