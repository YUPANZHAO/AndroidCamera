package com.androidcamera;

public class NativeHandle {

    /**
     * 初始化视频通道
     * @param width     视频宽度
     * @param height    视频高度
     * @param fps       视频帧率
     * @param bitrate   视频码率
     * @return          状态码 0 成功 其他值 失败
     */
    public native int initVideoChannel(int width, int height, int fps, int bitrate);

    /**
     * 设置推流地址
     * @param rtmpPushPath  RTMP 流地址
     * @return              状态码 0 成功 其他值 失败
     */
    public native int setRtmpPushPath(String rtmpPushPath);

    /**
     * 开启推流
     * @return  状态码 0 成功 其他值 失败
     */
    public native int startPush();

    /**
     * 关闭推流
     * @return  状态码 0 成功 其他值 失败
     */
    public native int stopPush();

    /**
     * 编码一帧数据
     * @param data  帧数据
     * @return      状态码 0 成功 其他值 失败
     */
    public native int encodeOneFrame(byte [] data);

    /**
     * 设置音频参数
     * @param sampleRateInHz 采样率
     * @param channelCfg    1 单声道 2 立体声
     * @return              状态码 0 成功 其他值 失败
     */
    public native int setAudioEncodeParams(int sampleRateInHz, int channelCfg);

    /**
     * 获取音频采样输入个数
     * @return 采样个数，用于指导 AudioRecord 每次读取多少字节数据，一个采样两个字节
     */
    public native int getInputSamples();

    /**
     * 编码音频数据
     * @param data  数据
     * @return      状态码 0 成功 其他值 失败
     */
    public native int encodeAudioData(byte [] data);

    /**
     * 视频帧回调
     */
    public interface OnVideoListener {
        public int receiveOneFrame(byte [] data, int width, int height, int pix_fmt);
    }

    /**
     * 音频帧回调
     */
    public interface OnAudioListener {
        public int receiveOneFrame(byte [] data, int sampleRate, int channels);
    }

    /**
     * 拉流
     * @param rtmpUrl 流地址
     * @return        状态码 0 成功 其他值 失败
     */
    public native int pullStream(String rtmpUrl, OnVideoListener videoListener, OnAudioListener audioListener);

    /**
     * 停止拉流
     * @return 状态码 0 成功 其他值 失败
     */
    public native int stopPullStream();
}
