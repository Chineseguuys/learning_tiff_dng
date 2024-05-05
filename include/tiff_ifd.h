#ifndef _TIFF_IFD_H_
#define _TIFF_IFD_H_

#include <boost/rational.hpp>
#include <map>
#include <stdint.h>
#include <vector>

#define LITTER_ENDIAN_MARKS 0x49

// It is possible that other TIFF field types will be added in the future.
// Readers should skip over fields containing an unexpected field type
enum DE_FIELD_TYPE {
    BYTE = 1,   // 8-bit unsigned integer
    ASCII = 2,  // 8-bit byte that contains a 7-bits ASCII code, must terminated with a NUL(binary)
    SHORT = 3,  // 16-bit(2 bytes) unsigned integer
    LONG = 4,       // 32-bit(4 bytes) unsigned integer
    RATIONAL = 5,   // Two LONGs: the first represents the numerator and the second the denominator
    SBYTE = 6,      // 8-bit signed integer
    UNDEFINED = 7,  // 8-bit byte that may contain anything, depending on the definition of the field
    SSHORT = 8,     // 16-bits signed integer
    SLONG = 9,      // 32 signed integer
    SRATIONAL = 10,  // Two SLONGs, the similar to RATIONAL
    FLOAT = 11,      // single precison(4-bytes) IEEE format
    DOUBLE = 12      // double precison(8-bytes) IEEE format
};

// list all types size
static std::map<int, int> FIELD_TYPE2SIZE = {
    {DE_FIELD_TYPE::BYTE, 1},
    {DE_FIELD_TYPE::ASCII, 1},
    {DE_FIELD_TYPE::SHORT, 2},
    {DE_FIELD_TYPE::LONG, 4},
    {DE_FIELD_TYPE::RATIONAL, 8},   // two LONGs
    {DE_FIELD_TYPE::SBYTE, 1},
    {DE_FIELD_TYPE::UNDEFINED, 1},
    {DE_FIELD_TYPE::SSHORT, 2},
    {DE_FIELD_TYPE::SLONG, 4},
    {DE_FIELD_TYPE::SRATIONAL, 8},  // two SLONGs
    {DE_FIELD_TYPE::FLOAT, 4},
    {DE_FIELD_TYPE::DOUBLE, 8}
};

