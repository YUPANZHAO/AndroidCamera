#ifndef _TYPES_H_
#define _TYPES_H_

#include <inttypes.h>

namespace ZYP {

using BYTE = uint8_t;
using UINT32 = uint32_t;

enum NALU_TYPE {
    NALU_TYPE_SPS,
    NALU_TYPE_PPS,
    NALU_TYPE_IDR,
    NALU_TYPE_OTHER
};

enum AUDIO_TYPE {
    AUDIO_INFO,
    AUDIO_DATA
};

enum AUDIO_CHANNEL_TYPE {
    AUDIO_CHANNEL_MONO,
    AUDIO_CHANNEL_STEREO
};

} // namespace ZYP

#endif // _TYPES_H_
