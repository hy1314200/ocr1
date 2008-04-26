

namespace library{

	class LibManager
	{
	public:
		static void appendChars(const char *appendFilePath);

	private:
		static const wchar_t s_chUnicBegin = 0x4E00;
		static const wchar_t s_chUnicEnd = 0x9FA5;

		LibManager(void){  };
		~LibManager(void){  };
	};

}