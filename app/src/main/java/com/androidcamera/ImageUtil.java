package com.androidcamera;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.Log;

public class ImageUtil {

    @SuppressLint("ResourceAsColor")
    public static Bitmap drawCenterLable(Context context, Bitmap bmp, String text) {
        float scale = context.getResources().getDisplayMetrics().density;
        //创建一样大小的图片
        Bitmap newBmp = Bitmap.createBitmap(bmp.getWidth(), bmp.getHeight(), Bitmap.Config.ARGB_8888);
        //创建画布
        Canvas canvas = new Canvas(newBmp);
        canvas.drawBitmap(bmp, 0, 0, null);  //绘制原始图片
        canvas.save();
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(R.color.black); //白色半透明
        paint.setTextSize(10);
        paint.setDither(true);
        paint.setFilterBitmap(true);
        Rect rectText = new Rect();  //得到text占用宽高， 单位：像素
        paint.getTextBounds(text, 0, text.length(), rectText);
        double beginX = 20;
        double beginY = 30;
        canvas.drawText(text, (int)beginX, (int)beginY, paint);
        canvas.restore();
        return newBmp;
    }

    public static Bitmap drawWateMaskTopLeft(Context context, Bitmap bmp, String text, int fontsize, int color, int left, int top) {
        //创建一样大小的图片
        Bitmap newBmp = Bitmap.createBitmap(bmp.getWidth(), bmp.getHeight(), Bitmap.Config.ARGB_8888);
        //创建画布
        Canvas canvas = new Canvas(newBmp);
        canvas.drawBitmap(bmp, 0, 0, null);  //绘制原始图片
        canvas.save();
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(color);
        paint.setTextSize(fontsize);
        paint.setDither(true);
        paint.setFilterBitmap(true);
        Rect rectText = new Rect();  //得到text占用宽高， 单位：像素
        paint.getTextBounds(text, 0, text.length(), rectText);
        canvas.drawText(text, left, top, paint);
        canvas.restore();
        return newBmp;
    }

    public static byte[] getNV21FromBitmap(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();

        if (width > GlobalInfo.width || height > GlobalInfo.height) {
            return null;
        }
        int[] argb = new int[width * height];
        bitmap.getPixels(argb, 0, width, 0, 0, width, height);

        byte[] yuv = new byte[GlobalInfo.width * GlobalInfo.height * 3 / 2];
        encodeARGBTO720PNV21(yuv, argb, width, height);

        return yuv;
    }

    public static void encodeARGBTO720PNV21(byte[] yuv420sp, int[] argb, int width, int height) {
        int yIndex = 0;
        int uvIndex = GlobalInfo.width * GlobalInfo.height;
        int R, G, B, Y, U, V;
        int index = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                R = (argb[index] & 0xff0000) >> 16;
                G = (argb[index] & 0xff00) >> 8;
                B = argb[index] & 0xff;

                Y = ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
                U = ((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
                V = ((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;

                yuv420sp[yIndex++] = (byte) ((Y < 0) ? 0 : ((Y > 255) ? 255 : Y));

                //偶数行取U，基数行取V，并存储
                if (i % 2 == 0 && index % 2 == 0) {
                    yuv420sp[uvIndex++] = (byte) ((V < 0) ? 0 : ((V > 255) ? 255 : V));
                    yuv420sp[uvIndex++] = (byte) ((U < 0) ? 0 : ((U > 255) ? 255 : U));
                }
                index++;
            }
            yIndex += (GlobalInfo.width - width);
            uvIndex += (GlobalInfo.width - width) / 2;
        }
    }

}
