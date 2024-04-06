#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <spdlog/spdlog.h>  // need link spdlog so library
#include <boost/rational.hpp>  // rational library

#define LITTER_ENDIAN_MARKS 0x49

enum DE_FIELD_TYPE {
    BYTE = 1,   // 8-bit unsigned integer
    ASCII = 2,  // 8-bit byte that contains a 7-bits ASCII code, must terminated with a NUL(binary)
    SHORT = 3,  // 16-bit(2 bytes) unsigned integer
    LONG,       // 32-bit(4 bytes) unsigned integer
    RATIONAL,   // Two LONGs: the first represents the numerator and the second the denominator
    SBYTE,      // 8-bit signed integer
    UNDEFINED,  // 8-bit byte that may contain anything, depending on the definition of the field
    SSHORT,     // 16-bits signed integer
    SLONG,      // 32 signed integer
    SRATIONAL,  // Two SLONGs, the similar to RATIONAL
    FLOAT,      // single precison(4-bytes) IEEE format
    DOUBLE      // double precison(8-bytes) IEEE format
};

// list all types size
static std::map<int, int> FIELD_TYPE2SIZE = {
    {DE_FIELD_TYPE::BYTE, 1},
    {DE_FIELD_TYPE::ASCII, 1},
    {DE_FIELD_TYPE::SHORT, 2},
    {DE_FIELD_TYPE::LONG, 4},
    {DE_FIELD_TYPE::RATIONAL, 4},
    {DE_FIELD_TYPE::SBYTE, 1},
    {DE_FIELD_TYPE::UNDEFINED, 1},
    {DE_FIELD_TYPE::SSHORT, 2},
    {DE_FIELD_TYPE::SLONG, 4},
    {DE_FIELD_TYPE::SRATIONAL, 4},
    {DE_FIELD_TYPE::FLOAT, 4},
    {DE_FIELD_TYPE::DOUBLE, 8}
};

enum DE_TAG {
    IMAGE_WIDTH = 256,  // width of image
    IMAGE_HEIGHT,       // height of image
    BITS_PER_SAMPLE,    // bit pre sample of the image
    COMPRESSION,        // compression type of the image
    PHOTO_METRIC_INTERPRETATION = 262,
    STRIP_OFFSETS = 273,
    SAMPLES_PER_PIXEL = 277,
    ROWS_PRE_STRIP = 278,
    STRIP_BYTE_COUNTS = 279,
    X_RESOLUTION = 282,
    Y_RESOLUTION = 283,
    RESOLUTION_UNIT = 296,
    SOFTWARE = 305,
    DATA_TIME = 306,
    ARTIST = 315,
    DOCUMENT_NAME = 269,
    MAKE = 271,
    MODEL = 272,
};

struct TIFF_Header {
    int16_t mByteOrder;
    int16_t mFlags;
    int32_t mOffsetofIFD;
};

// 12 bytes of DE, mValuesArray pointer to value that DE ownerd, but does not belong to the struct in file structure
struct DirectoryEntry {
    uint16_t mTag;  // The Tag that identifies the field
    uint16_t mFieldType; // the field type
    uint32_t mValueCounts; // the number of values, count the indicated type
    // the value of offset, value offset must be a even number, offset may point anywhere in the file
    uint32_t mOffsets;
    // array store the values
    unsigned char* mValuesArray;

    unsigned char getByte();
    const char* getASCII();
    uint16_t getShort();
    uint32_t getLong();
    boost::rational<uint32_t> getRational();
    char getSByte();
    int16_t getSShort();
    int32_t getSLong();
    boost::rational<int32_t> getSRational();
    float getFloat();
    double getDouble();
};

struct IFD {
    int16_t mNumDE; // number of Directory entries
    std::vector<DirectoryEntry> mDirectoryEnties;
    uint32_t mOffsets; // offset to next IFD
};


// function pre
// parse header
void parseHeader(FILE* handle, TIFF_Header& header);
// from tiff header we need
// offsetofIFD: accroding to parse header, location of directory entry in this file
// byteOrder: we need byte order to translate the value from litter to big endian
void parseDirectoryEntry(FILE* handle, IFD& aIFD, TIFF_Header& header);
// read values from file for each directory entry
void parseValueInDirectoryEntry(FILE* handle, std::vector<DirectoryEntry>& entries);

int main(int argc, char* argv[]) {

    // set log level to debug
    spdlog::set_level(spdlog::level::debug);

    if (argc < 2) {
        spdlog::critical("args number is {}, need give the file path", argc);
        return 1;
    }

    const char* filePath = argv[1];
    // open file and parse it
    FILE* fileHandle = fopen(filePath, "rb");
    if (fileHandle == nullptr) {
        spdlog::critical("file {} can not be opened!", filePath);
        return 2;
    }

    // parse header
    TIFF_Header tiffHeader;
    parseHeader(fileHandle, tiffHeader);

    // parse IFD
    IFD theIFD;
    parseDirectoryEntry(fileHandle, theIFD, tiffHeader);
    parseValueInDirectoryEntry(fileHandle, theIFD.mDirectoryEnties);

    // close file 
    fclose(fileHandle);
    return 0;
}

