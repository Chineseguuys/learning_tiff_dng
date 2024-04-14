#include "tiff_ifd.h"
#include <spdlog/spdlog.h>

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
            int curr_pos = ftell(handle);
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
                // Because of the memory alignment, when the number of bytes read is less than 4, it needs to be followed by 0
                int alignmentBytes = 4 - valueLength;
                fseek(handle, alignmentBytes, SEEK_CUR);
                spdlog::trace("memory alignment {} bytes", alignmentBytes);
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
            aIFD.mTag2DirectoryEntry.insert(std::pair(entry.mTag, entry));
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

void parseValueInDirectoryEntry(FILE* handle, IFD& aIFD) {
    for (auto& [tag, entry] : aIFD.mTag2DirectoryEntry) {
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
        case DE_FIELD_TYPE::BYTE:
        case DE_FIELD_TYPE::ASCII:
            entry.mValuesArray = static_cast<unsigned char*>(malloc(entry.mValueCounts * 1));
            fread(entry.mValuesArray, 1, entry.mValueCounts, handle);
            break;
        case DE_FIELD_TYPE::SHORT:
        case DE_FIELD_TYPE::SSHORT:
            entry.mValuesArray = static_cast<unsigned char*>(malloc(entry.mValueCounts * 2));
            fread(entry.mValuesArray, 1, 2 * entry.mValueCounts, handle);
            break;
        case DE_FIELD_TYPE::LONG:
        case DE_FIELD_TYPE::SLONG:
        case DE_FIELD_TYPE::FLOAT:
            entry.mValuesArray = static_cast<unsigned char*>(malloc(entry.mValueCounts * 4));
            fread(entry.mValuesArray, 1, 4 * entry.mValueCounts, handle);
            break;
        case DE_FIELD_TYPE::RATIONAL:
        case DE_FIELD_TYPE::SRATIONAL:
            entry.mValuesArray = static_cast<unsigned char*>(malloc(entry.mValueCounts * 8));
            fread(entry.mValuesArray, 1, entry.mValueCounts * 8, handle);
            break;
        default:
            break;
        }
    }
}