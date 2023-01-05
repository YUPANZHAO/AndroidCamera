#include "AACDecoder.h"
#include <android/log.h>

AACDecoder::AACDecoder()
: FFmpeg_Decoder(AV_CODEC_ID_AAC)
, pcmCallBack(nullptr)
, swrCtx(nullptr) {
    buf = new BYTE [MAX_BUF_SIZE];
}

AACDecoder::~AACDecoder() {
    if(buf) {
        delete [] buf;
        buf = nullptr;
    }
    if(swrCtx) {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }
}

void AACDecoder::setPCMCallBack(PCMCallBack callback) {
    this->pcmCallBack = callback;
}

void AACDecoder::handleOneFrame(AVFrame* frame) {
    __android_log_print(ANDROID_LOG_INFO, "RTMP", "audio handle one frame");
    UINT32 data_size = av_get_bytes_per_sample(codec_ctx->sample_fmt);
    if(data_size < 0) {
        return;
    }
    // 重采样转为16BIT，供Android播放
    if(!swrCtx) {
        AVSampleFormat in_sample_fmt = codec_ctx->sample_fmt;
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
        int in_sample_rate = codec_ctx->sample_rate;
        int out_sample_rate = in_sample_rate;
        uint64_t in_ch_layout = codec_ctx->channel_layout;
        uint64_t out_ch_layout = in_ch_layout;
        swrCtx = swr_alloc();
        swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate,
            in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
        swr_init(swrCtx);
    }
    swr_convert(swrCtx, &buf, MAX_BUF_SIZE, (const uint8_t **)frame->data, frame->nb_samples);
    UINT32 len = av_samples_get_buffer_size(NULL, codec_ctx->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1);
    pcmCallBack(buf, len, codec_ctx->sample_rate, codec_ctx->channels);
}
