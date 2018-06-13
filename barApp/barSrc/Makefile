#TODO: ADD ZBAR to compile

TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

# The following gets rid of the -fno-implicit-templates flag on vxWorks, 
# so we get automatic template instantiation.
# This is what we want for the source files that define template functions.
ifeq (vxWorks,$(findstring vxWorks, $(T_A)))
CODE_CXXFLAGS=
endif

DBD += NDPluginBar.dbd

INC += NDPluginBar.h

LIBRARY_IOC += NDPluginBar

NDPluginBar_SRCS += NDPluginBar.cpp

#TODO: When compiling external opencv+zbar test, I needed to run:
# g++ test.cpp $(pkg-config --libs opencv --cflags) $(pkg-config --libs zbar --cflags) -o check
#Must link Opencv and zbar libraries here

USR_INCLUDES += -I$(OPENCV_INCLUDE)
ifdef OPENCV_LIB
  NDPluginBar_DIR += $(OPENCV_LIB)
endif

USR_INCLUDES += -I$(ZBAR_INCLUDE)
ifdef ZBAR_LIB
  NDPluginBar_DIR += $(ZBAR_LIB)
endif

include $(AREA_DETECTOR)/ADCore/ADApp/commonLibraryMakefile

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
