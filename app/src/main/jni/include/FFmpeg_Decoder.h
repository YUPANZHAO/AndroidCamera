#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

#include "types.h"
#include <functional>

extern "C" {
    #include "libavcodec/avcodec.h"
}

using namespace std;
using namespace ZYP;

class FFmpeg_Decoder {

public:
    FFmpeg_Decoder(AVCodecID codec_id);
    virtual ~FFmpeg_Decoder();

public:
    void receiveData(BYTE* data, UINT32 len);

private:
    virtual void handleOneFrame(AVFrame* frame) = 0;

protected:
    AVCodecID codec_id;
    AVCodec* codec = nullptr;
    AVCodecParserContext* codec_parser_ctx = nullptr;
    AVCodecContext* codec_ctx = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;
    
    static const int INPUT_BUFFER_SIZE = 1920 * 1080 * 2;
    BYTE* inputBuf;

};

#endif // FFMPEG_DECODER_H
