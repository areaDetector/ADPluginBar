/*
 * NDPluginBar.cpp
 *
 * Barcode/QRcode reader plugin for EPICS area detector
 * Extends from the base NDPlugin Driver and overrides its processCallbacks function
 * The OpenCV computer vision library and the zbar barcode detection libraries are used
 *
 * Author: Jakub Wlodek
 *
 * Created December 3, 2017
 *
*/

//include some standard libraries
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <stdio.h>

//include epics/area detector libraries
#include <epicsMutex.h>
#include <epicsString.h>
#include <iocsh.h>
#include "NDArray.h"
#include "NDPluginBar.h"
#include <epicsExport.h>

//OpenCV is used for image manipulation, zbar for barcode detection
#include <opencv2/opencv.hpp>
#include <zbar.h>

//some basic namespaces
using namespace std;
using namespace cv;
using namespace zbar;
static const char *driverName="NDPluginBar";
static int previously_detected = 0;
static int database_init = 0;


/*
 * Function responsible for checking if discovered bar code is a repeat
 *
 * @params: data -> data in discovered bar code
 * @return: true if data is the same, false otherwise
 */
bool NDPluginBar::check_past_code(string data){
	string past_code1, past_code2, past_code3, past_code4, past_code5;
	getStringParam(NDPluginBarBarcodeMessage1, past_code1);
	if(data == past_code1) return true;
	getStringParam(NDPluginBarBarcodeMessage2, past_code2);
	if(data == past_code2) return true;
	getStringParam(NDPluginBarBarcodeMessage3, past_code3);
	if(data == past_code3) return true;
	getStringParam(NDPluginBarBarcodeMessage4, past_code4);
	if(data == past_code4) return true;
	getStringParam(NDPluginBarBarcodeMessage5, past_code5);
	if(data == past_code5) return true;
	else return false;
}



/*
 * Function used to use a form of thresholding to reverse the coloration of a bar code
 * or QR code that is in the white on black format rather than the standard black on white
 *
 * @params: im -> image containing inverse QR code
 * @return: inverted image
 */
Mat NDPluginBar::fix_inverted(Mat &im){
	Mat inverted(255-im);
	//imshow("temp", inverted);
	//waitKey(0);
	return inverted;
}



/*
 * Function that is used simply to offload the process of setting the PV values
 * of the detected corners. It takes the current detected bar code, assignes the values of
 * the corners to the struct, and then, if it is the first detected code, sets PV vals
 *
 * @params: discovered -> struct contatining discovered bar or QR code
 * @params: symbol -> current discovered code
 * @params: update_corners -> flag to see if it is the first detected code
 * @return: void
 */
void NDPluginBar::push_corners(bar_QR_code &discovered, Image::SymbolIterator &symbol, int update_corners){
	for(int i = 0; i< symbol->get_location_size(); i++){
		discovered.position.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
		if(update_corners==1){
			if(i==0){
				setIntegerParam(NDPluginBarUpperLeftX, symbol->get_location_x(i));
				setIntegerParam(NDPluginBarUpperLeftY, symbol->get_location_y(i));
			}
			else if(i==1){
				setIntegerParam(NDPluginBarUpperRightX, symbol->get_location_x(i));
				setIntegerParam(NDPluginBarUpperRightY, symbol->get_location_y(i));
			}
			else if(i==2){
				setIntegerParam(NDPluginBarLowerLeftX, symbol->get_location_x(i));
				setIntegerParam(NDPluginBarLowerLeftY, symbol->get_location_y(i));
			}
			else if(i==3){
				setIntegerParam(NDPluginBarLowerRightX, symbol->get_location_x(i));
				setIntegerParam(NDPluginBarLowerRightY, symbol->get_location_y(i));
			}
		}
	}
}


