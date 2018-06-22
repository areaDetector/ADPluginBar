ADPluginBar Releases
=====================

Dependancies: EPICS base, synApps, OpenCV, Zbar

Barcode reader with support for EPICS and area detector integration.
Adding the provided CSS screen to your setup will allow enabling the plugin.

Currently, only mono images are supported. When the plugin is enabled, it checks
the image given for a barcode or QR code. If one is found, it decodes it, then
calculates the positions of the 4 corners of the code. Finally, it increments a counter
that stores the number of bar codes detected. Further details in README.md


The versions of EPICS base, asyn, and other synApps modules used for each release can be obtained from 
the EXAMPLE_RELEASE_PATHS.local, EXAMPLE_RELEASE_LIBS.local, and EXAMPLE_RELEASE_PRODS.local
files respectively, in the configure/ directory of the appropriate release of the 
[top-level areaDetector](https://github.com/areaDetector/areaDetector) repository.

Release Notes
=============

R1-0 (22-June-2018)
----
* Original release.
