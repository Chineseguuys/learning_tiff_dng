#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <spdlog/spdlog.h>  // need link spdlog so library
#include <boost/rational.hpp>  // rational library

#include <iostream>
#include <string>

#include "tiff_ifd.h"

// function pre
// parse header
void parseHeader(FILE* handle, TIFF_Header& header);
// from tiff header we need
// offsetofIFD: accroding to parse header, location of directory entry in this file
// byteOrder: we need byte order to translate the value from litter to big endian
void parseDirectoryEntry(FILE* handle, IFD& aIFD, TIFF_Header& header);
// read values from file for each directory entry
void parseValueInDirectoryEntry(FILE* handle, IFD& aIFD);

int main(int argc, char* argv[]) {

    // set log level to debug
    spdlog::set_level(spdlog::level::trace);

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
    parseValueInDirectoryEntry(fileHandle, theIFD);

    // try to read the thumbnail from the 
    // get the stripOffset, In the current dng file, the thumbnail is not split into multiple parts.
    uint32_t stripOffset = 0;
    uint32_t stripByteCount = 0;
    auto search = theIFD.mTag2DirectoryEntry.find(DE_TAG::STRIP_OFFSETS);
    if (search != theIFD.mTag2DirectoryEntry.end()) {
        stripOffset = search->second.getLong();
        spdlog::trace("strip offset is {}", stripOffset);
    }

    search = theIFD.mTag2DirectoryEntry.find(DE_TAG::STRIP_BYTE_COUNTS);
    if (search != theIFD.mTag2DirectoryEntry.end()) {
        stripByteCount = search->second.getLong();
        spdlog::trace("strip byte count is {}", stripByteCount);
    }

    // read the thumbnail from file
    // In the current dng file, the thumbnail is not split into multiple parts.
    FILE* file = fopen("thumbnail.jpeg", "wb");
    if (file != nullptr) {
        char* thumbnail = static_cast<char*>(malloc(stripByteCount));
        fseek(fileHandle, stripOffset, SEEK_SET);
        fread(thumbnail, 1, stripByteCount, fileHandle);
        fwrite(thumbnail, 1, stripByteCount, file);
        fclose(file);
        free(thumbnail);
    }

    // close file 
    fclose(fileHandle);
    return 0;
}