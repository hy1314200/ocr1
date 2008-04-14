#include <stdio.h>
#include <string>

int main(int argc, char **argv)
{
	const char *usage = 
		"usage:character (-i [filepath]+)|(-e filepath)\n\
  -i one or more input files need combining\n\
  -e the file contains characters needing to be checked whether they are exists\n";
	const char *cannotfind = "can not find file: %s\n";
	const char *library = "library";
	if(argc < 3){
		printf(usage);

		return 0;
	}

	setlocale(LC_ALL, "");

	const int LEFTOFF = 0x4E00;
	const int RIGHTOFF = 0x9FA5;
	const int size = RIGHTOFF - LEFTOFF +1;	// unicode range of Chinese character
	bool list[size];
	for(int i = 0; i<size; i++){
		list[i] = false;
	}

	wchar_t word;
	FILE* file = NULL;
	if(strcmp(argv[1], "-i") == 0){
		int nonwordc, wordc;

		int i;
		for(i = 2; i < argc; i++){
			file = fopen(argv[i], "r");
			if(file == NULL){
				printf(cannotfind, argv[i]);

				return 0;
			}

			nonwordc = wordc = 0;
			while(!feof(file)){
				fwscanf(file, L"%c", &word);

				if(word < LEFTOFF || word > RIGHTOFF){
					++nonwordc;
				}else{
					list[word-LEFTOFF] = true;
					++wordc;
				}
			}

			printf("%d Chinese characters, and %d not in file %s\n", wordc, nonwordc, argv[i]);

			fclose(file);
		}

		file = fopen(library, "w");

		wordc = 0;
		for(int i = 0; i<size; i++){
			if(list[i] == true){
				++wordc;

				fwprintf(file, L"%c", (wchar_t)(LEFTOFF + i));
			}
		}

		printf("total %d Chinese characters\n", wordc);

		fclose(file);
	}else if(strcmp(argv[1], "-e") == 0){
		file = fopen(library, "r");
		if(file == NULL){
			printf(cannotfind, library);

			return 0;
		}

		while(!feof(file)){
			fwscanf(file, L"%c", &word);

			list[word - LEFTOFF] = true;
		}
		fclose(file);

		file = fopen(argv[2], "r");
		if(file == NULL){
			printf(cannotfind, argv[2]);

			return 0;
		}

		const int LEN = size;
		wchar_t str[LEN];

		int wordc = 0;
		fwscanf(file, L"%s", str);

		for(int i = 0; i<LEN && str[i] != '\0'; i++){
			if(list[str[i] - LEFTOFF] == true){
				wprintf(L"%c exists\n", str[i]);
			}else{
				wprintf(L"%c not exist\n", str[i]);
			}
		}

		fclose(file);
	}else{
		printf("invalid parameter: %s\n", argv[1]);
		printf(usage);

		return 0;
	}
}