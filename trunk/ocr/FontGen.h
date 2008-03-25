#ifndef _FONTGEN_H
#define _FONTGEN_H

#include <stdio.h>
#include <cxcore.h>

namespace generate{

	class FontGen
	{
	public:
		static const int s_CHARSIZE = 64;

		/** now, Fang Song, Song, Kai Ti, Hei Ti, Li Shu is supported */
		static const int TYPEKIND = 5;
		enum Typeface{
			SONGTI = 0,
			FANGSONG,
			KAITI,
			HEITI,
			LISHU
		};

		typedef struct _Char {
			/** the unicode value of this item */
			wchar_t value;

			/** the binary grey data of this item */
			IplImage* image;
		}Char;

		typedef struct _CharArray {
			/** the total number of all items */
			int size;

			Char* items;
		}CharArray;

		typedef struct _FontLib {

			Typeface typeface;

			/** thin_font_data: font data of ASCII chars */
			CharArray* thinCharArray;

			/** wide_font_data: font data of multibytes chars */
			CharArray* wideCharArray;
		}FontLib;

		static bool genExtFontLib(FontLib* fontLib, FILE* file);

		static bool genFontLib(FontLib* fontLib, FILE* file);

		static bool storeFontLib(const char* path, const FontLib* fontLib);

		static void releaseFontLib(FontLib* fontLib);

	private:
		FontGen(void){ };
		~FontGen(void){ };

		static bool isThinChar(wchar_t code){
			return code < 128;
		}

		static int findCharIndex(CharArray* fontData, wchar_t code);
		
		static void releaseCharArray(CharArray* charArray);
	};

}

#endif