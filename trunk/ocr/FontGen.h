#ifndef _FONTGEN_H
#define _FONTGEN_H

#include <cxcore.h>
#include <stdio.h>

namespace generate{

	class FontGen
	{
	public:
		static const int s_CHARSIZE = 64;

		/** now, Fang Song, Song, Kai Ti, Hei Ti, Li Shu is supported */
		static const int TYPEKIND = 5;
		enum Typeface{
			FANGSONG = 0,
			SONGTI,
			KAITI,
			HEITI,
			LISHU
		};

		static FontGen* getInstance();

		static void release(){
			if(s_instance != NULL){
				delete s_instance;

				s_instance = NULL;
			}
		}

		bool genFont(IplImage* image, wchar_t code, Typeface typeface);

		bool genFont(IplImage* image, int index, Typeface typeface);

		bool genFontLib(Typeface typeface);

		void genFontLibAll();

	private:
		static FontGen* s_instance;

		typedef struct _Char {
			/** the unicode value of this item */
			int value;

			/** compacted 01 pixel data */
			char* data;

			/** the binary grey data of this item */
			IplImage* image;
		}Char;

		typedef struct _CharArray {
			/** the total number of all items */
			int size;

			Char* items;
		}CharArray;

		typedef struct _FontLib {
			/** thin_font_data: font data of ASCII chars */
			CharArray* thinCharArray;

			/** wide_font_data: font data of multibytes chars */
			CharArray* wideCharArray;
		}FontLib;

		FontLib* fontLib[TYPEKIND];	// 仿宋、宋体、楷体、黑体、隶书

		FontGen(void);
		~FontGen(void);

		void installFont(FILE* file, Typeface typeface);

		bool isThinChar(wchar_t code){
			return code < 128;
		}

		int findCharIndex(CharArray* fontData, wchar_t code);
	};

}

#endif