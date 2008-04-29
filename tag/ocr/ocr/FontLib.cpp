#include "FontLib.h"
#include "OCRToolkit.h"
#include "DebugToolkit.h"
#include "GlobalCofig.h"
#include "ConfigFile.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cv.h>

using namespace std;
using namespace library;

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

void FontLib::retrieveFilePath(Typeface typeface, const char *dir, string &filePath)
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	stringstream ss;

	ss << dir;
	switch(typeface)
	{
	case SONGTI:
		ss << "/" << config->get("filename.songti");
		break;

	case HEITI:
		ss << "/" << config->get("filename.heiti");
		break;

	case FANGSONG:
		ss << "/" << config->get("filename.fangsong");
		break;

	case KAITI:
		ss << "/" << config->get("filename.kaiti");
		break;

	case LISHU:
		ss << "/" << config->get("filename.lishu");
		break;

	default:
		assert(false);
	}

	filePath = ss.str();
}


void FontLib::genCurrFontLib()
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	wstring wstr;
	wifstream wifs(config->get("path.file.currlib").c_str());

	wifs.imbue(locale("chs"));
	getline(wifs, wstr);
	wifs.close();

	string filePath;
	FILE *file = NULL;
	for(int i = 0; i<s_TYPEKIND; i++){
		retrieveFilePath((Typeface)i, config->get("path.dir.maxlib").c_str(), filePath);

		file = fopen(filePath.c_str(), "rb");
		assert(file);

		buildSubset(file, wstr);

		fclose(file);
	}
}

FontLib *FontLib::genFontLib(Typeface typeface)
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	string filePath;
	retrieveFilePath(typeface, config->get("path.dir.currlib").c_str(), filePath);

	FILE *file = fopen(filePath.c_str(), "rb");
	assert(file);

	return parseIntFile(file);
}

void FontLib::buildSubset(FILE *file, wstring subset)
{
	FontLib *maxLib = parseIntFile(file);

	int offset = 0, size = subset.size();
	FontLib *subLib = new FontLib(maxLib->typeface());
	vector<Char *> *maxlist = maxLib->charArray(), *sublist = subLib->charArray();

	for(vector<Char *>::iterator itr = maxlist->begin(); offset < size && itr != maxlist->end(); itr++){
		wchar_t a1 = subset[offset], a2 = (*itr)->value();
		if(subset[offset] == (*itr)->value()){
			sublist->push_back((*itr)->clone());
			
			++offset;
		}
	}

	subLib->storeData();

	delete subLib;
	delete maxLib;
}

void Char::storeData(FILE* file)
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
}

bool FontLib::storeData()
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	string filePath;
	retrieveFilePath(m_typeface, config->get("path.dir.currlib").c_str(), filePath);
	return storeData(filePath.c_str());
}

bool FontLib::storeData(const char* filepath){
	FILE* file = fopen(filepath, "wb");
	if(file == NULL){
		return false;
	}

	fwrite(&m_typeface, sizeof(Typeface), 1, file);

	int size = m_charArray->size();
	fwrite(&size, sizeof(int), 1, file);

	for(int i = 0; i<size; i++){
		m_charArray->at(i)->storeData(file);
	}

	fclose(file);

	return true;
}