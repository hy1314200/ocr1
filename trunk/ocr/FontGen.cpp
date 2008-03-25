#include "FontGen.h"
#include "OCRToolkit.h"

#include <cv.h>

using namespace generate;

bool FontGen::genExtFontLib(FontLib* fontLib, FILE* file)
{
	if(fontLib == NULL || file == NULL){
		return false;
	}

	fontLib->thinCharArray = new CharArray;
	fontLib->wideCharArray = new CharArray;

	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	char c, temp[50];
	int t, width, count;
	CharArray* carray = NULL;
	Char* item = NULL;
	IplImage* image = NULL;

	while(fscanf(file, "%s", temp) != EOF){

		if(strcmp(temp, "struct") == 0){

			fscanf(file, "%s", temp);
			if(temp[1] == 't'){
				width = 4;	// thin compacted 01 data with 4 elements in a line
				carray = fontLib->thinCharArray;
			}else{
				assert(temp[1] == 'w');

				width = 8;	//wide compacted 01 data with 8 elements in a line
				carray = fontLib->wideCharArray;
			}

			for(int i = 0; i<3; i++){
				while((c = fgetc(file)) != '['){  }
			}

			fscanf(file, "%d", &count);
			carray->size = count;
			carray->items = new Char[count];

			for(int i = 0; i<count; i++){
				image = (carray->items + i)->image;

				image = cvCreateImage(cvSize(s_CHARSIZE, s_CHARSIZE), 8, 1);
				cvSet(image, cvScalar(backColor));
			}

			for(int i = 0; i<count; i++){
				item = carray->items + count;

				while((c = fgetc(file)) != 'x'){  }	// EX_FONT_UNICODE_VAL(0x0000)

				fscanf(file, "%xd", &(item->value));

				char* data = item->image->imageData;
				int widthStep = item->image->widthStep;

				for(int j = 0; j<s_CHARSIZE; j++){
					for(int k = 0; k<width; k++){

						while((c = fgetc(file)) != 'x'){  }

						fscanf(file, "%xd", &t);

						if(t != 0x00){
							int s = 0x80;

							for(int l = 0; l<8; l++, s >> 1){
								if(t & s != 0){
									*(data + widthStep*i + 8*k + l) = charColor;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;
}

bool FontGen::genFontLib(FontLib* fontLib, FILE* file)
{
	return false;
}

bool FontGen::storeFontLib(const char* path, const FontLib* fontLib)
{
	return false;
}

void FontGen::releaseFontLib(FontLib* fontLib)
{
	if(fontLib->thinCharArray != NULL){
		releaseCharArray(fontLib->thinCharArray);
	}

	if(fontLib->wideCharArray != NULL){
		releaseCharArray(fontLib->wideCharArray);
	}

	delete fontLib;
}

void FontGen::releaseCharArray(CharArray* charArray)
{
	if(charArray->size > 0){
		assert(charArray->items != NULL);

		Char* temp = charArray->items;
		int size = charArray->size;

		for(int i = 0; i<size; i++){
			cvReleaseImage(&((temp + i)->image));

			(temp + i)->image = NULL;

			delete temp;
		}
	}

	delete charArray;
}

int FontGen::findCharIndex(CharArray* charArray, wchar_t code)
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
