package com.example.androidcamera;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import java.io.IOException;
import java.util.List;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceHolder surfaceHolder;
    private Camera camera;

    private Button btn_openCamera;
    private Button btn_closeCamera;
    private boolean is_camera_open;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        init();
    }

    private void init() {
        // Surface
        SurfaceView surfaceView = findViewById(R.id.main_sfv);
        surfaceHolder = surfaceView.getHolder();
        // Btn Open Close
        is_camera_open = false;
        btn_openCamera = findViewById(R.id.main_btn_openCamera);
        btn_openCamera.setOnClickListener(this);
        btn_closeCamera = findViewById(R.id.main_btn_closeCamera);
        btn_closeCamera.setOnClickListener(this);
    }

    @Override
    protected void onStart() {
        super.onStart();
        Permission.checkPermission(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(Permission.isPermissionGranted(this)) {
            Log.i("PERMISSION","请求权限成功");
        }
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == Permission.REQUEST_CODE) {
            for (int grantResult : grantResults) {
                if (grantResult != PackageManager.PERMISSION_GRANTED) {
                    Log.e("Permission","授权失败！");
                    // 授权失败，退出应用
                    this.finish();
                    return;
                }
            }
        }
    }

    private void openCamera() {
        camera = Camera.open(Camera.CameraInfo.CAMERA_FACING_BACK);
        //获取相机参数
        Camera.Parameters parameters = camera.getParameters();
        //获取相机支持的预览的大小
        Camera.Size previewSize = getCameraPreviewSize(parameters);
//        int width = previewSize.width;
//        int height = previewSize.height;
        int width = 640;
        int height = 360;
        int fps = 15;
        int bitrate = 1000000;
        //设置预览格式（也就是每一帧的视频格式）YUV420下的NV21
        parameters.setPreviewFormat(ImageFormat.NV21);
        //设置预览图像分辨率
        parameters.setPreviewSize(width, height);
        //设置帧率
        parameters.setPreviewFpsRange(fps * 1000, fps * 1000);
        //相机旋转90度
        camera.setDisplayOrientation(90);
        //配置camera参数
        camera.setParameters(parameters);
        try {
            camera.setPreviewDisplay(surfaceHolder);
        } catch (IOException e) {
            e.printStackTrace();
        }

        //设置监听获取视频流的每一帧
        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {

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
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.main_btn_openCamera:
                if(is_camera_open == false) {
                    openCamera();
                    is_camera_open = true;
                }
                break;
            case R.id.main_btn_closeCamera:
                if(is_camera_open == true) {
                    releaseCamera(camera);
                    is_camera_open = false;
                }
                break;
            default:
                break;
        }
    }
}