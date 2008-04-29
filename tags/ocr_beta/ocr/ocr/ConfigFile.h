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
		static ConfigFile *parseConfig(const char *filePath) throw (string);

		ConfigFile(void){  }
		~ConfigFile(void){  }
		
		bool put(string key, string value);

		void putByForce(string key, string value)
		{
			m_config[key] = value;
		}

		bool exist(string key)
		{
			return get(key).length() > 0;
		}

		bool change(string key, string value)
		{
			if(!exist(key)){
				return false;
			}

			putByForce(key, value);
		}

		string get(string key)
		{
			map<string, string>::iterator itr = m_config.find(key);

			if(itr == m_config.end()){
				return string();
			}

			return itr->second;
		}

		bool getBool(string key) throw (string)
		{
			string value = get(key);

			if (value == "true"){
				return true;
			}else if(value == "false"){
				return false;
			}else{
				string str("ERROR: the value of key \"");
				str += key + "\" is not bool";

				throw str;
			}
		}

		int getInt(string key)
		{
			string value = get(key);

			return atoi(value.c_str());
		}

		void store(const char *filePath);

	private:
		map<string, string> m_config;

	};

}

#endif
