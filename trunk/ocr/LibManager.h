

namespace library{

	class LibManager
	{
	public:
		static void appendChars(const char *appendFilePath);

	private:
		static const wchar_t s_chUnicBegin = 0x4E00;
		static const wchar_t s_chUnicEnd = 0x9FA5;
		static const char *s_currLibPath;
		static const char *s_maxLibPath;

		LibManager(void){  };
		~LibManager(void){  };
	};

}