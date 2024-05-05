#include "tiff_ifd.h"
#include <spdlog/spdlog.h>


//======Directory Entry Function======
int16_t DirectoryEntry::getShort() {
    if (this->mValuesArray == nullptr) {
        spdlog::warn("Directory entry does not containe value");
        return 0;
    }
    int16_t value = 0;
    // copy 2 bytes from valuesArray
    // Todo: consider about this->mVaulesCount
    memcpy(&value, this->mValuesArray + this->mValuesArrayOffsets, 2);
    this->mValuesArrayOffsets += 2;
    // Todo: consider litter endian and big endian
    return value;
}

int32_t DirectoryEntry::getLong() {
    if (this->mValuesArray == nullptr) {
        spdlog::warn("Directory entry does not containe value");
        return 0;
    }
    int32_t value = 0;
    // Todo: consider about this->mVaulesCount
    memcpy(&value, this->mValuesArray + this->mValuesArrayOffsets, 4);
    mValuesArrayOffsets += 4;
    // Todo: consider litter endian and big endian
    return value;
}

void DirectoryEntry::printEntry() {
    spdlog::trace("mTag:{},mFiledType:{},mValueCounts:{},mOffsets:{},mValuesArrays:{}",
        mTag,
        mFieldType,
        mValueCounts,
        mOffsets,
        fmt::ptr(mValuesArray)
    );

    if (mValuesArray != nullptr) {
        std::stringstream ss;
        for (int i = 0; i < FIELD_TYPE2SIZE[mFieldType] * mValueCounts; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0');
            ss << static_cast<int>(mValuesArray[i]);
            ss << ",";
        }

        spdlog::trace("{}", ss.str());
    }
}

DirectoryEntry::DirectoryEntry()
    : mTag(0),
    mFieldType(0),
    mValueCounts(0),
    mOffsets(0),
    mValuesArrayOffsets(0),
    mValuesArray(nullptr) {}

bool IFD::containDirectoryEntry(DE_TAG tag) {
    if (this->mTag2DirectoryEntry.find(tag) != this->mTag2DirectoryEntry.end()) {
        return true;
    }
    return false;
}

DirectoryEntry &IFD::getDirectoryEntry(DE_TAG tag) {
    return this->mTag2DirectoryEntry.at(tag);
}

IFD::IFD()
    : mNumDE(0),
    mOffsets(0),
    mTag2DirectoryEntry(),
    mSubIFDs() {}

TIFF_Header::TIFF_Header()
    : mOffsetofIFD(0),
    mFlags(0),
    mByteOrder(0) {}
