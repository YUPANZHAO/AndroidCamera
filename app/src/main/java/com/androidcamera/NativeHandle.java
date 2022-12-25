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

}
