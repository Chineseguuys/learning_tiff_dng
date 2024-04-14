#include "tiff_ifd.h"
#include <spdlog/spdlog.h>


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