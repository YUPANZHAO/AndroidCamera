package com.androidcamera;

public class GlobalInfo {

    // 分辨率（宽度）
    static int width = 640;
    // 分辨率（高度）
    static int height = 360;
    // 帧率
    static int fps = 25;
    // 比特率
    static int bitrate = 4000000;
    // RTMP推流地址
    static String rtmpPushUrl = "rtmp://192.168.43.59:50051/hls/test1";
    // 音频采样率
    static int sampleRateInHz = 44100;
    // 音频类型 1: 单声道 2: 立体声
    static int channelCfg = 2;

    // 注册码
    static String IdentCode = "";

    // grpc
    static String host = "192.168.0.107";
    static int port = 50052;
}