void parseHeader(FILE* handle, TIFF_Header& header) {
    // seek to start of the file
    fseek(handle, 0, SEEK_SET);
    size_t readBytes = 0;
    bool isBigEndian = false;
    readBytes = fread(&header.mByteOrder, 1, sizeof(TIFF_Header::mByteOrder), handle);
    if (readBytes != sizeof(TIFF_Header::mByteOrder)) {
        spdlog::warn("read header byte order faild, only read {} bytes for ByteOrder", readBytes);
        return;
    }
    // check file use litter endian or big endian
    isBigEndian = ((header.mByteOrder & 0x00FF) == LITTER_ENDIAN_MARKS) ? false : true;
    spdlog::debug("tiff file use {}", isBigEndian ? "big endian" : "litter endian");

    readBytes = fread(&header.mFlags, 1, sizeof(TIFF_Header::mFlags), handle);
    if (readBytes != sizeof(TIFF_Header::mFlags)) {
        spdlog::warn("read header flags failed, only {} bytes read for flags", readBytes);
    }

    readBytes = fread(&header.mOffsetofIFD, 1, sizeof(TIFF_Header::mOffsetofIFD), handle);
    if (readBytes != sizeof(TIFF_Header::mOffsetofIFD)) {
        spdlog::critical("read IFD failed, only {} bytes read for IFD", readBytes);
        return;
    }

    if (isBigEndian) {
        // change litter endian value to big endian value
        // [3] -> [0] | [2] -> [1] | [1] -> [2] | [0] -> [3]
        header.mOffsetofIFD = (header.mOffsetofIFD >> 24) | ((header.mOffsetofIFD >> 8) & 0xFF00) |
            ((header.mOffsetofIFD << 8) & 0xff0000) | (header.mOffsetofIFD << 24);
    }

    spdlog::debug("mOffsetofIFD is {}", header.mOffsetofIFD);
}

void parseDirectoryEntry(FILE* handle, IFD& aIFD, TIFF_Header& header) {
    if (fseek(handle, header.mOffsetofIFD, SEEK_SET)) {
        spdlog::critical("seek to IFD postion failed");
        return;
    }
    // parse directory entry
    int readBytes = 0;
    readBytes = fread(&aIFD.mNumDE, 1, sizeof(IFD::mNumDE), handle);
    if (readBytes != sizeof(IFD::mNumDE)) {
        spdlog::warn("read number of directory entry failed");
        return;
    }
    // pay attention priority of & and !=
    if ((header.mByteOrder & 0xFF) != LITTER_ENDIAN_MARKS) {
        // change litter endian value to big endian value
        // [1] -> [0] | [0] -> [1]
        aIFD.mNumDE = (aIFD.mNumDE >> 8) | ((aIFD.mNumDE << 8) & 0xFF00);
    }
    spdlog::debug("mNumDE is {}", aIFD.mNumDE);

    if (aIFD.mNumDE != 0) {
        // add earch directory entry to IFD
        for (int idx = 0; idx < aIFD.mNumDE; idx++) {
            DirectoryEntry entry;
            readBytes = fread(&entry.mTag, 1, sizeof(DirectoryEntry::mTag), handle);
            if (readBytes != sizeof(DirectoryEntry::mTag)) {
                spdlog::warn("parse mTag for {}'s entry", idx);
                return;
            }
            readBytes = fread(&entry.mFieldType, 1, sizeof(DirectoryEntry::mFieldType), handle);
            if (readBytes != sizeof(DirectoryEntry::mFieldType)) {
                spdlog::warn("parse mFiledType for {}'s failed", idx);
                return;
            }
            readBytes = fread(&entry.mValueCounts, 1, sizeof(DirectoryEntry::mValueCounts), handle);
            if (readBytes != sizeof(DirectoryEntry::mValueCounts)) {
                spdlog::warn("parse mValueCounts for {}'s failed", idx);
                return;
            }
            // maybe need change counts from litter endian to big endian
            if ((header.mByteOrder & 0xFF) != LITTER_ENDIAN_MARKS) {
                entry.mValueCounts = (entry.mValueCounts >> 24) | ((entry.mValueCounts >> 8) & 0xFF00) |
                    ((entry.mValueCounts << 8) & 0xFF0000) | (entry.mValueCounts << 24);
            }
            spdlog::debug("mValueCounts is {} for {}'s entry", entry.mValueCounts, idx);

            // To save time and space the Value Offset contains the Value instead of pointing to
            // the Value if and only if the Value fits into 4 bytes. If the Value is shorter than 4
            // bytes, it is left-justified within the 4-byte Value Offset
            entry.mValuesArray = nullptr;
            entry.mOffsets = 0;
            int valueLength = FIELD_TYPE2SIZE[entry.mFieldType] * entry.mValueCounts;
            if (valueLength <= 4) {
                // we need read value now, do not need to store offsets
                spdlog::debug("read {} bytes from file", valueLength);
                entry.mValuesArray = static_cast<unsigned char*>(malloc(valueLength));
                readBytes = fread(entry.mValuesArray, 1, valueLength, handle);
            } else {
                // store offsets
                readBytes = fread(&entry.mOffsets, 1, sizeof(DirectoryEntry::mOffsets), handle);
                if (readBytes != sizeof(DirectoryEntry::mOffsets)) {
                    spdlog::warn("parse mOffsets for {}'s failed", idx);
                    return;
                }
            }
            // maybe need change offsets from litter eidian to big endian
            if ((header.mByteOrder & 0xFF) != LITTER_ENDIAN_MARKS) {
                entry.mOffsets = (entry.mOffsets >> 24) | ((entry.mOffsets >> 8) & 0xFF00) |
                    ((entry.mOffsets << 8) & 0xFF0000) | (entry.mFieldType << 24);
            }
            spdlog::debug("mOffsets is {} for {}'s entry", entry.mOffsets, idx);
            aIFD.mDirectoryEnties.push_back(entry);
        }
    }

    readBytes = fread(&aIFD.mOffsets, 1, sizeof(IFD::mOffsets), handle);
    if (readBytes != sizeof(IFD::mOffsets)) {
        spdlog::warn("parse ifd's mOffsets error");
        return;
    }

    // maybe need change offsets from litter eidian to big endian
    if ((header.mByteOrder & 0xFF) != LITTER_ENDIAN_MARKS) {
        aIFD.mOffsets = (aIFD.mOffsets >> 24) | ((aIFD.mOffsets >> 8) & 0xFF00) |
                    ((aIFD.mOffsets << 8) & 0xFF0000) | (aIFD.mOffsets << 24);
    }
    spdlog::debug("next ifd position is {}", aIFD.mOffsets);
}

