#include "ConfigFile.h"

#include <fstream>
#include <sstream>

using namespace util;

ConfigFile *ConfigFile::parseConfig(const char *filePath) throw (string)
{
	ifstream ifs(filePath);
	if(!ifs){
		stringstream ss;
		ss << "ERROR: can not find config file \"" << filePath << "\"";

		throw ss.str();
	}

	string temp, key, value;
	string::size_type offset1, offset2, offset3;
	int line = 0;
	ConfigFile *config = new ConfigFile;

	while(!ifs.eof()){
		getline(ifs, temp);
		++line;	// begin with 1

		if(temp.size() == 0){
			continue;
		}

		offset1 = temp.find_first_not_of(" ");
		if(offset1 == string::npos || temp[offset1] == '#'){
			continue;
		}

		offset2 = temp.find_first_of("=");

		if(offset2 == string::npos || temp.find_first_of("=", offset2+1) != string::npos){
			stringstream ss;
			ss << "ERROR: at line " << line << ": invalid format";

			throw ss.str();
		}

		for(offset3 = offset2-1; offset3 >= offset1 && temp[offset3] == ' '; offset3--){   }
		//offset3 = temp.find_last_not_of(" ", offset1, offset2 - offset1); // something wrong with unicode
		key.assign(temp, offset1, offset3-offset1+1);

		offset1 = temp.find_first_not_of(" ", offset2+1);
		for(offset2 = temp.size()-1; offset3 >= 0 && temp[offset3] == ' '; offset3--){   }
		//offset2 = temp.find_last_not_of(" "); // something wrong with unicode
		value.assign(temp, offset1, offset2-offset1+1);

		if(config->put(key, value) == false){
			stringstream ss;
			ss << "ERROR: at line " << line << ": duplicated key \"" << key << "\"";

			throw ss.str();
		}
	}

	return config;
}

bool ConfigFile::put(string key, string value)
{
	pair<map<string ,string>::iterator, bool> res;

	res = m_config.insert(pair<string ,string>(key, value));

	return res.second;
}

void ConfigFile::store(const char *filePath)
{
	ofstream ofs(filePath);

	for(map<string, string>::iterator itr = m_config.begin(); itr != m_config.end(); itr++){
		ofs << itr->first << "=" << itr->second << endl;
	}
}