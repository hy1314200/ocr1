#ifndef _OCRTOOLKIT_H
#define _OCRTOOLKIT_H

#include <vector>

using namespace std;

class OCRToolkit
{
public:

	/**
	 * Recognize the single line characters embedded in the grey picture array
	 * NOTICE: Manually release the space located to wchar_t** res
	 * 
	 * @param greys Binary grey picture
	 * @param iWidth The width of the picture
	 * @param iHeight The height of the picture
	 * @param res The recognized character array's pointer 
	 *
	 * @return the length of the recognized character array
	 */
	static int recognise(char* greys, int iWidth, int iHeight, wchar_t** res);

private:
	static const float s_SCALETHRESHOLD;

	OCRToolkit(){ }
	~OCRToolkit(){ }

};

#endif