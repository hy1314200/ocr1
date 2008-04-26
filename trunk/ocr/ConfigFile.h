#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H

#include <string>
#include <map>

using namespace std;

namespace util
{

	class ConfigFile
	{
	public:
		static ConfigFile *parseConfig(FILE *file) throw (string);

		ConfigFile(void){  }
		~ConfigFile(void){  }
		
		bool put(string key, string value);

		void putForce(string key, string value);

		string get(string key);

		void store(const char *filePath);

	private:
		map<string, string> m_config;

	};

}

#endif
