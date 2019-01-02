# ADPluginBar

A Barcode reading plugin for EPICS Area Detector using OpenCV and Zbar libraries
for barcode and QR code reading.

Primary Author:    	Jakub Wlodek, 
Corresponding Author: 	Kazimierz Gofron

### Installation and dependancies

The ADPluginBar plugin works with the EPICS control system and thus requires a supported version of
EPICS base as well as synApps. synApps will include area detector and asyn, both required for ADPluginBar.

Two external libraries are required as well: OpenCV, which is used for image manipulation, and zbar
which is an open source barcode detection library. Both can be built from source by cloning the
respective repositories from github, or if on a debin/ubuntu system, slightly older versions can
be downloaded using the package manager with the following commands:

```
sudo apt install libopencv-dev 
sudo apt install libopencv-contrib-dev
sudo apt install libzbar-dev
```

In addition, the numbered versions of these packages may be needed. For example, on the debian 8 machine
this was tested on, libcv2.4 was used.

It is also possible to build both opencv and zbar from source, links to each github are placed in
the github.io website linked at the top of this repository.

Once EPICS base, synApps, OpenCV, and zbar are all installed, some changes need to be made to the
configuration files within area detector. First, enter into your areaDetector directory, and
clone the ADPluginBar repository. It should be on the same level as ADCore.

Next, enter the ADCore directory, enter ADApp, and open the commonDriverMakefile file,
 and ensure that the following is added and uncommented after ADPluginEdge:

```
ifdef ADPLUGINBAR
  $(DBD_NAME)_DBD += NDPluginBar.dbd
  PROD_LIBS	  += NDPluginBar
  ifdef OPENCV_LIB
    opencv_core_DIR +=$(OPENCV_LIB)
    PROD_LIBS       += opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs zbar
  else
    PROD_SYS_LIBS   += opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs zbar
  endif
endif
```

This will link the necessary OpenCV and zbar libraries when making area detector.

Return to the ADCore directory, and now enter the iocBoot directory. Open commonPlugins.cmd,
and ensure the following is uncommented:

```
# Optional: load NDPluginBar plugin
NDBarConfigure("BAR1", $(QSIZE), 0, "$(PORT)", 0, 0, 0, 0)
dbLoadRecords("$(ADPLUGINBAR)/db/NDBar.template",  "P=$(PREFIX),R=Bar1:, PORT=BAR1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")
set_requestfile_path("$(ADPLUGINBAR)/barApp/Db")
```

This will add ADPluginBar to the boot operation when the ioc is run.  

Optionally:
In the same directory, check the commonPlugin_settings.req file to make sure the following line is uncommented:

```
file "NDBar_settings.req",         P=$(P),  R=Bar1:
```
This has to do with the EPICS autosave feature, and currently is buggy. This is not required for Plugin operation and can
be omitted.

Next in the configure directory at the top level of areaDetector, open the RELEASE_PRODS.local file, and add the following:

```
# Load the ADPluginBar plugin
ADPLUGINBAR=$(AREA_DETECTOR)/ADPluginBar
```

This will tell areaDetector to include ADPluginBar at compilation.   

Then, in the CONFIG_SITE.local.$(YOUR HOST) file, ensure that the following is defined:  

```
# OPENCV_LIB and OPENCV_INCLUDE variables should not be defined if using the opencv system library in a default location
WITH_OPENCV     = YES 
OPENCV          = /usr
#OPENCV_LIB     = $(OPENCV)/lib64
#OPENCV_INCLUDE = -I$(OPENCV)/include

# ZBAR_LIB and ZBAR_INCLUDE variables should not be defined if using the zbar system library in a default location
WITH_ZBAR       = YES 
ZBAR            = /usr
#ZBAR_LIB       = $(ZBAR)/lib64
#ZBAR_INCLUDE   = -I$(ZBAR)/include
```

Simply replace the variable to give your path to OpenCV and zbar includes. Make sure that WITH_OPENCV and WITH_ZBAR are
set to "YES"

Next, in CONFIG_SITE.local, in the same directory, make sure that WITH_OPENCV, OPENCV_EXTERNAL,
WITH_ZBAR, and ZBAR_EXTERNAL are all set to "YES"

Once you have done all of this, compile ADSupport, then ADCore, and then run

```
make -sj
```

in the ADPluginBar directory to compile it.

You have now installed the ADPluginBar Plugin.

### Usage

To use ADPluginBar with CSS, place the provided .opi screens into your CSS setup, and link to it
appropriately. It requires a mono image, so set your camera to mono, and enable the plugin.
When a barcode is detected, the plugin will populate the appropriate PVs, and display an image with 
a bounding box representing where in the image it identified a barcode.

### Process Variables Supported

PV		|  Comment
----------------|---------------
BarcodeMessage(1-5)  |  The message contained within the decoded barcode
BarcodeType(1-5)     |  The type of the decoded barcode i.e. CODE-128, QR-Code etc.
NumberCodes     |  Live count of the number of decoded bar codes
UpperLeftX	|  X-coordinate of the upper left corner of the detected barcode
UpperRightX	|  X-coordinate of the upper right corner of the detected barcode
LowerLeftX	|  X-coordinate of the lower left corner of the detected barcode
LowerRightX	|  X-coordinate of the lower right corner of the detected barcode 
UpperLeftY	|  Y-coordinate of the upper left corner of the detected barcode
UpperRightY	|  Y-coordinate of the upper right corner of the detected barcode
LowerLeftY	|  Y-coordinate of the lower left corner of the detected barcode
LowerRightY	|  Y-coordinate of the lower right corner of the detected barcode


Note: Only the corners of the first detected barcode are stored in PVs. However,
if the image is reset, and different barcodes are found, these will be replaced.
