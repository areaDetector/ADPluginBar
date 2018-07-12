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

//EXPERIMENTAL
#define NDPluginBarDecodedBarcodesString "DECODED_BARCODES" //asynGenericPointer

//DATABASE
#define NDPluginBarEnableBarcodeDatabaseString "ENABLE_DATABASE" //asynInt32
#define NDPluginBarBarcodeDatabaseTypeString "DATABASE_TYPE" //asynInt32
#define NDPluginBarBarcodeDatabaseNameString "DATABASE_NAME" //asynOctect
#define NDPluginBarBarcodeTableNameString "TABLE_NAME" //asynOctet
//MySQL
#define NDPluginBarBarcodeSQLServerString "SQL_SERVER" //asynOctet
#define NDPluginBarBarcodeSQLUserString "SQL_USER" //asynOctet
#define NDPluginBarBarcodeSQLPassString "SQL_PASSWORD" //asynOctet

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

		//in this section i define the coords of database vals

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
		int NDPluginBarUpperLeftY;

		//upper right pixel of found bar code
		int NDPluginBarUpperRightX;
		int NDPluginBarUpperRightY;

		//lower left pixel of found bar code
		int NDPluginBarLowerLeftX;
		int NDPluginBarLowerLeftY;

		//lower right pixel of found bar code
		int NDPluginBarLowerRightX;
		int NDPluginBarLowerRightY;

		//EXPERIMENTAL
		int NDPluginBarDecodedBarcodes;

		//Databases
		int NDPluginBarEnableBarcodeDatabase;
		int NDPluginBarBarcodeDatabaseType;
		int NDPluginBarBarcodeDatabaseName;
		int NDPluginBarBarcodeTableName;

		//MySQL
		int NDPluginBarBarcodeSQLServer;
		int NDPluginBarBarcodeSQLUser;
		int NDPluginBarBarcodeSQLPass;

	private:

		//function that does the decoding
		void decode_bar_code(Mat &im, vector<bar_QR_code> &codes_in_image);

		//function that displays detected bar codes
		Mat show_bar_codes(Mat &im, vector<bar_QR_code> &codes_in_image);

		//function that checks for barcode repetition
		bool check_past_code(string data);

		//function that allows for reading inverted barcodes
		Mat fix_inverted(Mat &im);

		//function that pushes barcode coordinate data to PVs
		void push_corners(bar_QR_code &discovered, Image::SymbolIterator &symbol, int update_corners);

};

#endif
