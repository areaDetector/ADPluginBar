/*
 * NDPluginBar.cpp
 *
 * Barcode/QRcode reader plugin for EPICS area detector
 * Extends from the base NDPlugin Driver and overrides its processCallbacks function
 * The OpenCV computer vision library and the zbar barcode detection libraries are used
 *
 * Author: Jakub Wlodek
 *
 * Created on: December 3, 2017
 * Last updated: January 4, 2019
 *
*/

//include some standard libraries
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

//include epics/area detector libraries
#include <epicsExport.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <iocsh.h>
#include "NDArray.h"
#include "NDPluginBar.h"

//OpenCV is used for image manipulation, zbar for barcode detection
#include <zbar.h>
#include <opencv2/opencv.hpp>

//some basic namespaces
using namespace std;
using namespace cv;
using namespace zbar;

static const char *driverName = "NDPluginBar";

//------------------------------------------------------
// Functions called at init
//------------------------------------------------------

/* Function that places PV indexes into arrays for easier iteration */
asynStatus NDPluginBar::initPVArrays() {
    barcodeMessagePVs[0] = NDPluginBarBarcodeMessage1;
    barcodeMessagePVs[1] = NDPluginBarBarcodeMessage2;
    barcodeMessagePVs[2] = NDPluginBarBarcodeMessage3;
    barcodeMessagePVs[3] = NDPluginBarBarcodeMessage4;
    barcodeMessagePVs[4] = NDPluginBarBarcodeMessage5;

    barcodeTypePVs[0] = NDPluginBarBarcodeType1;
    barcodeTypePVs[1] = NDPluginBarBarcodeType2;
    barcodeTypePVs[2] = NDPluginBarBarcodeType3;
    barcodeTypePVs[3] = NDPluginBarBarcodeType4;
    barcodeTypePVs[4] = NDPluginBarBarcodeType5;

    cornerXPVs[0] = NDPluginBarUpperLeftX;
    cornerXPVs[1] = NDPluginBarUpperRightX;
    cornerXPVs[2] = NDPluginBarLowerLeftX;
    cornerXPVs[3] = NDPluginBarLowerRightX;

    cornerYPVs[0] = NDPluginBarUpperLeftY;
    cornerYPVs[1] = NDPluginBarUpperRightY;
    cornerYPVs[2] = NDPluginBarLowerLeftY;
    cornerYPVs[3] = NDPluginBarLowerRightY;

    return asynSuccess;
}

//------------------------------------------------------
// Utility functions for printing errors and for clearing previous codes
//------------------------------------------------------

/**
 * Function for printing out OpenCV exception information
 */
void NDPluginBar::printCVError(cv::Exception &e, const char *functionName) {
    cout << "OpenCV Error in function " << functionName << " with code: " << e.code << ", " << e.err;
}

/**
 * Function that clears out the currently detected barcodes.
 */
asynStatus NDPluginBar::clearPreviousCodes() {
    codes_in_image.clear();
    setIntegerParam(NDPluginBarNumberCodes, 0);
    return asynSuccess;
}

//------------------------------------------------------
// Image type conversion functions
//------------------------------------------------------

/**
 * Function that converts an NDArray into a Mat object.
 * Currently supports NDUInt8, NDInt8, NDUInt16, NDInt16 data types, and either mono or RGB 
 * Image types
 * 
 * If the image is in RGB, it is converted to grayscale before it is passed to the plugin, because
 * zbar requires a grayscale image for detection
 * 
 * @params[in]: pArray	-> pointer to an NDArray
 * @params[in]: arrayInfo -> pointer to info about NDArray
 * @params[out]: img	-> smart pointer to output Mat
 * @return: success if able to convert, error otherwise
 */
