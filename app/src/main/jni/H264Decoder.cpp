#include "H264Decoder.h"
#include <android/log.h>

H264Decoder::H264Decoder()
: FFmpeg_Decoder(AV_CODEC_ID_H264)
, frameCallBack(nullptr)
, buf(nullptr)
, swsCtx(nullptr)
, outFrame(nullptr) {
    buf = new BYTE[MAX_BUF_SIZE];
}

H264Decoder::~H264Decoder() {
    if(buf) {
        delete [] buf;
        buf = nullptr;
    }
    if(outFrame) {
        av_frame_free(&outFrame);
    }
    if(swsCtx) {
        sws_freeContext(swsCtx);
    }
}

void H264Decoder::setFrameCallBack(FrameCallBack cb) {
    this->frameCallBack = cb;
}

void H264Decoder::handleOneFrame(AVFrame* frame) {
    UINT32 width = frame->width;
    UINT32 height = frame->height;
    // 转码为 RGBA, 供 Android 播放
    if(!swsCtx) {
        AVPixelFormat in_pix_fmt = codec_ctx->pix_fmt;
        AVPixelFormat out_pix_fmt = AV_PIX_FMT_RGBA;
        
        swsCtx = sws_getContext(width, height, in_pix_fmt, width, height, out_pix_fmt,
            SWS_BILINEAR, NULL, NULL, NULL);

        outFrame = av_frame_alloc();
        BYTE* outBuffer = (BYTE*) av_malloc(av_image_get_buffer_size(out_pix_fmt, width, height, 1));
        av_image_fill_arrays(outFrame->data, outFrame->linesize, outBuffer, out_pix_fmt, width, height, 1);
    }
    int ret = sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, outFrame->data, outFrame->linesize);
    __android_log_print(ANDROID_LOG_INFO, "RTMP", "sws_scale ret: %d\n", ret);
    frameCallBack(outFrame->data[0], outFrame->linesize[0], width, height, AV_PIX_FMT_RGBA);
}
