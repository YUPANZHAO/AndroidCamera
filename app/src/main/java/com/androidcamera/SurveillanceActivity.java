package com.androidcamera;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import android.content.pm.ActivityInfo;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.io.IOException;
import java.util.List;

public class SurveillanceActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private Camera camera;
    private ImageButton btn_closeCamera;
    private FrameChannel frameChannel;
    private TextView t_videoInfo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surveillance);

        init();
        flushVideoInfo();
        // 延迟开机，等待Activity渲染完成
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                openCamera();
            }
        }).start();
    }

    private void init() {
        // 去除状态栏
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        }
        // 去除默认标题栏
        ActionBar actionBar=getSupportActionBar();
        if(actionBar!=null){
            actionBar.hide();
        }
        // Surface
        surfaceView = findViewById(R.id.surveillance_sfv);
        surfaceHolder = surfaceView.getHolder();
        // Btn Close
        btn_closeCamera = findViewById(R.id.surveillance_btn_closeCamera);
        btn_closeCamera.setOnClickListener(this);
        // frameChannel
        frameChannel = new FrameChannel();
        // video Info
        t_videoInfo = findViewById(R.id.surveillance_t_videoInfo);
        //设置屏幕为横屏, 设置后会锁定方向
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.surveillance_btn_closeCamera:
                finish();
                break;
            default:
                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.out.printf("安全退出\n");
        releaseCamera(camera);
    }

    @Override
    protected void onResume() {
        super.onResume();
        //设置屏幕为横屏, 设置后会锁定方向
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //自动对焦
        camera.autoFocus(new Camera.AutoFocusCallback() {
            @Override
            public void onAutoFocus(boolean success, Camera camera) {
                camera.cancelAutoFocus();
            }
        });
        return super.onTouchEvent(event);
    }

    private void openCamera() {
        camera = Camera.open(Camera.CameraInfo.CAMERA_FACING_BACK);
        //获取相机参数
        Camera.Parameters parameters = camera.getParameters();
        //获取相机支持的预览的大小
        Camera.Size previewSize = getCameraPreviewSize(parameters);
//        int width = previewSize.width;
//        int height = previewSize.height;
        int width = GlobalInfo.width;
        int height = GlobalInfo.height;
        int fps = GlobalInfo.fps;
        int bitrate = GlobalInfo.bitrate;
        String rtmpPushUrl = GlobalInfo.rtmpPushUrl;
        //设置预览格式（也就是每一帧的视频格式）YUV420下的NV21
        parameters.setPreviewFormat(ImageFormat.NV21);
        //设置预览图像分辨率
        parameters.setPreviewSize(width, height);
        //设置帧率
        parameters.setPreviewFpsRange(fps * 1000, fps * 1000);
        //相机旋转90度
//        camera.setDisplayOrientation(90);
        //自动对焦
//        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        //配置camera参数
        camera.setParameters(parameters);
        try {
            camera.setPreviewDisplay(surfaceHolder);
        } catch (IOException e) {
            e.printStackTrace();
        }

        // 按16:9的比例显示画面
        surfaceView.post(new Runnable() {
            @Override
            public void run() {
                RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) surfaceView.getLayoutParams();
                int screen_height = surfaceView.getMeasuredHeight();
                int new_width =  screen_height * 16 / 9;
                int new_height = screen_height;
                lp.width = new_width;
                lp.height = new_height;
                lp.addRule(RelativeLayout.CENTER_HORIZONTAL);
                surfaceView.setLayoutParams(lp);
            }
        });

        // 注册编码器
        frameChannel.init(width, height, ImageFormat.NV21, fps, bitrate, rtmpPushUrl);

        //设置监听获取视频流的每一帧
        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                frameChannel.receive(data);
            }
        });
        //调用startPreview()用以更新preview的surface
        camera.startPreview();
    }

    private Camera.Size getCameraPreviewSize(Camera.Parameters parameters) {
        List<Camera.Size> list = parameters.getSupportedPreviewSizes();
        Camera.Size needSize = null;
        for (Camera.Size size : list) {
            if (needSize == null) {
                needSize = size;
                continue;
            }
            if (size.width >= needSize.width) {
                if (size.height > needSize.height) {
                    needSize = size;
                }
            }
        }
        return needSize;
    }

    public void releaseCamera(Camera camera) {
        if (camera != null) {
            camera.setPreviewCallback(null);
            camera.stopPreview();
            camera.release();
            frameChannel.release();
        }
    }

    private void flushVideoInfo() {
        String info = "RTMP推流地址: " + GlobalInfo.rtmpPushUrl + "\n"
                    + "分辨率: " + String.valueOf(GlobalInfo.width) + "x" + String.valueOf(GlobalInfo.height) + "\n"
                    + "帧率: " + String.valueOf(GlobalInfo.fps) + "\n"
                    + "比特率: " + String.valueOf(GlobalInfo.bitrate) + "\n";
        t_videoInfo.setText(info);
    }

}