asynStatus NDPluginBar::ndArray2Mat(NDArray *pArray, NDArrayInfo *arrayInfo, Mat &img) {
    const char *functionName = "ndArray2Mat";
    // data type and num dimensions used during conversion
    NDDataType_t dataType = pArray->dataType;
    int ndims = pArray->ndims;
    // convert based on color depth and data type
    switch (dataType) {
        case NDUInt8:
            if (ndims == 2)
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_8UC1, pArray->pData);
            else
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_8UC3, pArray->pData);
            break;
        case NDInt8:
            if (ndims == 2)
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_8SC1, pArray->pData);
            else
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_8SC3, pArray->pData);
            break;
        case NDUInt16:
            if (ndims == 2)
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_16UC1, pArray->pData);
            else
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_16UC3, pArray->pData);
            break;
        case NDInt16:
            if (ndims == 2)
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_16SC1, pArray->pData);
            else
                img = Mat(arrayInfo->ySize, arrayInfo->xSize, CV_16SC3, pArray->pData);
            break;
        default:
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error: unsupported data format, only 8 and 16 bit images allowed\n", driverName, functionName);
            return asynError;
    }
    try {
        // image must be converted to grayscale before barcode processing
        if (img.channels() != 1) {
            cvtColor(img, img, COLOR_RGB2GRAY);
        }
    } catch (cv::Exception &e) {
        printCVError(e, functionName);
        return asynError;
    }
    return asynSuccess;
}

/**
 * Function that converts Mat back into NDArray. This function is guaranteed to 
 * have either a 8 bit or 16 bit color image, because the bounding boxes drawn
 * around the detected barcodes are blue. The rest of the image will appear
 * black and white, but the actual color mode will be RGB
 * 
 * @params[out]: pScratch -> output NDArray
 * @params[in]: img	-> Mat with barcode bounding boxes drawn
 * @return: success if converted correctly, error otherwise 
 */
asynStatus NDPluginBar::mat2NDArray(NDArray *pScratch, Mat &img) {
    const char *functionName = "mat2NDArray";
    int ndims = 3;
    Size matSize = img.size();
    NDDataType_t dataType;
    NDColorMode_t colorMode = NDColorModeRGB1;
    size_t dims[ndims];
    dims[0] = 3;
    dims[1] = matSize.width;
    dims[2] = matSize.height;

    if (img.depth() == CV_8U)
        dataType = NDUInt8;
    else if (img.depth() == CV_8S)
        dataType = NDInt8;
    else if (img.depth() == CV_16U)
        dataType = NDUInt16;
    else if (img.depth() == CV_16S)
        dataType = NDInt16;

    pScratch = pNDArrayPool->alloc(ndims, dims, dataType, 0, NULL);
    if (pScratch == NULL) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error, unable to allocate array\n", driverName, functionName);
        img.release();
        return asynError;
    }

    NDArrayInfo arrayInfo;
    pScratch->getInfo(&arrayInfo);

    size_t dataSize = img.step[0] * img.rows;

    if (dataSize != arrayInfo.totalBytes) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error, invalid array size\n", driverName, functionName);
        img.release();
        return asynError;
    }

    memcpy((unsigned char *)pScratch->pData, (unsigned char *)img.data, dataSize);
    pScratch->pAttributeList->add("ColorMode", "Color Mode", NDAttrInt32, &colorMode);
    pScratch->pAttributeList->add("DataType", "Data Type", NDAttrInt32, &dataType);
    getAttributes(pScratch->pAttributeList);
    doCallbacksGenericPointer(pScratch, NDArrayData, 0);
    //release the array
    pScratch->release();
    // now that data is copied to NDArray, we don't need mat anymore
    img.release();
    return asynSuccess;
}

/**
 * Function used to use a form of thresholding to reverse the coloration of a bar code
 * or QR code that is in the white on black format rather than the standard black on white
 *
 * @params[out]: img -> image containing inverse QR code. Required to be 8 bit
 * @return: status -> if image is 8 bit, then invert it and success, otherwise, error
 */
asynStatus NDPluginBar::fix_inverted(Mat &img) {
    if (img.depth() != CV_8U || img.depth() != CV_8S) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error, only 8 bit images support inversion\n", driverName, "fix_inverted");
        return asynError;
    }
    subtract(Scalar(255), img, img);
    return asynSuccess;
}

/**
 * Function that is used simply to offload the process of setting the PV values
 * of the detected corners. It takes the current detected bar code, assignes the values of
 * the corners to the struct, and then, if it is the first detected code, sets PV vals
 *
 * @params[out]:  discovered 	-> struct contatining discovered bar or QR code
 * @params[in]:  symbol 		-> current discovered code
 * @params[in]:  update_corners -> flag to see if it is the first detected code
 * @return: status
 */