/*
 * Function responsible for calling the appropriate database push back function
 * based on the corresponding PV value. If an invalid type is selected an error
 * message is printed
 *
 * @params: barQR -> detected barcode or QR code
 * @return: status -> -1 for failure, 0 for success
*/
int NDPluginBar::push_to_db(bar_QR_code barQR){
	static const char* functionName = "push_to_db";
	int database_type;
	getIntegerParam(NDPluginBarBarcodeDatabaseType, &database_type);
	switch(database_type){
		case 0:
			//MySQL
			return push_to_sql(barQR);
		case 1:
			//MongoDB
			//TODO
			break;
		default:
			asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s the selected database type does not exist or is not supported in this release\n", driverName, functionName);
			return -1;
	}
}


/*
 * Function responsible for pushing back to a MySQL database.
 * First, if the connection to the database hasn't yet been initialized, we do this now
 * Next, helper functions that are part of the NDBarSQL class are called to insert the
 * information into the database
 *
 * @params: barQR -> detected barcode or QR code
 * @return: status -> integer representing the status of the db write
*/
int NDPluginBar::push_to_sql(bar_QR_code barQR){
	static const char* functionName = "push_to_sql";
	if(database_init==0){
		try{
			string dbName, tableName, server, username, password;
			getStringParam(NDPluginBarBarcodeDatabaseName, dbName);
			getStringParam(NDPluginBarBarcodeTableName, tableName);
			getStringParam(NDPluginBarBarcodeSQLServer, server);
			getStringParam(NDPluginBarBarcodeSQLUser, username);
			getStringParam(NDPluginBarBarcodeSQLPass, password);
			sqlAccessor = new NDBarSQL(dbName, tableName, server, username, password);
			database_init = 1;
		}
		catch(...){
			asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s There was an error initializing the database connection\n" driverName, functionName);
			return -1;
		}
	}
	try{
		sqlAccessor->add_to_table(barQR.data, barQR.type)
	}
	catch(...){
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s There was an error writing to the database\n" driverName, functionName);
		return -1;
	}
	return 0;
}



/*
 * Function that does the barcode decoding. It is passed an image and a vector
 * that will store all of the codes found in the image. A zbar scanner is initialized.
 * The image is changed from an opencv to a Image object, and then it is scanned by zbar.
 * We then iterate over the discovered symbols in the image, and create a instance of the
 * struct. the struct is added to the vector, and the bars location data and type are
 * stored, and printed. Additionally, we populate the associated PVs
 *
 * @params: im -> the opencv image generated by converting the NDArray
 * @params: codes_in_image -> vector that stores all of the detected barcodes
 * @return: void
 */
bool NDPluginBar::decode_bar_code(Mat &im, vector<bar_QR_code> &codes_in_image){

	static const char* functionName = "decode_bar_code";

	int database_enabled;
	getIntegerParam(NDPluginBarEnableBarcodeDatabase, &database_enabled);

	bool found = false;

	//initialize the image and the scanner object
	ImageScanner zbarScanner;
	zbarScanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE,1);
	Image image(im.cols, im.rows, "Y800", (uchar*) im.data, im.cols *im.rows);

	//scan the image with the zbar scanner
	zbarScanner.scan(image);

	//arrays of barcode messages and types
	int messages[5];
	messages[0] = NDPluginBarBarcodeMessage1;
	messages[1] = NDPluginBarBarcodeMessage2;
	messages[2] = NDPluginBarBarcodeMessage3;
	messages[3] = NDPluginBarBarcodeMessage4;
	messages[4] = NDPluginBarBarcodeMessage5;
	int types[5];
	types[0] = NDPluginBarBarcodeType1;
	types[1] = NDPluginBarBarcodeType2;
	types[2] = NDPluginBarBarcodeType3;
	types[3] = NDPluginBarBarcodeType4;
	types[4] = NDPluginBarBarcodeType5;

	//counter for number of codes in current image
	int counter = 0;

	for(Image::SymbolIterator symbol = image.symbol_begin(); symbol!=image.symbol_end();++symbol){

		//get information from detected bar code and populate a bar_QR_code struct
		bar_QR_code barQR;
		barQR.type = symbol->get_type_name();
		barQR.data = symbol->get_data();

		//check to see if code has been detected already
		bool check = check_past_code(barQR.data);
		if(check == true){
			if(previously_detected==0)
				asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Has detected the same barcode or QR code\n",  driverName, functionName);
			previously_detected = 1;
		}
		else{
			previously_detected = 0;
			//print information
			cout << "Type: " << barQR.type << endl;
			cout << "Data: " << barQR.data << endl << endl;

			//set PVs
			setStringParam(types[counter], barQR.type);
			setStringParam(messages[counter], barQR.data);

			//iterate the number of discovered codes
			int num_codes = 0;
			getIntegerParam(NDPluginBarNumberCodes, &num_codes);
			num_codes = num_codes+1;
			setIntegerParam(NDPluginBarNumberCodes, num_codes);

			//only the first code has its coordinates saved
			if(counter == 0){
				//push location data
				push_corners(barQR, symbol, 1);
			}
			else{
				push_corners(barQR, symbol, 0);
			}
			codes_in_image.push_back(barQR);
			if(database_enabled==1) push_to_db(barQR);
			found = true;
		}
		counter++;
	}
	return found;
}



