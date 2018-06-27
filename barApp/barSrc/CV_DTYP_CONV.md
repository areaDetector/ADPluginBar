# Conversions between NDArray Data types and OpenCV images

### Basic Types of NDArrays:

ND type     |   Data type
---         |   ---
NDInt8      |   Signed 8-bit integer
NDUInt8     |   Unsigned 8-bit integer
NDInt16     |   Signed 16-bit integer
NDUInt16    |   Unsigned 16-bit integer
NDInt32     |   Signed 32-bit integer
NDUInt32    |   Unsigned 32-bit integer
NDFloat32   |   32-bit float
NDFloat64   |   64-bit float

### Color Modes of NDArrays:

ND image type       |       Image type
---                 |       ---
NDColorModeMono     |    Monochromatic image
NDColorModeBayer    |    Bayer pattern image, 1 value per pixel but with color Filter on detector
NDColorModeRGB1     |    RGB image with pixel color interleave, data array is [3, NX, NY]
NDColorModeRGB2     |    RGB image with row color interleave, data array is [NX, 3, NY]
NDColorModeRGB3     |    RGB image with plane color interleave, data array is [NX, NY, 3]
NDColorModeYUV444   |    YUV image, 3 bytes encodes 1 RGB pixel
NDColorModeYUV422   |    YUV image, 4 bytes encodes 2 RGB pixel
NDColorModeYUV411   |    YUV image, 6 bytes encodes 4 RGB pixels

### Conversions between NDArrays and OpenCV Mats

NDArray         |         OpenCV
---             |           ---
NDInt8          |          CV_8S
NDUInt8         |         CV_8U
NDInt16         |         CV_16S
NDUInt16        |        CV_16U
NDInt32         |         CV_32S
NDUInt32        |        CV_32U
NDFloat32       |       CV_32F
NDFloat64       |       CV_64F

### Conversions between NDArray and OpenCV bayer patterns

ND_BayerPatern      |  OpenCV
-----               |   -----
NDBayer_RGGB        |    RG
NDBayer_GBRG        |   GB
NDBayer_GRGB        |   GR
NDBayer_BGGR        |   BG
