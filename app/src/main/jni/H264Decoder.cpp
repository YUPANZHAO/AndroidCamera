#include "H264Decoder.h"

H264Decoder::H264Decoder()
: FFmpeg_Decoder(AV_CODEC_ID_H264)
, frameCallBack(nullptr)
, buf(nullptr) {
    buf = new BYTE[MAX_BUF_SIZE];
}

H264Decoder::~H264Decoder() {
    if(buf) {
        delete [] buf;
        buf = nullptr;
    }
}

void H264Decoder::setFrameCallBack(FrameCallBack cb) {
    this->frameCallBack = cb;
}

void H264Decoder::handleOneFrame(AVFrame* frame) {
    UINT32 width = frame->width;
    UINT32 height = frame->height;
    int ysize = width * height;
    int uvsize = ysize >> 2;
    int bufsize = ysize + (uvsize << 1);
    memcpy(buf, frame->data[0], ysize);
    memcpy(buf + ysize, frame->data[1], uvsize);
    memcpy(buf + ysize + uvsize, frame->data[2], uvsize);
    frameCallBack(buf, bufsize, width, height, codec_ctx->pix_fmt);
}
