#include "FontGen.h"
#include "OCRToolkit.h"
#include "DebugToolkit.h"

#include <cv.h>

using namespace generate;

FontLib::~FontLib(){
	int size = m_thinCharArray->size();

	for(int i = 0; i<size; i++){
		delete m_thinCharArray->at(i);
	}

	size = m_wideCharArray->size();

	for(int i = 0; i<size; i++){
		delete m_wideCharArray->at(i);
	}

	delete m_thinCharArray;
	delete m_wideCharArray;
}

FontGen::_Char* FontGen::_Char::parseExtFile(FILE* file, int widthOfLine)
{
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	char c;
	wchar_t value;
	char* data = new char[s_CHARSIZE*s_CHARSIZE];

	while((c = fgetc(file)) != 'x'){  }	// EX_FONT_UNICODE_VAL(0x0000)

	fscanf(file, "%xd", &value);

	memset(data, backColor, s_CHARSIZE*s_CHARSIZE);

	for(int i = 0; i<s_CHARSIZE; i++){
		for(int j = 0; j<widthOfLine; j++){

			while((c = fgetc(file)) != 'x'){  }

			int t;
			fscanf(file, "%xd", &t);

			if(t != 0x00){
				int s = 0x80;

				for(int k = 0; k<8; k++, s = s >> 1){
					if((t & s) != 0){
						*(data + s_CHARSIZE*i + 8*j + k) = charColor;
					}
				}
			}
		}
	}

	return new _Char(value, data);
}

FontGen::_Char* FontGen::_Char::parseIntFile(FILE* file)
{
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	wchar_t value;
	char* data = new char[s_CHARSIZE*s_CHARSIZE];

	memset(data, backColor, s_CHARSIZE*s_CHARSIZE);

	fread(&value, sizeof(wchar_t), 1, file);

	int offset = 0, count = s_CHARSIZE*s_CHARSIZE/8;
	char temp;
	for(int i = 0; i<count; i++){
		fread(&temp, sizeof(char), 1, file);

		if(temp != 0x00){
			int s = 0x80;

			for(int j = 0; j<8; j++, s = s >> 1){
				if((temp & s) != 0){
					*(data + 8*i + j) = charColor;
				}
			}
		}
	}

	return new _Char(value, data);
}

void FontGen::_Char::storeData(FILE* file)
{
	char charColor = OCRToolkit::s_CHARACTERCOLOR;
	char backColor = OCRToolkit::s_BACKGROUNDCOLOR;

	fwrite(&m_value, sizeof(wchar_t), 1, file);

	int offset = 0, count = s_CHARSIZE*s_CHARSIZE/8;
	char temp;

	for(int i = 0; i<count; i++){
		temp = 0;

		for(int j = 0; j < 8; j++, offset++){
			temp = temp << 1;

			if(*(m_imageData + offset) == charColor){
				temp |= 1;
			}
		}

		fwrite(&temp, sizeof(char), 1, file);
	}

	//fwrite(m_image->imageData, sizeof(char), m_image->imageSize, file);
}

FontGen::_FontLib* FontGen::_FontLib::parseExtFile(FILE* file, Typeface typeface)
{
	char c, temp[50];
	int t, width, count;
	vector<Char*> *cTArray = new vector<Char*>, *cWArray = new vector<Char*>, *cArray = NULL;
	Char* item = NULL;

	while(true){
		if(feof(file)){
			break;
		}

		fscanf(file, "%s", temp);

		if(strcmp(temp, "struct") == 0){

			fscanf(file, "%s", temp);
			if(temp[1] == 't'){
				assert(false);	// ÔÝ²»Ö§³Ö

				width = 4;	// thin compacted 01 data with 4 elements in a line

				cArray = cTArray;
			}else{
				assert(temp[1] == 'w');

				width = 8;	//wide compacted 01 data with 8 elements in a line

				cArray = cWArray;
			}

			for(int i = 0; i<3; i++){
				while((c = fgetc(file)) != '['){  }
			}

			fscanf(file, "%d", &count);

			printf("ext file parsing process:\n");
			for(int i = 0; i<count; i++){
				cArray->push_back(_Char::parseExtFile(file, width));

				if(i%500 == 0){
					printf("%.2f%% finished\n", i*100*1.0/count);
				}
			}
			printf("100%% finished\n");
		}
	}

	return new _FontLib(typeface, cTArray, cWArray);
}

FontGen::_FontLib* FontGen::_FontLib::parseIntFile(FILE* file)
{
	Typeface typeface;
	vector<Char*> *cTArray, *cWArray;
	int size;

	fread(&typeface, sizeof(Typeface), 1, file);

	fread(&size, sizeof(int), 1, file);
	cTArray = new vector<Char*>();

	for(int i = 0; i<size; i++){
		cTArray->push_back(FontGen::_Char::parseIntFile(file));
	}

	fread(&size, sizeof(int), 1, file);
	cWArray = new vector<Char*>();

	for(int i = 0; i<size; i++){
		cWArray->push_back(FontGen::_Char::parseIntFile(file));
	}

	return new _FontLib(typeface, cTArray, cWArray);
}

bool FontGen::_FontLib::storeData(const char* filepath){
	FILE* file = fopen(filepath, "wb");
	if(file == NULL){
		return false;
	}

	fwrite(&m_typeface, sizeof(Typeface), 1, file);

	int size = m_thinCharArray->size();
	fwrite(&size, sizeof(int), 1, file);

	for(int i = 0; i<size; i++){
		m_thinCharArray->at(i)->storeData(file);
	}

	size = m_wideCharArray->size();
	fwrite(&size, sizeof(int), 1, file);

	for(int i = 0; i<size; i++){
		m_wideCharArray->at(i)->storeData(file);
	}

	fclose(file);

	return true;
}

FontLib* FontGen::genExtFontLib(FILE* file, Typeface typeface)
{
	return _FontLib::parseExtFile(file, typeface);
}

FontLib* FontGen::genIntFontLib(FILE* file)
{
	return FontGen::_FontLib::parseIntFile(file);
}

int FontGen::findCharIndex(vector<Char*>* charArray, wchar_t code)
{
	int low = 0;
	int hign = charArray->size();
	int mid = (low + hign) / 2;

	if(charArray == NULL) return -1;

	if(charArray != NULL)
	{
		while(low <= hign)
		{
			if(charArray->at(mid)->value() == code)
			{
				return mid;
			}

			if(charArray->at(mid)->value() > code)
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