/* Function that uses opencv methods with the locations of the discovered codes to place
 * bounding boxes around the areas of the image that contain barcodes. This is
 * so the user can confirm that the correct area of the image was discovered
 *
 * TODO: Change this function to pipe coordinate information back into NDArray
 *
 * @params: im -> image in which the barcode was discovered
 * @params: codes_in_image -> all barcodes detected in the image
 * @return: void
*/
Mat NDPluginBar::show_bar_codes(Mat &im, vector<bar_QR_code> &codes_in_image){
	for(int i = 0; i<codes_in_image.size(); i++){
		vector<Point> barPoints = codes_in_image[i].position;
		vector<Point> outside;
		if(barPoints.size() > 4) convexHull(barPoints, outside);
		else outside = barPoints;
		int n = outside.size();
		for(int j = 0; j<n; j++){
			line(im, outside[j], outside[(j+1)%n], Scalar(255,255,255),3);
		}
	}
	//imshow("Barcode found", im);
	//waitKey(0);
	return im;
}



/* Process callbacks function inherited from NDPluginDriver.
 * Here it is overridden, and the following steps are taken:
 * 1) Check if the NDArray is mono, as zbar only accepts mono/grayscale images
 * 2) Convert the NDArray recieved into an OpenCV Mat object
 * 3) Decode barcode method is called
 * 4) (Currently Disabled) Show barcode method is called
 *
 * @params: pArray -> NDArray recieved by the plugin from the camera
 * @return: void
*/
void NDPluginBar::processCallbacks(NDArray *pArray){

	//start with an empty array for copy and array info
	NDArray *pScratch = NULL;
	NDArrayInfo arrayInfo;

	//some information we need
	unsigned int numRows, rowSize;
	unsigned char *inData, *outData;

	static const char* functionName = "processCallbacks";

	// check if image is in mono form
	if (pArray->ndims != 2){
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Please convert barcode reader plugin input image format to mono\n",  driverName, functionName);
		return;
	}

	//call base class and get information about frame
	NDPluginDriver::beginProcessCallbacks(pArray);
	pArray->getInfo(&arrayInfo);
	rowSize = pArray->dims[arrayInfo.xDim].size;
	numRows = pArray->dims[arrayInfo.yDim].size;

	//unlock the mutex for the processing portion
	this->unlock();

	//create a copy of the array
	NDDimension_t scratch_dims[2];
	pScratch->initDimension(&scratch_dims[0], rowSize);
	pScratch->initDimension(&scratch_dims[1], numRows);
	this->pNDArrayPool->convert(pArray, &pScratch, NDUInt8);

	pScratch->getInfo(&arrayInfo);
	rowSize = pScratch->dims[arrayInfo.xDim].size;
	numRows = pScratch->dims[arrayInfo.yDim].size;

	//convert the array into an OpenCV "Mat" image
	Mat img = Mat(numRows, rowSize, CV_8UC1);
	Mat barcodeFound;
	vector<bar_QR_code> codes_in_image;

	inData = (unsigned char *)pScratch->pData;
	outData = (unsigned char *)img.data;
	memcpy(outData, inData, arrayInfo.nElements * sizeof(unsigned char));

	//check to see if we need to invert barcode
	int inverted_code;
	getIntegerParam(NDPluginBarInvertedBarcode, &inverted_code);

	Mat found_codes;
	bool found;

	if(inverted_code == 1){
		Mat temp = fix_inverted(img);
		found = decode_bar_code(temp, codes_in_image);
		if(found) found_codes = show_bar_codes(temp, codes_in_image);
	}
	else{
		//decode the bar codes in the image if any
		found = decode_bar_code(img, codes_in_image);
		if(found) found_codes = show_bar_codes(img, codes_in_image);
	}
	if(found){
		NDArray* pDetected = NULL;
		NDDimension_t detected_dims[2];
		pDetected->initDimension(&dcratch_dims[0], rowSize);
		pDetected->initDimension(&dcratch_dims[1], numRows);
		resultDat = (unsigned char*)found_codes.data;
		pDetDat = (unsigned char*)pDetected->data;

		memcpy(resultData, pDetDat, (rowSize*numRows)*sizeof(unsigned char));

		doCallbacksGenericPointer(pDetected, NDPluginBarDetectedBarcodes, 0);
	}

	this->lock();

	//release the array
	if (NULL != pScratch)
		pScratch->release();

	callParamCallbacks();

}