asynStatus NDPluginBar::push_corners(bar_QR_code &discovered, Image::SymbolIterator &symbol, int update_corners) {
    for (int i = 0; i < symbol->get_location_size(); i++) {
        discovered.position.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
    }
    if (update_corners == 1) {
        updateCorners(discovered);
    }
    return asynSuccess;
}

/**
 * Function that updates corner coordinate PVs to those of
 * a specific detected bar code
 * 
 * @params[in]: discovered	-> code from which we want corner info
 * @return: status
 */
asynStatus NDPluginBar::updateCorners(bar_QR_code &discovered) {
    //const char* functionName = "updateCorners";
    int i;
    for (i = 0; i < 4; i++) {
        if (i == 0) {
            setIntegerParam(NDPluginBarUpperLeftX, discovered.position[i].x);
            setIntegerParam(NDPluginBarUpperLeftY, discovered.position[i].y);
        } else if (i == 1) {
            setIntegerParam(NDPluginBarUpperRightX, discovered.position[i].x);
            setIntegerParam(NDPluginBarUpperRightY, discovered.position[i].y);
        } else if (i == 2) {
            setIntegerParam(NDPluginBarLowerLeftX, discovered.position[i].x);
            setIntegerParam(NDPluginBarLowerLeftY, discovered.position[i].y);
        } else if (i == 3) {
            setIntegerParam(NDPluginBarLowerRightX, discovered.position[i].x);
            setIntegerParam(NDPluginBarLowerRightY, discovered.position[i].y);
        }
    }
    return asynSuccess;
}

/**
 * Function that uses zbar to scan image for barcodes
 * 
 * @params[in]: img -> input image in Mat format
 * @return: Image -> new image object with scanned symbols
 */
Image NDPluginBar::scan_image(Mat &img) {
    //initialize the image and the scanner object
    ImageScanner zbarScanner;
    zbarScanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    Image scannedImage(img.cols, img.rows, "Y800", (uchar *)img.data, img.cols * img.rows);

    //scan the image with the zbar scanner
    zbarScanner.scan(scannedImage);

    return scannedImage;
}

/**
 * Function that clears any non-overwritten barcode PVs between array callbacks
 * 
 * @params[in]: counter -> number of codes detected in the new image
 * @return: success if set correctly otherwise error
 */
asynStatus NDPluginBar::clear_unused_barcode_pvs(int counter) {
    //const char* functionName = "clear_unused_barcode_pvs";
    int i;
    for (i = counter; i < NUM_CODES; i++) {
        asynStatus s1 = setStringParam(barcodeMessagePVs[i], "No Barcode Found");
        asynStatus s2 = setStringParam(barcodeTypePVs[i], "None");
        if (s1 == asynError || s2 == asynError) return asynError;
    }
    return asynSuccess;
}

/**
 * Function that does the barcode decoding. It is passed an image and a vector
 * that will store all of the codes found in the image. A zbar scanner is initialized.
 * The image is changed from an opencv to a Image object, and then it is scanned by zbar.
 * We then iterate over the discovered symbols in the image, and create a instance of the
 * struct. the struct is added to the vector, and the bars location data and type are
 * stored, and printed. Additionally, we populate the associated PVs
 *
 * @params[in]: im -> the opencv image generated by converting the NDArray
 * @return: void
 */
asynStatus NDPluginBar::decode_bar_codes(Mat &img) {
    // static const char* functionName = "decode_bar_codes";

    // first scan the image for barcodes
    Image scannedImage = scan_image(img);

    //counter for number of codes in current image
    int counter = 0;

    // remove any previously detected barcodes
    clearPreviousCodes();

    for (Image::SymbolIterator symbol = scannedImage.symbol_begin(); symbol != scannedImage.symbol_end(); ++symbol) {
        //get information from detected bar code and populate a bar_QR_code struct
        bar_QR_code barQR;
        barQR.type = symbol->get_type_name();
        barQR.data = symbol->get_data();

        //set PVs
        setStringParam(barcodeTypePVs[counter], barQR.type);
        setStringParam(barcodeMessagePVs[counter], barQR.data);
        //iterate the number of discovered codes
        int num_codes = 0;
        getIntegerParam(NDPluginBarNumberCodes, &num_codes);
        num_codes = num_codes + 1;
        setIntegerParam(NDPluginBarNumberCodes, num_codes);

        int code_corners;
        getIntegerParam(NDPluginBarCodeCorners, &code_corners);
        //only the first code has its coordinates saved
        if (counter == code_corners) {
            //push location data
            push_corners(barQR, symbol, 1);
        } else {
            push_corners(barQR, symbol, 0);
        }
        codes_in_image.push_back(barQR);

        counter++;
    }

    // clear any old barcode PVs that were not overwritten
    clear_unused_barcode_pvs(counter);

    return asynSuccess;
}

