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
#include "parse_ifd.h"


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
    TIFF_Contexet theContext;
    memset(&theContext.mTiffHeader, 0x00, sizeof(TIFF_Header));
    parseHeader(fileHandle, theContext.mTiffHeader);

    parseAllDirectoryEntry(fileHandle, theContext);

    // close file
    if (fileHandle != nullptr)
        fclose(fileHandle);
    return 0;
}