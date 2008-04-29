#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

#include "DebugToolkit.h"
#include "FontLib.h"

using namespace std;
using namespace generate;

void convert(const char *extDir, const char *intDir)
{
	string extDirPath(extDir), intDirPath(intDir), filePath;
	FontLib *lib = NULL;

	// songti
	filePath = extDirPath + "/songti.ext";
	FILE *file = fopen(filePath.c_str(), "r");
	assert(file != NULL);

	cout << "songti.ext:" << endl;
	lib = FontLib::parseExtFile(file, FontLib::SONGTI);

	// generate library file
	filePath = intDirPath + "/library";
	wofstream ofs(filePath.c_str());
	ofs.imbue(locale("chs"));

	vector<Char *> *list = lib->charArray();
	for(vector<Char *>::iterator itr = list->begin(); itr != list->end(); itr++){
		ofs << (wchar_t)(*itr)->value();
	}

	ofs.close();

	fclose(file);

	filePath = intDirPath + "/songti.int";
	lib->storeData(filePath.c_str());

	// check if the lastest character generated successfully
	DebugToolkit::displayGreyImage(lib->charArray()->at(list->size()-1)->imageData(), Char::s_CHARSIZE, Char::s_CHARSIZE);

	delete lib;

	// heiti
	filePath = extDirPath + "/heiti.ext";
	file = fopen(filePath.c_str(), "r");
	assert(file != NULL);

	cout << "\nheiti.ext:" << endl;
	lib = FontLib::parseExtFile(file, FontLib::HEITI);

	fclose(file);

	filePath = intDirPath + "/heiti.int";
	lib->storeData(filePath.c_str());

	delete lib;

	// fangsong
	filePath = extDirPath + "/fangsong.ext";
	file = fopen(filePath.c_str(), "r");

	if(file != NULL){
		cout << "\nfangsong.ext:" << endl;
		lib = FontLib::parseExtFile(file, FontLib::FANGSONG);

		fclose(file);

		filePath = intDirPath + "/fangsong.int";
		lib->storeData(filePath.c_str());

		delete lib;
	}else{
		cout << "\nINFO: can not find file \"fangsong.ext\"" << endl;
	}

	// kaiti
	filePath = extDirPath + "/kaiti.ext";
	file = fopen(filePath.c_str(), "r");

	if(file != NULL){
		cout << "\nkaiti.ext:" << endl;
		lib = FontLib::parseExtFile(file, FontLib::KAITI);

		fclose(file);

		filePath = intDirPath + "/kaiti.int";
		lib->storeData(filePath.c_str());

		delete lib;
	}else{
		cout << "\nINFO: can not find file \"kaiti.ext\"" << endl;
	}

	// lishu
	filePath = extDirPath + "/lishu.ext";
	file = fopen(filePath.c_str(), "r");

	if(file != NULL){
		cout << "\nlishu.ext:" << endl;
		lib = FontLib::parseExtFile(file, FontLib::LISHU);

		fclose(file);

		filePath = intDirPath + "/lishu.int";
		lib->storeData(filePath.c_str());

		delete lib;
	}else{
		cout << "\nINFO: can not find file \"lishu.ext\"" << endl;
	}
}

void displayIntFile(int index, const char *filePath)
{
	FILE *file = fopen(filePath, "rb");
	assert(file != NULL);

	FontLib *lib = FontLib::parseIntFile(file);
	
	vector<Char *> *list = lib->charArray();
	if(index > list->size()){
		cerr << "ERROR: index " << index << " is bigger than library's size, please try under " << list->size() << endl;
		return;
	}

	DebugToolkit::displayGreyImage(list->at(index)->imageData(), Char::s_CHARSIZE, Char::s_CHARSIZE);
}


int main(int argc, char** argv){
	if(argc%2 == 0){
		cerr << "Usage:" << endl;

		return 1;
	}

	if(argc == 1 || strcmp(argv[1], "-c") == 0){
		string str(".");

		if(argc > 4 && strcmp(argv[3], "-o") == 0){
			str = argv[4];
		}
		
		if(argc == 1){
			cout << "Default execute: ocr -c . -o ." << endl;

			convert(".", ".");
		}else{
			convert(argv[2], str.c_str());
		}
		
	}else if(strcmp(argv[1], "-d") == 0){
		assert(argc > 4 && strcmp(argv[3], "-f") == 0);

		int index = atoi(argv[2]);

		displayIntFile(index, argv[4]);
	}
}