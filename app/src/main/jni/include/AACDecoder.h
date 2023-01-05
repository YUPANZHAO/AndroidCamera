#ifndef AACDECODER_H
#define AACDECODER_H

#include "types.h"
#include <memory.h>
#include <functional>
#include "FFmpeg_Decoder.h"

extern "C" {
    #include "libswresample/swresample.h"
    #include "libswscale/swscale.h"
}

using namespace ZYP;
using namespace std;

class AACDecoder : public FFmpeg_Decoder {

    using PCMCallBack = function<void(BYTE*,UINT32,UINT32,UINT32)>;

    static const int MAX_BUF_SIZE = 10240;

public:
    AACDecoder();
    ~AACDecoder();

public:
    void setPCMCallBack(PCMCallBack callback);

private:
    void handleOneFrame(AVFrame* frame);

private:
    PCMCallBack pcmCallBack;
    BYTE* buf;

    SwrContext* swrCtx;
};

#endif // AACDECODER_H
