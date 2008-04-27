#include "LibManager.h"
#include "FontLib.h"
#include "ConfigFile.h"
#include "GlobalCofig.h"

#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>

using namespace std;
using namespace library;
using namespace util;

void LibManager::appendChars(const char *appendFilePath)
{
	ConfigFile *config = GlobalCofig::getConfigFile();

	const wchar_t chUnicodeSize = s_chUnicBegin - s_chUnicEnd;
	bitset<chUnicodeSize> maxbs;
	bitset<chUnicodeSize> currbs;

	wstring str;
	vector<wchar_t> exist, notValid;
	int size;

	// maxLib
	wifstream wmaxifs(config->get("path.file.maxlib").c_str());
	wmaxifs.imbue(locale("chs"));
	wmaxifs >> str;
	
	size = str.size();
	for(int i = 0; i<size; i++){
		maxbs.set(str[i] - s_chUnicBegin);
	}
	wmaxifs.close();

	// currLib
	wifstream wcurrifs(config->get("path.file.currlib").c_str());
	if(wcurrifs){
		wcurrifs.imbue(locale("chs"));
		wcurrifs >> str;

		size = str.size();
		for(int i = 0; i<size; i++){
			currbs.set(str[i] - s_chUnicBegin);
		}
		wcurrifs.close();
	}

	// append
	wifstream wappifs(appendFilePath);
	wappifs.imbue(locale("chs"));
	wappifs >> str;

	size = str.size();
	for(int i = 0; i<size; i++){
		if(str[i] < s_chUnicBegin || str[i] > s_chUnicEnd || maxbs.test(str[i] - s_chUnicBegin) == false){
			notValid.push_back(str[i]);
			continue;
		}

		if(currbs.test(str[i] - s_chUnicBegin)){
			exist.push_back(str[i]);
			continue;
		}

		currbs.set(str[i] - s_chUnicBegin);
	}
	wappifs.close();

	wcout.imbue(locale("chs"));
	size = exist.size();
	if(size > 0){
		cout << "exist=";
		for(int i = 0; i<size; i++){
			wcout << exist[i];
		}
		cout << endl;
	}

	size = notValid.size();
	if(size > 0){
		cout << "not valid=";
		for(int i = 0; i<size; i++){
			wcout << notValid[i];
		}
		cout << endl;
	}

	wofstream wcurrofs(config->get("path.file.currlib").c_str());
	wcurrofs.imbue(locale("chs"));
	for(wchar_t i = 0; i<chUnicodeSize; i++){
		if(currbs.test(i)){
			wcurrofs << (wchar_t)(s_chUnicBegin + i);
		}
	}
	wcurrofs.close();

	FontLib::genCurrFontLib();
}