/* Function that uses opencv methods with the locations of the discovered codes to place
 * bounding boxes around the areas of the image that contain barcodes. This is
 * so the user can confirm that the correct area of the image was discovered
 *
 * TODO: Change this function to pipe coordinate information back into NDArray
 *
 * @params[out]: img -> image in which the barcode was discovered
 * @params[in]: codes_in_image -> all barcodes detected in the image
 * @return: status
*/
asynStatus NDPluginBar::show_bar_codes(Mat &img) {
    const char *functionName = "show_bar_codes";
    try {
        cvtColor(img, img, COLOR_GRAY2RGB);
        for (unsigned int i = 0; i < codes_in_image.size(); i++) {
            vector<Point> barPoints = codes_in_image[i].position;
            vector<Point> outside;
            if (barPoints.size() > 4)
                convexHull(barPoints, outside);
            else
                outside = barPoints;
            int n = outside.size();
            for (int j = 0; j < n; j++) {
                line(img, outside[j], outside[(j + 1) % n], Scalar(0, 0, 255), 3);
            }
        }
        return asynSuccess;
    } catch (cv::Exception &e) {
        printCVError(e, functionName);
        return asynError;
    }
}

static void barcode_image_callback_wrapper(void* pPtr, NDArray* pArray){
    NDPluginBar* pPlugin = (NDPluginBar*) pPtr;
    pPlugin->barcode_image_callback(pArray);
}

asynStatus NDPluginBar::barcode_image_callback(NDArray* pArray){
    //start with an empty array for copy and array info
    NDArray *pScratch = NULL;
    Mat img;
    const char* functionName = "barcode_image_callback";
    NDArrayInfo arrayInfo;
    // convert to Mat
    pArray->getInfo(&arrayInfo);
    asynStatus status = ndArray2Mat(pArray, &arrayInfo, img);
    if (status == asynError) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error converting to Mat\n", driverName, functionName);
        this->processing = false;
        return status;
    }
    //check to see if we need to invert barcode
    int inverted_code;
    getIntegerParam(NDPluginBarInvertedBarcode, &inverted_code);

    if (inverted_code == 1) {
        status = fix_inverted(img);
        if (status != asynError) status = decode_bar_codes(img);
    }
    else {
        status = decode_bar_codes(img);
    }
    if (status != asynError) status = show_bar_codes(img);
    status = mat2NDArray(pScratch, img);
    if (status == asynError) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error, image not processed correctly\n", driverName, functionName);
    }
    this->processing = false;
    return status;
}

/**
 * Override of NDPluginDriver function. Used when selecting between barcodes
 * for which corners should be shown.
 * 
 * @params[in]: pasynUser	-> pointer to asyn User that initiated the transaction
 * @params[in]: value		-> value PV was set to
 * @return: success if PV was updated correctly, otherwise error
 */
asynStatus NDPluginBar::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    const char *functionName = "writeInt32";
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    status = setIntegerParam(function, value);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s::%s function = %d value=%d\n", driverName, functionName, function, value);

    if (function == NDPluginBarCodeCorners) {
        int i;
        if (codes_in_image.size() <= (size_t) value) {
            for (i = 0; i < 4; i++) {
                setIntegerParam(cornerXPVs[i], 0);
                setIntegerParam(cornerYPVs[i], 0);
            }
        } else {
            for (i = 0; i < 4; i++) {
                setIntegerParam(cornerXPVs[i], codes_in_image[value].position[i].x);
                setIntegerParam(cornerYPVs[i], codes_in_image[value].position[i].y);
            }
        }
    } else if (function < ND_BAR_FIRST_PARAM) {
        status = NDPluginDriver::writeInt32(pasynUser, value);
    }
    callParamCallbacks();
    if (status) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s Error writing Int32 val to PV\n", driverName, functionName);
    }
    return status;
}

