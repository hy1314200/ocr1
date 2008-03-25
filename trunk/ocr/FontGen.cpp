#include "FontGen.h"
#include "OCRToolkit.h"

#include <cv.h>

using namespace generate;

FontGen* FontGen::s_instance = NULL;

FontGen::FontGen(void)
{
	fontLib[FANGSONG] = NULL;
	fontLib[SONGTI] =  NULL;
	fontLib[KAITI] = NULL;
	fontLib[HEITI] = NULL;
	fontLib[LISHU] = NULL;
}

FontGen::~FontGen(void)
{
	char* data = NULL;
	Char* temp = NULL;
	CharArray* arr = NULL;

	for(int i = 0; i<TYPEKIND; i++){
		if(fontLib[i] != NULL){

			for(int j = 0; j<2; j++){
				if(j == 0){
					arr = fontLib[i]->thinCharArray;
				}else{
					arr = fontLib[i]->wideCharArray;
				}

				if(arr != NULL){
					int size = arr->size;

					for(int k = 0; k<size; k++){
						temp = arr->items + k;

						if(temp->data != NULL){
							delete[] temp->data;
						}

						if(temp->image != NULL){
							cvReleaseImage(&(temp->image));
						}

						delete temp;
					}

					delete arr;
				}
			}

			delete fontLib[i];
		}
	}
}

FontGen* FontGen::getInstance()
{
	if(s_instance == 0){
		s_instance = new FontGen;

		FILE* file = fopen("data/font/fangsong", "r");
		if(file != NULL){
			s_instance->installFont(file, SONGTI);
			fclose(file);
		}

		file = fopen("data/font/songti", "r");
		if(file != NULL){
			s_instance->installFont(file, SONGTI);
			fclose(file);
		}

		file = fopen("data/font/kaishu", "r");
		if(file != NULL){
			s_instance->installFont(file, SONGTI);
			fclose(file);
		}

		file = fopen("data/font/heiti", "r");
		if(file != NULL){
			s_instance->installFont(file, SONGTI);
			fclose(file);
		}

		file = fopen("data/font/lishu", "r");
		if(file != NULL){
			s_instance->installFont(file, SONGTI);
			fclose(file);
		}
	}

	return s_instance;
}

bool FontGen::genFont(IplImage* image, wchar_t code, Typeface typeface)
{
	int index;
	if(isThinChar(code)){
		index = findCharIndex(fontLib[typeface]->thinCharArray, code);
	}else{
		index = findCharIndex(fontLib[typeface]->wideCharArray, code);
	}

	return genFont(image, index, typeface);
}

bool FontGen::genFont(IplImage* image, int index, Typeface typeface)
{
	if(image->width != image->height || image->width != s_CHARSIZE || image->imageData == NULL){
		return false;
	}


	return false;
}

bool FontGen::genFontLib(Typeface typeface){

	return false;
}

void FontGen::genFontLibAll()
{
	for(int i = 0; i<TYPEKIND; i++){
		if(fontLib[i]){
			genFontLib((Typeface)i);
		}
	}
}

void FontGen::installFont(FILE* file, Typeface typeface)
{
	const char* offset = NULL;
	const int thinW = 4, wideW = 8;
	char temp[50];
	int value, width, len = strlen("EX_FONT_UNICODE_VAL(0x0000)");
	CharArray* carray = NULL;

	fontLib[typeface] = new FontLib;
	fontLib[typeface]->thinCharArray = new CharArray;
	fontLib[typeface]->wideCharArray = new CharArray;

	while(true){
		int res = fscanf(file, "%s", temp);

		if(res == EOF){
			break;
		}						
		
		if(strlen(temp) == len){	// len = strlen("EX_FONT_UNICODE_VAL(0x0000)")
			offset = temp + 20;
			temp[len - 1] = '\0';

			value = atoi(offset);

			if(isThinChar(value)){
				width = thinW;

				carray = fontLib[typeface]->thinFontData;
			}else{
				width = wideW;

				carray = fontLib[typeface]->wideFontData;
			}

			char c;
			fscanf(file, "%c", &c);
			assert(c == '{');

			int t;
			for(int i = 0; i<s_CHARSIZE; i++){
				for(int j = 0; j<width; j++){
					fscanf(file, "%d", &t);


				}

				fscanf(file, "%c", c);
				assert(c == ',' || c == '}');
			}
		}
	}
}

int generate::FontGen::findCharIndex(CharArray* charArray, wchar_t code)
{
	int low = 0;
	int hign = charArray->size;
	int mid = (low + hign) / 2;

	if(charArray == NULL) return -1;

	if(charArray != NULL)
	{
		while(low <= hign)
		{
			if(charArray->items[mid].value == code)
			{
				return mid;
			}

			if(charArray->items[mid].value > code)
			{
				hign = mid - 1;
			}
			else
			{
				low = mid + 1;
			}

			mid = (low + hign) / 2;
		}
	}

	return -1;
}
