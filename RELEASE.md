# ADPluginBar Releases
Dependancies: EPICS base, synApps, OpenCV, Zbar
Barcode reader with support for EPICS and area detector integration.
Adding the provided CSS screen to your setup will allow enabling the plugin.
When the plugin is enabled, it checks the image given for a barcode or QR code. If one is found, it decodes it, then calculates the positions of the 4 corners of the code. Further details in README.md
The versions of EPICS base, asyn, and other synApps modules used for each release can be obtained from 
the EXAMPLE_RELEASE_PATHS.local, EXAMPLE_RELEASE_LIBS.local, and EXAMPLE_RELEASE_PRODS.local
files respectively, in the configure/ directory of the appropriate release of the 
[top-level areaDetector](https://github.com/areaDetector/areaDetector) repository.

Release Notes
=============
<!--RELEASE START-->
R2-0 (4-January-2018)
----
* Features Added:
	* Corners can now be displayed for any of the 5 barcodes via a PV toggle
	* Number Codes now lists the number of codes in the image not the total number of codes
	* Passing the ADPluginBar Array Port to an EPICS Viewer allows for live display of detected barcodes and QR codes
	* Support for both 8 bit and 16 bit images
	* Added support for input of RGB images as opposed to only Mono images
	* Barcode Message PVs changed to waveforms to increase max character count to 256
* Bug Fixes/Code Refactoring:
	* Fixed issue where plugin would freeze in certain situations where codes were repeated.
	* Removed redundant functions, split up large functions into more compact pieces
	* Improved documentation for all functions
* Future Plans:
	* Add support for automatic image saving when barcodes detected
	* Support for databases like MySQL and MongoDB

R1-1 (27-June-2018)
----
* Several key features added and bugs fixed:
	* Support added for reading up to 5 codes in one image (Corner data stored in PV for first code)
	* Support added for reading inverted bar codes i.e. white code on black background
	* Fixed issue where number of barcodes would continue to increment when seeing the same bar code
	* Fixed issuse where seeing the same barcode would cause an infinite loop of asyn messages in the IOC shell
* Future plans:
	* Add ability to save image file with detected barcodes marked.
	* Add ability to pipe information into an NDArray and display it in CSS

R1-0 (22-June-2018)
----
* Original release. Some issues that can be resolved with future releases:
    * Only mono images supported
    * stringin record used for barcode message, so limit of 40 characters currently
