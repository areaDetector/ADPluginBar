/*
 * NDPluginBar.h
 *
 * Header file for EPICS Bar/QR reader plugin
 * Author: Jakub Wlodek
 * Co-author: Kazimier Gofron
 *
 * Created on: December 5, 2017
 * Last Updated: January 4, 2019
*/

#ifndef NDPluginBar_H
#define NDPluginBar_H

//two includes
#include <zbar.h>
#include <opencv2/opencv.hpp>
#include <thread>

using namespace std;
using namespace cv;
using namespace zbar;

//include base plugin driver
#include "NDPluginDriver.h"

//version numbers
#define BAR_VERSION 2
#define BAR_REVISION 2
#define BAR_MODIFICATION 0

// Number of barcodes supported at one time
#define NUM_CODES 5

/* Here I will define all of the output data types once the database is written */
#define NDPluginBarBarcodeMessage1String    "BARCODE_MESSAGE1"      //asynOctet
#define NDPluginBarBarcodeType1String       "BARCODE_TYPE1"         //asynOctet
#define NDPluginBarBarcodeMessage2String    "BARCODE_MESSAGE2"      //asynOctet
#define NDPluginBarBarcodeType2String       "BARCODE_TYPE2"         //asynOctet
#define NDPluginBarBarcodeMessage3String    "BARCODE_MESSAGE3"      //asynOctet
#define NDPluginBarBarcodeType3String       "BARCODE_TYPE3"         //asynOctet
#define NDPluginBarBarcodeMessage4String    "BARCODE_MESSAGE4"      //asynOctet
#define NDPluginBarBarcodeType4String       "BARCODE_TYPE4"         //asynOctet
#define NDPluginBarBarcodeMessage5String    "BARCODE_MESSAGE5"      //asynOctet
#define NDPluginBarBarcodeType5String       "BARCODE_TYPE5"         //asynOctet
#define NDPluginBarNumberCodesString        "NUMBER_CODES"          //asynInt32
#define NDPluginBarCodeCornersString        "CODE_CORNERS"          //asynInt32
#define NDPluginBarInvertedBarcodeString    "INVERTED_CODE"         //asynInt32
#define NDPluginBarUpperLeftXString         "UPPER_LEFT_X"          //asynInt32
#define NDPluginBarUpperRightXString        "UPPER_RIGHT_X"         //asynInt32
#define NDPluginBarLowerLeftXString         "LOWER_LEFT_X"          //asynInt32
#define NDPluginBarLowerRightXString        "LOWER_RIGHT_X"         //asynInt32
#define NDPluginBarUpperLeftYString         "UPPER_LEFT_Y"          //asynInt32
#define NDPluginBarUpperRightYString        "UPPER_RIGHT_Y"         //asynInt32
#define NDPluginBarLowerLeftYString         "LOWER_LEFT_Y"          //asynInt32
#define NDPluginBarLowerRightYString        "LOWER_RIGHT_Y"         //asynInt32


/* structure that contains information about the bar/QR code */
typedef struct {
    string type;
    string data;
    vector<Point> position;
} bar_QR_code;

/* class that does barcode readings */
class NDPluginBar : public NDPluginDriver {
    public:
        NDPluginBar(const char *portName, int queueSize, int blockingCallbacks,
                    const char *NDArrayPort, int NDArrayAddr, int maxBuffers,
                    size_t maxMemory, int priority, int stackSize, int maxThreads);

        //~NDPluginBar();

        void processCallbacks(NDArray *pArray);
        asynStatus barcode_image_callback(Mat &img, NDArray* pArrayOut);
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    protected:
        //in this section i define the coords of database vals

        //message contained in bar code and its type
        int NDPluginBarBarcodeMessage1;
        #define ND_BAR_FIRST_PARAM NDPluginBarBarcodeMessage1
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

        int NDPluginBarCodeCorners;

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

        #define ND_BAR_LAST_PARAM NDPluginBarLowerRightY

    private:
        // processing thread
        bool processing = false;

        // arrays that hold indexes of PVs for messages and types
        int barcodeMessagePVs[NUM_CODES];
        int barcodeTypePVs[NUM_CODES];

        // arrays that hold indexes for PVs for corners
        int cornerXPVs[4];
        int cornerYPVs[4];

        // vector that stores currently discovered barcodes
        vector<bar_QR_code> codes_in_image;
        asynStatus clearPreviousCodes();
        asynStatus clear_unused_barcode_pvs(int counter);

        // functions called on plugin initialization
        asynStatus initPVArrays();

        // image type conversion functions
        void printCVError(cv::Exception &e, const char *functionName);
        asynStatus ndArray2Mat(NDArray *pArray, NDArrayInfo *arrayInfo, Mat &img);
        asynStatus mat2NDArray(NDArray *pScratch, Mat &img);

        // Decoding functions
        Image scan_image(Mat &img);
        asynStatus decode_bar_codes(Mat &img);


        //function that displays detected bar codes
        asynStatus show_bar_codes(Mat &img);

        //function that allows for reading inverted barcodes
        asynStatus fix_inverted(Mat &img);

        //function that pushes barcode coordinate data to PVs
        asynStatus push_corners(bar_QR_code &discovered, Image::SymbolIterator &symbol, int update_corners, int imgHeight);
        asynStatus updateCorners(bar_QR_code &discovered, int imgHeight);
};

#define NUM_BAR_PARAMS ((int)(&ND_BAR_LAST_PARAM - &ND_BAR_FIRST_PARAM + 1))

#endif
