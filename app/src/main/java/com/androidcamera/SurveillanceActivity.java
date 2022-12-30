package com.androidcamera;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class SurveillanceActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private Camera camera;
    private ImageButton btn_closeCamera;
    private DataChannel dataChannel;
    private TextView t_videoInfo;

    private AudioRecord audioRecord;
    private ExecutorService executorService;
    private int inputSamplesCount;
    private int inputBytesCount;
    private boolean is_pushing_audio;

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
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                openCamera();
                openAudioRecord();
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
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.hide();
        }
        // Surface
        surfaceView = findViewById(R.id.surveillance_sfv);
        surfaceHolder = surfaceView.getHolder();
        // Btn Close
        btn_closeCamera = findViewById(R.id.surveillance_btn_closeCamera);
        btn_closeCamera.setOnClickListener(this);
        // frameChannel
        dataChannel = new DataChannel();
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
        releaseAudioRecord();
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
        int sampleRateInHz = GlobalInfo.sampleRateInHz;
        int channelCfg = GlobalInfo.channelCfg;
        //设置预览格式（也就是每一帧的视频格式）YUV420下的NV21
        parameters.setPreviewFormat(ImageFormat.NV21);
        //设置预览图像分辨率
        parameters.setPreviewSize(width, height);
        //设置帧率
        parameters.setPreviewFpsRange(fps * 1000, fps * 1000);
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
                int new_width = screen_height * 16 / 9;
                int new_height = screen_height;
                lp.width = new_width;
                lp.height = new_height;
                lp.addRule(RelativeLayout.CENTER_HORIZONTAL);
                surfaceView.setLayoutParams(lp);
            }
        });

        // 注册编码器
        dataChannel.init(width, height, ImageFormat.NV21, fps, bitrate, rtmpPushUrl, sampleRateInHz, channelCfg);

        //设置监听获取视频流的每一帧
        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                dataChannel.receiveVideoData(data);
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
            dataChannel.release();
        }
    }

    private void releaseAudioRecord() {
        if (audioRecord != null) {
            is_pushing_audio = false;
            executorService.shutdown();
            audioRecord.release();
        }
    }

    private void flushVideoInfo() {
        String info = "RTMP推流地址: " + GlobalInfo.rtmpPushUrl + "\n"
                + "分辨率: " + String.valueOf(GlobalInfo.width) + "x" + String.valueOf(GlobalInfo.height) + "\n"
                + "帧率: " + String.valueOf(GlobalInfo.fps) + "\n"
                + "码率: " + String.valueOf(GlobalInfo.bitrate) + "\n"
                + "音频采样率: " + String.valueOf(GlobalInfo.sampleRateInHz) + "\n"
                + "音频类型: " + (GlobalInfo.channelCfg == 1 ? "单声道" : "立体声") + "\n";
        t_videoInfo.setText(info);
    }

    @SuppressLint("MissingPermission")
    private void openAudioRecord() {
        // 单线程池
        executorService = Executors.newSingleThreadExecutor();
        // 开启相机时已经初始化了编码通道，直接获取FAAC编码器一次可以读取的采样个数
        inputSamplesCount = dataChannel.getInputSamplesCount();
        // 计算FAAC编码器一次可以读取的字节个数
        inputBytesCount = inputSamplesCount * 2;

        int minBufferSize = AudioRecord.getMinBufferSize(
                GlobalInfo.sampleRateInHz, AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT) * 2;

        int maxBufferSize = inputBytesCount > minBufferSize ? inputBytesCount : minBufferSize;

        audioRecord = new AudioRecord(
                MediaRecorder.AudioSource.MIC,
                GlobalInfo.sampleRateInHz,
                (GlobalInfo.channelCfg == 1 ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_IN_STEREO),
                AudioFormat.ENCODING_PCM_16BIT,
                maxBufferSize);

        is_pushing_audio = true;
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                // 音频采样
                audioRecord.startRecording();
                // 读取inputSamplesCount个采样，一个采样16bit
                byte [] readBuffer = new byte [inputBytesCount];
                while(is_pushing_audio) {
                    int len = audioRecord.read(readBuffer, 0, readBuffer.length);
                    if(len > 0) {
                        dataChannel.receiveAudioData(readBuffer);
                    }
                }
                // 停止采样
                audioRecord.stop();
            }
        });
    }

}