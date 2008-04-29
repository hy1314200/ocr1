#include "FontLib.h"
#include "DebugToolkit.h"

#include <cv.h>

using namespace generate;

FontLib::~FontLib(){
	int size = m_charArray->size();

	for(int i = 0; i<size; i++){
		delete m_charArray->at(i);
	}

	delete m_charArray;
}

Char* Char::parseExtFile(FILE* file, int widthOfLine)
{
	char charColor = Char::s_CHARACTERCOLOR;
	char backColor = Char::s_BACKGROUNDCOLOR;

	char c, lastC;
	wchar_t value;
	char* data = new char[s_CHARSIZE*s_CHARSIZE];

	// EX_FONT_UNICODE_VAL(0x0000)
	while(true){
		while((c = fgetc(file)) != 'x'){
			lastC = c;
		}

		if(lastC == '0'){
			break;
		}
	}

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

	return new Char(value, data);
}

Char* Char::parseIntFile(FILE* file)
{
	char charColor = s_CHARACTERCOLOR;
	char backColor = s_BACKGROUNDCOLOR;

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

	return new Char(value, data);
}

void Char::storeData(FILE* file)
{
	char charColor = Char::s_CHARACTERCOLOR;
	char backColor = Char::s_BACKGROUNDCOLOR;

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

FontLib* FontLib::parseExtFile(FILE* file, Typeface typeface)
{
	char c, temp[50];
	int width, count;
	vector<Char*> *cArray = new vector<Char*>;
	Char* item = NULL;

	while(true){
		if(feof(file)){
			break;
		}

		fscanf(file, "%s", temp);

		if(strcmp(temp, "struct") == 0){

			fscanf(file, "%s", temp);
			if(temp[1] == 't'){
				width = 4;	// thin compacted 01 data with 4 elements in a line
			}else{
				assert(temp[1] == 'w');

				width = 8;	//wide compacted 01 data with 8 elements in a line
			}

			for(int i = 0; i<3; i++){
				while((c = fgetc(file)) != '['){  }
			}

			fscanf(file, "%d", &count);

			printf("ext file parsing process:\n");
			for(int i = 0; i<count; i++){
				cArray->push_back(Char::parseExtFile(file, width));

				if(i%500 == 0){
					printf("%.2f%% finished\n", i*100*1.0/count);
				}
			}
			printf("100%% finished\n");
		}
	}

	return new FontLib(typeface, cArray);
}

FontLib* FontLib::parseIntFile(FILE* file)
{
	Typeface typeface;
	fread(&typeface, sizeof(Typeface), 1, file);

	int size;
	fread(&size, sizeof(int), 1, file);

	vector<Char*> *cArray = new vector<Char*>();
	for(int i = 0; i<size; i++){
		cArray->push_back(Char::parseIntFile(file));
	}

	return new FontLib(typeface, cArray);
}

bool FontLib::storeData(const char* filepath){
	FILE* file = fopen(filepath, "wb");
	if(file == NULL){
		return false;
	}

	fwrite(&m_typeface, sizeof(Typeface), 1, file);

	int size;
	
	size = m_charArray->size();
	fwrite(&size, sizeof(int), 1, file);

	size = m_charArray->size();
	for(int i = 0; i<size; i++){
		m_charArray->at(i)->storeData(file);
	}

	fclose(file);

	return true;
}

int FontLib::findCharIndex(wchar_t code)
{
	int low = 0;
	int hign = m_charArray->size();
	int mid = (low + hign) / 2;

	if(m_charArray == NULL) return -1;

	if(m_charArray != NULL)
	{
		while(low <= hign)
		{
			if(m_charArray->at(mid)->value() == code)
			{
				return mid;
			}

			if(m_charArray->at(mid)->value() > code)
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
