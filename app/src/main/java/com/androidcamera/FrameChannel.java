package com.androidcamera;

public class FrameChannel {

    static {
        System.loadLibrary("nativehandle");
    }

    private NativeHandle nativeHandle;

    public FrameChannel() {
        nativeHandle = new NativeHandle();
    }

    public void init(int width, int height, int pix_fmt, int fps, int bitrate) {
        System.out.printf("Init:\n" +
                "\twidth:\t%d\n" +
                "\theight:\t%d\n" +
                "\tpix_fmt:\t%d\n" +
                "\tfpx:\t%d\n" +
                "\tbitrate:\t%d\n",
                width, height, pix_fmt, fps, bitrate);
        int res = nativeHandle.initVideoChannel(width, height, fps, bitrate);
        System.out.printf("initVideoChannel res: %d\n", res);
        res = nativeHandle.setRtmpPushPath("rtmp://192.168.43.59:50051/hls/test");
        System.out.printf("setRtmpPushPath res: %d\n", res);
        res = nativeHandle.startPush();
        System.out.printf("startPush res: %d\n", res);
    }

    public void receive(byte [] data) {
        System.out.printf("data length: %d\n", data.length);
        int res = nativeHandle.encodeOneFrame(data);
        System.out.printf("receive res: %d\n", res);
    }

    public void release() {
        int res = nativeHandle.stopPush();
        System.out.printf("release res: %d\n", res);
    }

}