void parseValueInDirectoryEntry(FILE* handle, std::vector<DirectoryEntry>& entries) {
    for (DirectoryEntry& entry : entries) {
        if (fseek(handle, entry.mOffsets, SEEK_SET)) {
            spdlog::critical("seek error: try seek to {}", entry.mOffsets);
            return;
        }
        // for tag value small then DE_TAG::IMAGE_WIDTH , skip it
        if (entry.mTag < DE_TAG::IMAGE_WIDTH) {
            spdlog::warn("for Directory Entry's Tag = {}, less then {}", entry.mTag, DE_TAG::IMAGE_WIDTH);
            continue;
        }
        if (entry.mValuesArray != nullptr) {
            spdlog::debug("value already read, skip it");
            continue;
        }
        switch (entry.mFieldType)
        {
        case DE_FIELD_TYPE::SHORT:
        case DE_FIELD_TYPE::SSHORT:
            entry.mValuesArray = static_cast<unsigned char*>(malloc(entry.mValueCounts * 2));
            fread(entry.mValuesArray, 1, 2 * entry.mValueCounts, handle);
            break;
        case DE_FIELD_TYPE::LONG:
        case DE_FIELD_TYPE::SLONG:
        case DE_FIELD_TYPE::RATIONAL:
        case DE_FIELD_TYPE::SRATIONAL:
        case DE_FIELD_TYPE::FLOAT:
            entry.mValuesArray = static_cast<unsigned char*>(malloc(entry.mValueCounts * 4));
            fread(entry.mValuesArray, 1, 4 * entry.mValueCounts, handle);
            break;
        default:
            break;
        }
    }
}

//======Directory Entry Function======
uint16_t DirectoryEntry::getShort() {
    if (this->mValuesArray == nullptr) {
        spdlog::warn("Directory entry does not containe value");
        return 0;
    }
    uint16_t value = 0;
    // copy 2 bytes from valuesArray
    // Todo: consider about this->mVaulesCount
    memcpy(&value, this->mValuesArray, 2);
    // Todo: consider litter endian and big endian
    return value;
}

uint32_t DirectoryEntry::getLong() {
    if (this->mValuesArray == nullptr) {
        spdlog::warn("Directory entry does not containe value");
        return 0;
    }
    uint32_t value = 0;
    // Todo: consider about this->mVaulesCount
    memcpy(&value, this->mValuesArray, 4);
    // Todo: consider litter endian and big endian
    return value;
}