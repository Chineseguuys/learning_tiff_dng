#ifndef _TIFF_PARSE_IFD_H_
#define _TIFF_PARSE_IFD_H_

#include <stdio.h>
#include "tiff_ifd.h"

// parse header
void parseHeader(FILE* handle, TIFF_Header& header);
// from tiff header we need
// offsetofIFD: accroding to parse header, location of directory entry in this file
// byteOrder: we need byte order to translate the value from litter to big endian
void parseDirectoryEntry(FILE* handle, IFD& aIFD, TIFF_Header& header, int32_t currIFDOffset);
// read values from file for each directory entry
void parseValueInDirectoryEntry(FILE* handle, IFD& aIFD);

void parseAllDirectoryEntry(FILE* handle, TIFF_Contexet& contexet);

#endif /* _TIFF_PARSE_IFD_H_ */