#ifndef _OCRTOOLKIT_H
#define _OCRTOOLKIT_H

#include "CharRecogniser.h"

#include <vector>

using namespace std;
using namespace recognise;

class OCRToolkit
{
public:
	static const char s_CHARACTERCOLOR = (char)255;
	static const char s_BACKGROUNDCOLOR = 0;

	/**
	 * Recognize the single line characters embedded in the binary grey picture array
	 * 
	 * @param greys Binary grey picture
	 * @param iWidth The width of the picture
	 * @param iHeight The height of the picture
	 * @param res The recognized character array's pointer 
	 */
	static void recognise(char* greys, int iWidth, int iHeight, vector<wchar_t> &res);

	/**
	* Recognize the single line characters embedded in the image file
	* 
	* @param filePath image file path
	* @param res The recognized character array's pointer 
	*
	* @return the length of the recognized character array
	*/
	static bool recognise(const char *filePath, vector<wchar_t> &res);

	/**
	* train the specified classifier
	*/
	static void trainClassifier()
	{
		CharRecogniser::buildInstance()->trainClassifier();
	}

private:
	static const double s_SCALETHRESHOLD;

	OCRToolkit(){ }
	~OCRToolkit(){ }

};

#endif