package com.androidcamera;

import android.accounts.OnAccountsUpdateListener;
import android.media.AudioTrack;
import android.view.Surface;

import java.lang.reflect.Array;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.util.Arrays;

public class DataChannel {

    static {
        System.loadLibrary("nativehandle");
    }

    private NativeHandle nativeHandle;

    public DataChannel() {
        nativeHandle = new NativeHandle();
    }

    public void init(int width, int height, int pix_fmt, int fps, int bitrate, String rtmpPushUrl, int sampleRateInHz, int channelCfg) {
        System.out.printf("Init:\n" +
                "\twidth:\t%d\n" +
                "\theight:\t%d\n" +
                "\tpix_fmt:\t%d\n" +
                "\tfpx:\t%d\n" +
                "\tbitrate:\t%d\n" +
                "\tsampleRateInHz:\t%d\n" +
                "\tchannelCfg:\t%d\n",
                width, height, pix_fmt, fps, bitrate, sampleRateInHz, channelCfg);
        int res = nativeHandle.initVideoChannel(width, height, fps, bitrate);
        System.out.printf("initVideoChannel res: %d\n", res);
        res = nativeHandle.setAudioEncodeParams(sampleRateInHz, channelCfg);
        System.out.printf("setAudioEncodeParams res: %d\n", res);
        res = nativeHandle.setRtmpPushPath(rtmpPushUrl);
        System.out.printf("setRtmpPushPath res: %d\n", res);
        res = nativeHandle.startPush();
        System.out.printf("startPush res: %d\n", res);
    }

    public void receiveVideoData(byte [] data) {
//        System.out.printf("data length: %d\n", data.length);
        int res = nativeHandle.encodeOneFrame(data);
//        System.out.printf("receive res: %d\n", res);
    }

    public void receiveAudioData(byte [] data) {
//        System.out.printf("data length: %d\n", data.length);
        int res = nativeHandle.encodeAudioData(data);
//        System.out.printf("receive res: %d\n", res);
    }

    public void release() {
        int res = nativeHandle.stopPush();
        System.out.printf("release res: %d\n", res);
        res = nativeHandle.stopPullStream();
        System.out.printf("release res: %d\n", res);
    }

    public int getInputSamplesCount() {
        return nativeHandle.getInputSamples();
    }

    public int pullStream(String rtmpUrl, Surface surface, NativeHandle.OnAudioListener audioListener) {
        return nativeHandle.pullStream(rtmpUrl, surface, audioListener);
    }

    public int stopPullStream() {
        return nativeHandle.stopPullStream();
    }

}