//constructror from base class
NDPluginBar::NDPluginBar(const char *portName, int queueSize, int blockingCallbacks,
		const char *NDArrayPort, int NDArrayAddr,
		int maxBuffers, size_t maxMemory,
		int priority, int stackSize)
		/* Invoke the base class constructor */
		: NDPluginDriver(portName, queueSize, blockingCallbacks,
		NDArrayPort, NDArrayAddr, 1, maxBuffers, maxMemory,
		asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
		asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
		ASYN_MULTIDEVICE, 1, priority, stackSize, 1)
{

	char versionString[25];

	//basic barcode parameters 1-5
	createParam(NDPluginBarBarcodeMessage1String, asynParamOctet, &NDPluginBarBarcodeMessage1);
	createParam(NDPluginBarBarcodeType1String, asynParamOctet, &NDPluginBarBarcodeType1);
	createParam(NDPluginBarBarcodeMessage2String, asynParamOctet, &NDPluginBarBarcodeMessage2);
	createParam(NDPluginBarBarcodeType2String, asynParamOctet, &NDPluginBarBarcodeType2);
	createParam(NDPluginBarBarcodeMessage3String, asynParamOctet, &NDPluginBarBarcodeMessage3);
	createParam(NDPluginBarBarcodeType3String, asynParamOctet, &NDPluginBarBarcodeType3);
	createParam(NDPluginBarBarcodeMessage4String, asynParamOctet, &NDPluginBarBarcodeMessage4);
	createParam(NDPluginBarBarcodeType4String, asynParamOctet, &NDPluginBarBarcodeType4);
	createParam(NDPluginBarBarcodeMessage5String, asynParamOctet, &NDPluginBarBarcodeMessage5);
	createParam(NDPluginBarBarcodeType5String, asynParamOctet, &NDPluginBarBarcodeType5);


	//common params
	createParam(NDPluginBarNumberCodesString, asynParamInt32, &NDPluginBarNumberCodes);
	createParam(NDPluginBarInvertedBarcodeString, asynParamInt32, &NDPluginBarInvertedBarcode);

	//x coordinates
	createParam(NDPluginBarUpperLeftXString, asynParamInt32, &NDPluginBarUpperLeftX);
	createParam(NDPluginBarUpperRightXString, asynParamInt32, &NDPluginBarUpperRightX);
	createParam(NDPluginBarLowerLeftXString, asynParamInt32, &NDPluginBarLowerLeftX);
	createParam(NDPluginBarLowerRightXString, asynParamInt32, &NDPluginBarLowerRightX);

	//y coordinates
	createParam(NDPluginBarUpperLeftYString, asynParamInt32, &NDPluginBarUpperLeftY);
	createParam(NDPluginBarUpperRightYString, asynParamInt32, &NDPluginBarUpperRightY);
	createParam(NDPluginBarLowerLeftYString, asynParamInt32, &NDPluginBarLowerLeftY);
	createParam(NDPluginBarLowerRightYString, asynParamInt32, &NDPluginBarLowerRightY);

	//EXPERIMENTAL: barcodes detected in image
	createParam(NDPluginBarDetectedBarcodesString, asynParamGenericPointer, &NDPluginBarDetectedBarcodes);

	//Database Common
	createParam(NDPluginBarEnableBarcodeDatabaseString, asynParamInt32, &NDPluginBarEnableBarcodeDatabase);
	createParam(NDPluginBarBarcodeDatabaseTypeString, asynParamInt32, &NDPluginBarBarcodeDatabaseType);
	createParam(NDPluginBarBarcodeDatabaseNameString, asynParamOctet, &NDPluginBarBarcodeDatabaseName);
	createParam(NDPluginBarBarcodeTableNameString, asynParamOctet, &NDPluginBarBarcodeTableName);

	//MySQL
	createParam(NDPluginBarBarcodeSQLServerString, asynParamOctet, &NDPluginBarBarcodeSQLServer);
	createParam(NDPluginBarBarcodeSQLUserString, asynParamOctet, &NDPluginBarBarcodeSQLUser);
	createParam(NDPluginBarBarcodeSQLPassStringm asynParamOctet, &NDPluginBarBarcodeSQLPass);


	setStringParam(NDPluginDriverPluginType, "NDPluginBar");
	epicsSnprintf(versionString, sizeof(versionString), "%d.%d.%d", BAR_VERSION, BAR_REVISION, BAR_MODIFICATION);
	setStringParam(NDDriverVersion, versionString);
	connectToArrayPort();
}



