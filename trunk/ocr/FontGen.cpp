#include "FontGen.h"
#include "OCRToolkit.h"
#include "DebugToolkit.h"

#include <string>
#include <cv.h>

using namespace std;
using namespace library;

const char *FontLib::s_currLibDirPath = "data/font/curr";

FontLib::~FontLib(){
	int size = m_charArray->size();

	for(int i = 0; i<size; i++){
		delete m_charArray->at(i);
	}

	delete m_charArray;
}

Char* FontLib::_Char::parseIntFile(FILE* file)
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

FontLib* FontLib::parseIntFile(FILE* file)
{
	Typeface typeface;
	vector<Char*> *cArray;
	int size;

	fread(&typeface, sizeof(Typeface), 1, file);

	fread(&size, sizeof(int), 1, file);
	cArray = new vector<Char*>();

	for(int i = 0; i<size; i++){
		cArray->push_back(_Char::parseIntFile(file));
	}

	return new FontLib(typeface, cArray);
}

FontLib* FontLib::genCurrFontLib(Typeface typeface)
{
	string dirPath(s_currLibDirPath);
	string filePath;

	switch(typeface)
	{
	case SONGTI:
		filePath += dirPath + "/songti.int";
		break;

	case HEITI:
		filePath += dirPath + "/heiti.int";
		break;

	case FANGSONG:
		filePath += dirPath + "/fangsong.int";
	    break;

	case KAITI:
		filePath += dirPath + "/kaiti.int";
	    break;

	case LISHU:
		filePath = dirPath + "/lishu.int";
		break;

	default:
	    assert(false);
	}

	FILE *file = fopen(filePath.c_str(), "r");
	assert(file);

	return FontLib::parseIntFile(file);
}