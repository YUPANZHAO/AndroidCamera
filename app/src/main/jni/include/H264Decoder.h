#ifndef H264DECODER_H
#define H264DECODER_H

#include "types.h"
#include <functional>
#include "FFmpeg_Decoder.h"

using namespace std;
using namespace ZYP;

class H264Decoder : public FFmpeg_Decoder {

    // data len width height pix_fmt
    using FrameCallBack = function<void(BYTE*,UINT32,UINT32,UINT32,AVPixelFormat)>;

    static const int MAX_BUF_SIZE = 1920 * 1080 * 2;

public:
    H264Decoder();
    ~H264Decoder();

public:
    void setFrameCallBack(FrameCallBack cb);

private:
    void handleOneFrame(AVFrame* frame);

private:
    FrameCallBack frameCallBack;
    BYTE* buf;

};

#endif // H264DECODER_H
