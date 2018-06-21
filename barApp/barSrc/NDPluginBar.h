/*
 * NDPluginBar.h
 *
 * Header file for EPICS Bar/QR reader plugin
 * Author: Jakub Wlodek
 *
 * Created December 5, 2017
*/

#ifndef NDPluginBar_H
#define NDPluginBar_H


//two includes
#include <opencv2/opencv.hpp>
#include <zbar.h>

using namespace std;
using namespace cv;
using namespace zbar;

//include base plugin driver
#include "NDPluginDriver.h"


//version numbers
#define BAR_VERSION      0
#define BAR_REVISION     2
#define BAR_MODIFICATION 1


//Here I will define all of the output data types once the database is written
#define NDPluginBarBarcodeMessageString "BARCODE_MESSAGE" //asynOctet
#define NDPluginBarBarcodeTypeString "BARCODE_TYPE" //asynOctet
#define NDPluginBarNumberCodesString "NUMBER_CODES" //asynInt32
#define NDPluginBarBarcodeFoundString "BARCODE_FOUND" //asynInt32
#define NDPluginBarUpperLeftXString "UPPER_LEFT_X" //asynInt32
#define NDPluginBarUpperRightXString "UPPER_RIGHT_X" //asynInt32
#define NDPluginBarLowerLeftXString "LOWER_LEFT_X" //asynInt32
#define NDPluginBarLowerRightXString "LOWER_RIGHT_X" //asynInt32
#define NDPluginBarUpperLeftYString "UPPER_LEFT_Y" //asynInt32
#define NDPluginBarUpperRightYString "UPPER_RIGHT_Y" //asynInt32
#define NDPluginBarLowerLeftYString "LOWER_LEFT_Y" //asynInt32
#define NDPluginBarLowerRightYString "LOWER_RIGHT_Y" //asynInt32

//structure that contains information about the bar/QR code
typedef struct{
	string type;
	string data;
	vector <Point> position;
}bar_QR_code;

//class that does barcode readings
class NDPluginBar : public NDPluginDriver {
	public:
		NDPluginBar(const char *portName, int queueSize, int blockingCallbacks,
			const char* NDArrayPort, int NDArrayAddr, int maxBuffers,
			size_t maxMemory, int priority, int stackSize);
		//void decode_bar_code(Mat &im, vector<bar_QR_code> &codes_in_image);
		//Mat show_bar_codes(Mat &im, vector<bar_QR_code> &codes_in_image);
		void processCallbacks(NDArray *pArray);

	protected:

		//in this section, once I define the database values, I will have to define them here
		//message contained in bar code
		int NDPluginBarBarcodeMessage;

		//type of bar code i.e. QR, BAR
		int NDPluginBarBarcodeType;

		//if there is a bar code found
		int NDPluginBarBarcodeFound;

		//number of codes found
		int NDPluginBarNumberCodes;

		//upper left pixel of found bar code
		int NDPluginBarUpperLeftX;

		//upper right pixel of found bar code
		int NDPluginBarUpperRightX;

		//lower left pixel of found bar code
		int NDPluginBarLowerLeftX;

		//lower right pixel of found bar code
		int NDPluginBarLowerRightX;

		//upper left pixel of found bar code
		int NDPluginBarUpperLeftY;

		//upper right pixel of found bar code
		int NDPluginBarUpperRightY;

		//lower left pixel of found bar code
		int NDPluginBarLowerLeftY;

		//lower right pixel of found bar code
		int NDPluginBarLowerRightY;
	private:
		void decode_bar_code(Mat &im, vector<bar_QR_code> &codes_in_image);


};

#endif