extern "C" int NDBarConfigure(const char *portName, int queueSize, int blockingCallbacks,
		const char *NDArrayPort, int NDArrayAddr,
		int maxBuffers, size_t maxMemory,
		int priority, int stackSize){

	NDPluginBar *pPlugin = new NDPluginBar(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
		maxBuffers, maxMemory, priority, stackSize);
	return pPlugin->start();
}

static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "frame queue size",iocshArgInt};
static const iocshArg initArg2 = { "blocking callbacks",iocshArgInt};
static const iocshArg initArg3 = { "NDArrayPort",iocshArgString};
static const iocshArg initArg4 = { "NDArrayAddr",iocshArgInt};
static const iocshArg initArg5 = { "maxBuffers",iocshArgInt};
static const iocshArg initArg6 = { "maxMemory",iocshArgInt};
static const iocshArg initArg7 = { "priority",iocshArgInt};
static const iocshArg initArg8 = { "stackSize",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
					&initArg1,
					&initArg2,
					&initArg3,
					&initArg4,
					&initArg5,
					&initArg6,
					&initArg7,
					&initArg8};

static const iocshFuncDef initFuncDef = {"NDBarConfigure",9,initArgs};

static void initCallFunc(const iocshArgBuf *args){
	NDBarConfigure(args[0].sval, args[1].ival, args[2].ival,
			args[3].sval, args[4].ival, args[5].ival,
			args[6].ival, args[7].ival, args[8].ival);
}


extern "C" void NDBarRegister(void){
	iocshRegister(&initFuncDef,initCallFunc);
}

extern "C" {
	epicsExportRegistrar(NDBarRegister);
}