/* Process callbacks function inherited from NDPluginDriver.
 * Here it is overridden, and the following steps are taken:
 * 1) Check if the NDArray is mono, as zbar only accepts mono/grayscale images
 * 2) Convert the NDArray recieved into an OpenCV Mat object
 * 3) Decode barcode method is called
 * 4) (Currently Disabled) Show barcode method is called
 *
 * @params[in]: pArray -> NDArray recieved by the plugin from the camera
 * @return: void
*/
void NDPluginBar::processCallbacks(NDArray *pArray) {
    static const char *functionName = "processCallbacks";

    //call base class and get information about frame
    NDPluginDriver::beginProcessCallbacks(pArray);

    //unlock the mutex for the processing portion
    this->unlock();

    if(!this->processing){
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s Starting processing thread\n", driverName, functionName);
        this->processing = true;
        thread processing_thread(barcode_image_callback_wrapper, this, pArray);
        processing_thread.detach();
    }

    this->lock();

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
                     ASYN_MULTIDEVICE, 1, priority, stackSize, 1) {
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
    createParam(NDPluginBarCodeCornersString, asynParamInt32, &NDPluginBarCodeCorners);
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

    initPVArrays();

    setStringParam(NDPluginDriverPluginType, "NDPluginBar");
    epicsSnprintf(versionString, sizeof(versionString), "%d.%d.%d", BAR_VERSION, BAR_REVISION, BAR_MODIFICATION);
    setStringParam(NDDriverVersion, versionString);
    connectToArrayPort();
}

/**
 * External configure function. This will be called from the IOC shell of the
 * detector the plugin is attached to, and will create an instance of the plugin and start it
 * 
 * @params[in]	-> all passed to constructor
 */
extern "C" int NDBarConfigure(const char *portName, int queueSize, int blockingCallbacks,
                              const char *NDArrayPort, int NDArrayAddr,
                              int maxBuffers, size_t maxMemory,
                              int priority, int stackSize) {
    NDPluginBar *pPlugin = new NDPluginBar(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
                                           maxBuffers, maxMemory, priority, stackSize);
    return pPlugin->start();
}

/* IOC shell arguments passed to the plugin configure function */
static const iocshArg initArg0 = {"portName", iocshArgString};
static const iocshArg initArg1 = {"frame queue size", iocshArgInt};
static const iocshArg initArg2 = {"blocking callbacks", iocshArgInt};
static const iocshArg initArg3 = {"NDArrayPort", iocshArgString};
static const iocshArg initArg4 = {"NDArrayAddr", iocshArgInt};
static const iocshArg initArg5 = {"maxBuffers", iocshArgInt};
static const iocshArg initArg6 = {"maxMemory", iocshArgInt};
static const iocshArg initArg7 = {"priority", iocshArgInt};
static const iocshArg initArg8 = {"stackSize", iocshArgInt};
static const iocshArg *const initArgs[] = {&initArg0,
                                           &initArg1,
                                           &initArg2,
                                           &initArg3,
                                           &initArg4,
                                           &initArg5,
                                           &initArg6,
                                           &initArg7,
                                           &initArg8};

/* Definition of the configure function for NDPluginBar in the IOC shell */
static const iocshFuncDef initFuncDef = {"NDBarConfigure", 9, initArgs};

/* link the configure function with the passed args, and call it from the IOC shell */
static void initCallFunc(const iocshArgBuf *args) {
    NDBarConfigure(args[0].sval, args[1].ival, args[2].ival,
                   args[3].sval, args[4].ival, args[5].ival,
                   args[6].ival, args[7].ival, args[8].ival);
}

/* function to register the configure function in the IOC shell */
extern "C" void NDBarRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

/* Exports plugin registration */
extern "C" {
    epicsExportRegistrar(NDBarRegister);
}
