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
#define BAR_VERSION      1
#define BAR_REVISION     1
#define BAR_MODIFICATION 0


//Here I will define all of the output data types once the database is written
#define NDPluginBarBarcodeMessage1String "BARCODE_MESSAGE1" //asynOctet
#define NDPluginBarBarcodeType1String "BARCODE_TYPE1" //asynOctet
#define NDPluginBarBarcodeMessage2String "BARCODE_MESSAGE2" //asynOctet
#define NDPluginBarBarcodeType2String "BARCODE_TYPE2" //asynOctet
#define NDPluginBarBarcodeMessage3String "BARCODE_MESSAGE3" //asynOctet
#define NDPluginBarBarcodeType3String "BARCODE_TYPE3" //asynOctet
#define NDPluginBarBarcodeMessage4String "BARCODE_MESSAGE4" //asynOctet
#define NDPluginBarBarcodeType4String "BARCODE_TYPE4" //asynOctet
#define NDPluginBarBarcodeMessage5String "BARCODE_MESSAGE5" //asynOctet
#define NDPluginBarBarcodeType5String "BARCODE_TYPE5" //asynOctet
#define NDPluginBarNumberCodesString "NUMBER_CODES" //asynInt32
#define NDPluginBarInvertedBarcodeString "INVERTED_CODE" //asynInt32
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

		void processCallbacks(NDArray *pArray);

	protected:

		//in this section, once I define the database values, I will have to define them here
		//message contained in bar code and its type
		int NDPluginBarBarcodeMessage1;
		int NDPluginBarBarcodeType1;

		int NDPluginBarBarcodeMessage2;
		int NDPluginBarBarcodeType2;

		int NDPluginBarBarcodeMessage3;
		int NDPluginBarBarcodeType3;

		int NDPluginBarBarcodeMessage4;
		int NDPluginBarBarcodeType4;

		int NDPluginBarBarcodeMessage5;
		int NDPluginBarBarcodeType5;

		//number of codes found
		int NDPluginBarNumberCodes;

		//white on black/ black on white
		int NDPluginBarInvertedBarcode;

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
		void show_bar_codes(Mat &im, vector<bar_QR_code> &codes_in_image);
		bool check_past_code(string data);
		Mat fix_inverted(Mat &im);
		void push_corners(bar_QR_code &discovered, Image::SymbolIterator &symbol);

};

#endif
