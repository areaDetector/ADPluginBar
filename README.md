# ADPluginBar

A Barcode reading plugin for EPICS Area Detector using OpenCV and Zbar libraries
for barcode and QR code reading.

This plugin is still in development.

### Installation and dependancies

The ADPluginBar plugin works with the EPICS control system and thus requires a supported version of
EPICS base as well as synApps. synApps will include area detector and asyn, both required for ADPluginBar.

Two external libraries are required as well: OpenCV, which is used for image manipulation, and zbar
which is an open source barcode detection library. Both can be built from source by cloning the
respective repositories from github, or if on a debin/ubuntu system, slightly older versions can
be downloaded using the package manager with the following commands:

```
sudo apt-get install libcv-dev libcv2.4 libcvaux-dev libcvaux2.4 libhighgui-dev 
sudo apt-get install libhighgui2.4 libopencv-contrib-dev libopencv-contrib2.4
sudo apt-get install libzbar-dev libzbar0 libzbarqt-dev libzbarqt0 libzbargtk-dev libzbargtk0
```

If possible, building from source is preferable, as the debian/ubuntu packages are outdated.
For example, the debian package for opencv is version 2.4, while the newest version at the time
of writing is 3.3. Thus, while the plugin will function with the default packages, it is not 
recommended.

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
    PROD_LIBS       += opencv_core opencv_imgproc opencv_highgui zbar
  else
    PROD_SYS_LIBS   += opencv_core opencv_imgproc opencv_highgui zbar
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

In the same directory, check the commonPlugin_settings.req file to make sure the following line is uncommented:

```
file "NDBar_settings.req",         P=$(P),  R=Bar1:
```

Next, go back into your area detector base directory, and enter the configure directory.
Here, in the CONFIG_SITE.local.$(YOUR HOST) file, ensure that the following is defined:

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

Once you have done all of this, compile ADCore, and then run

```
make -sj
```

in the ADPluginBar directory to compile it.

You have now installed the ADPluginBar Plugin.

### Usage

To use ADPluginBar with CSS, place the provided .opi screen into your CSS setup, and link to it
appropriately. It requires a mono image, so set your camera to mono, and enable the plugin.
When a barcode is detected, the plugin will populate the appropriate PVs, and display an image with 
a bounding box representing where in the image it identified a barcode.
