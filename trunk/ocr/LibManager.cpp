#include "LibManager.h"
#include "FontLib.h"

#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>

using namespace std;
using namespace library;

void LibManager::appendChars(const char *appendFilePath)
{
	const int chUnicodeSize = s_chUnicBegin - s_chUnicEnd;
	bitset<chUnicodeSize> maxbs;
	bitset<chUnicodeSize> currbs;

	wchar_t wc;
	vector<wchar_t> exist, notValid;

	wifstream wmaxifs(FontLib::s_maxLibFilePath);
	while(!wmaxifs.eof()){
		wmaxifs >> wc;

		maxbs.set(wc - s_chUnicBegin);
	}
	wmaxifs.close();

	wifstream wcurrifs(FontLib::s_currLibFilePath);
	while(!wcurrifs.eof()){
		wcurrifs >> wc;

		currbs.set(wc - s_chUnicBegin);
	}
	wcurrifs.close();

	wifstream wappifs(appendFilePath);
	while(!wappifs.eof()){
		wappifs >> wc;

		if(wc < s_chUnicBegin || wc > s_chUnicEnd || maxbs.test(wc) == false){
			notValid.push_back(wc);
			continue;
		}

		if(currbs.test(wc)){
			exist.push_back(wc);
			continue;
		}

		currbs.set(wc - s_chUnicBegin);
	}
	wappifs.close();

	int len = exist.size();
	cout << "exist = ";
	for(int i = 0; i<len; i++){
		wcout << exist[i];
	}

	len = notValid.size();
	cout << "not valid = ";
	for(int i = 0; i<len; i++){
		wcout << notValid[i];
	}

	wofstream wcurrofs(FontLib::s_currLibFilePath);
	for(wchar_t i = 0; i<chUnicodeSize; i++){
		if(currbs.test(i)){
			wcurrofs << s_chUnicBegin + i;
		}
	}
	wcurrofs.close();

	FontLib::genCurrFontLib();
}