enum DE_TAG {
    IMAGE_WIDTH = 256,  // width of image or ImageLen
    IMAGE_HEIGHT = 257,       // height of image
    BITS_PER_SAMPLE = 258,    // bit pre sample of the image
    COMPRESSION = 259,        // compression type of the image
    PHOTO_METRIC_INTERPRETATION = 262,
    MAKE = 271,     // name of producter
    MODEL = 272,    // Specific model or version number of equipment or software
    STRIP_OFFSETS = 273,        // For each strip, the byte offset of that strip
    ORIENTATION = 274,
    SAMPLES_PER_PIXEL = 277,    // such like grayscale image, full rgb image use this
    ROWS_PRE_STRIP = 278,       // The number of rows in each strip
    STRIP_BYTE_COUNTS = 279,    // For each strip, the number of bytes in that strip after any compression.
    X_RESOLUTION = 282,
    Y_RESOLUTION = 283,
    PLANAR_CONFIGURATION = 284,  // pixel 排列方式
    // TIFF also supports breaking an image into separate strips for increased editing flexibility and efficient I/O buffering
    RESOLUTION_UNIT = 296,
    SOFTWARE = 305,
    DATA_TIME = 306,
    ARTIST = 315,
    DOCUMENT_NAME = 269,
    COLOR_MAP = 320,    // for palette-color images, palette-color images has a RGB-lookup table(Color map)
    TILE_WIDTH = 322, // The tile width in pixels. This is the number of columns in each tile.
    TILE_HEIGHT = 323, // The tile length (height) in pixels. This is the number of rows in each tile.
    TILE_OFFSET = 324, // For each tile, the byte offset of that tile, as compressed and stored on disk.
    TILE_BYTE_COUNTS = 325, // For each tile, the number of (compressed) bytes in that tile.
    // Each value is an offset (from the beginning of the TIFF file, as always) to a child IFD.
    // Child images provide extra information for the parent image - such as a subsampled version of the parent image.
    SUB_IFDS = 330,
    YCbCr_COEFFICIENTS = 529,
    EXIF_OFFSET = 34665, // offset of the ExifIFD for current image
    DNG_VERSION = 50706, // dng version
    DNG_BACKWARD_VERSION = 50707, // dng backward version
    UNIQUE_CAMERA_MODEL = 50708, // unique camera model, ascii string
    COLOR_MATRIX_1 = 50721, // use the color matrix convert sensor data to standard color space(sRGB or AdobeRGB)
    // Provides additional colour correction or adjustment.Such as "Standard Mode", "Landscape Mode", "Portrait Mode", etc.
    COLOR_MATRIX_2 = 50722,
    CAMERA_CALIBRATION_1 = 50723, // For storing the camera's calibration data
    CAMERA_CALIBRATION_2 = 50724, // For storing the camera's calibration data
    ANALOG_BALANCE = 50727, // Specify the camera's analogue white balance factor， three rational array(R-G-B)
    AS_SHOT_NEUTRAL = 50728, // representing the neutral colour state of the camera's sensor at the time of capture
    BASELINE_EXPOSURE = 50730, // Base exposure level when the camera takes an image
    BASELINE_NOISE = 50731, // Baseline noise level of images captured by the camera
    BASELINE_SHARPNESS = 50732, // Specifies the baseline sharpness level for images captured by the camera
    LINEAR_RESPONSE_LIMIT = 50734, // Specify the linear response limit of the camera sensor
    PROWARD_MATRIX_1 = 50964, // This tag defines a matrix that maps white balanced camera colors to XYZ D50 colors.
    FORWARD_MATRIX_2 = 50965, // This tag defines a matrix that maps white balanced camera colors to XYZ D50 colors.
    PROFILE_LOOK_TABLE_DATA = 50982,
    // NoiseProfile describes the amount of noise in a raw image. Specifically, this tag models the amount of signal-dependent
    // photon (shot) noise and signal-independent sensor readout noise, two common sources of noise in raw images.
    NOISE_PROFILE = 51041,
    // This optional tag in a color profile provides a hint to the raw converter regarding how to handle the black point during rendering
    DEFAULT_BLACK_RENDER = 51110, // 减黑操作
};

struct TIFF_Header {
    int16_t mByteOrder;
    int16_t mFlags;
    int32_t mOffsetofIFD;   // offset of the first ifd

    // constructor
    TIFF_Header();
};

// 12 bytes of DE, mValuesArray pointer to value that DE ownerd, but does not belong to the struct in file structure
struct DirectoryEntry {
    uint16_t mTag;  // The Tag that identifies the field
    uint16_t mFieldType; // the field type
    uint32_t mValueCounts; // the number of values, count the indicated type
    // the value of offset, value offset must be a even number, offset may point anywhere in the file
    uint32_t mOffsets;
    uint32_t mValuesArrayOffsets; // for mValueCounts > 1, we can get value for multi-times
    // array store the values
    unsigned char* mValuesArray;

    // constructor
    DirectoryEntry();
    int16_t getShort();
    int32_t getLong();

    // for debug
    void printEntry();
};

// Todo: There may be more than one IFD in a TIFF file. Each IFD defines a subfile.
struct IFD {
    int16_t mNumDE; // number of Directory entries
    uint32_t mOffsets; // offset to next IFD
    // map DE_TAG -> DirectoryEntry
    std::map<uint16_t, DirectoryEntry> mTag2DirectoryEntry;
    std::vector<IFD> mSubIFDs;  // sub ifd's

    // constructor
    IFD();
    bool containDirectoryEntry(DE_TAG tag);
    DirectoryEntry& getDirectoryEntry(DE_TAG tag);
};

// store all IFD in tiff file
struct TIFF_Contexet {
    // tiff header, need prase the ifd from there
    TIFF_Header mTiffHeader;
    // store all ifd in the file
    std::vector<IFD> mIFDs;
};
#endif /* _TIFF_IFD_H_ */