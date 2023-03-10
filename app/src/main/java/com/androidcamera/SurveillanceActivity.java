package com.androidcamera;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.Rect;
import android.hardware.Camera;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.Image;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.VideoView;

import com.google.gson.JsonParser;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.sql.Time;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.annotation.security.RunAs;

import io.grpc.Grpc;

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

    private AudioTrack audioTrack;
    private SurfaceView sfv_videoView;
    private SurfaceHolder sfv_videoView_holder;

    private Thread message_callback_listener;
    private Thread send_heart_beat_thread;
    private boolean is_sending_heart_beat;

    private Handler mainHandler;

    private NV21ToBitmap nv21ToBitmap;

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surveillance);
        init();
        flushVideoInfo();
        // ?????????????????????Activity????????????
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
                listenMessageCallBack();
                sendHeartBeat();
            }
        }).start();
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    private void init() {
        // ???????????????
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        }
        // ?????????????????????
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
        //?????????????????????, ????????????????????????
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        // sfv video
        sfv_videoView = findViewById(R.id.surveillance_sfv_pull);
        sfv_videoView_holder = sfv_videoView.getHolder();

        // mainHandler
        mainHandler = new Handler(Looper.myLooper()) {
            @Override
            public void handleMessage(@NonNull Message msg) {
                super.handleMessage(msg);
                Log.i("HandleMsg", String.valueOf(msg.what));
                if(msg.what == 1) {
                    sfv_videoView.setVisibility(View.GONE);
                    sfv_videoView.invalidate();
                } else if(msg.what == 2) {
                    sfv_videoView.setVisibility(View.VISIBLE);
                    sfv_videoView.invalidate();
                }
            }
        };

        // nv21 to bitmap
        nv21ToBitmap = new NV21ToBitmap(this);
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
        System.out.printf("????????????\n");
        releaseCamera(camera);
        releaseAudioRecord();
        releasePullThread();
        releaseMsgCBThread();
        releaseHeartBeatThread();
        super.onDestroy();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //????????????
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
        //??????????????????
        Camera.Parameters parameters = camera.getParameters();
        //????????????????????????????????????
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
        String encryption = GlobalInfo.encryption;
        //?????????????????????????????????????????????????????????YUV420??????NV21
        parameters.setPreviewFormat(ImageFormat.NV21);
        //???????????????????????????
        parameters.setPreviewSize(width, height);
        //????????????
        parameters.setPreviewFpsRange(fps * 1000, fps * 1000);
        //??????camera??????
        camera.setParameters(parameters);
        try {
            camera.setPreviewDisplay(surfaceHolder);
        } catch (IOException e) {
            e.printStackTrace();
        }

        // ???16:9?????????????????????
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

        // ???????????????
        dataChannel.init(width, height, ImageFormat.NV21, fps, bitrate, rtmpPushUrl, sampleRateInHz, channelCfg, encryption);

        //???????????????????????????????????????
        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                Bitmap bitmap = nv21ToBitmap.nv21ToBitmap(data, GlobalInfo.width, GlobalInfo.height);
                SimpleDateFormat formatter = new SimpleDateFormat("yyyy???MM???dd??? HH:mm:ss");
                Date curDate = new Date(System.currentTimeMillis());
                String str = formatter.format(curDate);
                Bitmap ret_bitmap = ImageUtil.drawWateMaskTopLeft(SurveillanceActivity.this,
                        bitmap, str, (int)(GlobalInfo.height * 0.07),
                        Color.argb(255, 255, 255,255),
                        (int)(GlobalInfo.width * 0.0625),
                        (int)(GlobalInfo.height * 0.139));
                if(ret_bitmap == null) return;
                byte[] ret_data = ImageUtil.getNV21FromBitmap(ret_bitmap);
                if(ret_data == null) return;
                dataChannel.receiveVideoData(ret_data);
            }
        });
        //??????startPreview()????????????preview???surface
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

    private void releasePullThread() {
        dataChannel.stopPullStream();
        Message msg = new Message();
        msg.what = 1;
        mainHandler.sendMessage(msg);
        if (audioTrack != null) {
            audioTrack.stop();
            audioTrack.release();
            audioTrack = null;
        }
    }

    private void releaseMsgCBThread() {
        GrpcService grpcService = new GrpcService();
        grpcService.deviceQuit();
        grpcService.shutdown();
        try {
            Log.i("StreamMsg", "start join");
            message_callback_listener.join();
            Log.i("StreamMsg", "end join");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void releaseHeartBeatThread() {
        is_sending_heart_beat = false;
        try {
            send_heart_beat_thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void flushVideoInfo() {
        String info = "RTMP????????????: " + GlobalInfo.rtmpPushUrl + "\n"
                + "?????????: " + String.valueOf(GlobalInfo.width) + "x" + String.valueOf(GlobalInfo.height) + "\n"
                + "??????: " + String.valueOf(GlobalInfo.fps) + "\n"
                + "??????: " + String.valueOf(GlobalInfo.bitrate) + "\n"
                + "???????????????: " + String.valueOf(GlobalInfo.sampleRateInHz) + "\n"
                + "????????????: " + (GlobalInfo.channelCfg == 1 ? "?????????" : "?????????") + "\n"
                + "?????????: " + GlobalInfo.IdentCode + "\n";
        t_videoInfo.setText(info);
    }

    @SuppressLint("MissingPermission")
    private void openAudioRecord() {
        // ????????????
        executorService = Executors.newSingleThreadExecutor();
        // ????????????????????????????????????????????????????????????FAAC??????????????????????????????????????????
        inputSamplesCount = dataChannel.getInputSamplesCount();
        // ??????FAAC??????????????????????????????????????????
        inputBytesCount = inputSamplesCount * 2;

        int minBufferSize = AudioRecord.getMinBufferSize(
                GlobalInfo.sampleRateInHz, AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT) * 2;

        int maxBufferSize = inputBytesCount > minBufferSize ? inputBytesCount : minBufferSize;

        audioRecord = new AudioRecord(
                MediaRecorder.AudioSource.VOICE_COMMUNICATION,
                GlobalInfo.sampleRateInHz,
                (GlobalInfo.channelCfg == 1 ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_IN_STEREO),
                AudioFormat.ENCODING_PCM_16BIT,
                maxBufferSize);

        is_pushing_audio = true;
        executorService.submit(new Runnable() {
            @Override
            public void run() {
                // ????????????
                audioRecord.startRecording();
                // ??????inputSamplesCount????????????????????????16bit
                byte [] readBuffer = new byte [inputBytesCount];
                while(is_pushing_audio) {
                    int len = audioRecord.read(readBuffer, 0, readBuffer.length);
                    if(len > 0) {
                        dataChannel.receiveAudioData(readBuffer);
                    }
                }
                // ????????????
                audioRecord.stop();
            }
        });
    }

    private void playRtmp(String rtmp_url) {
        System.out.printf("Start Play Rtmp\n");
        Message msg = new Message();
        msg.what = 2;
        mainHandler.sendMessage(msg);
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        dataChannel.pullStream(rtmp_url,
        sfv_videoView_holder.getSurface()
        , new NativeHandle.OnAudioListener() {
            @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
            @Override
            public int receiveOneFrame(byte[] data, int sampleRate, int channels) {
                System.out.printf("audio callback: %d %d %d\n", data.length, sampleRate, channels);
                if(audioTrack == null) {
                    int buffsize = AudioTrack.getMinBufferSize(sampleRate,
                            (channels == 1 ? AudioFormat.CHANNEL_OUT_MONO : AudioFormat.CHANNEL_OUT_STEREO),
                            AudioFormat.ENCODING_PCM_16BIT);
                    System.out.printf("audio buffsize: %d\n", buffsize);

                    AudioAttributes audioAttributes = new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_MEDIA)
                            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                            .build();
                    AudioFormat audioFormat = new AudioFormat.Builder()
                            .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                            .setSampleRate(sampleRate)
                            .build();
                    audioTrack = new AudioTrack(audioAttributes, audioFormat, buffsize,
                            AudioTrack.MODE_STREAM, AudioManager.AUDIO_SESSION_ID_GENERATE);

                    System.out.printf("audioTrack state: %d\n", audioRecord.getState());
                    if(audioRecord.getState() != AudioRecord.STATE_UNINITIALIZED)
                        audioTrack.play();
                }
                int ret = audioTrack.write(data, 0, data.length);
                System.out.printf("audio write ret: %d\n", ret);
                return 0;
            }
        });
    }

    private void listenMessageCallBack() {
        message_callback_listener = new Thread(new Runnable() {
            @Override
            public void run() {
                GrpcService grpcService = new GrpcService();
                grpcService.streamCall(GlobalInfo.IdentCode,
                new GrpcService.MessageCallback() {
                    @Override
                    public void handle(String json) {
                        Log.i("StreamMsg", "handle json" + json);
                        try {
                            JSONObject object = new JSONObject(json);
                            String oper = object.getString("operation");
                            Log.i("StreamMsg", "oper " + oper);
                            if(oper.equalsIgnoreCase("talkStart")) {
                                Log.i("StreamMsg", "start");
                                String rtmp_url = object.getString("rtmp_pull_url");
                                Log.i("StreamMsg", "rtmp_url " + rtmp_url);
                                playRtmp(rtmp_url);
                            }else if(oper.equalsIgnoreCase("talkStop")) {
                                Log.i("StreamMsg", "stop???");
                                releasePullThread();
                            }
                        } catch (JSONException e) {
                            e.printStackTrace();
                            return;
                        }
                    }
                });
                grpcService.shutdown();
            }
        });
        message_callback_listener.start();
    }

    private void sendHeartBeat() {
        send_heart_beat_thread = new Thread(new Runnable() {
            @Override
            public void run() {
                is_sending_heart_beat = true;
                while(is_sending_heart_beat) {
                    GrpcService grpcService = new GrpcService();
                    grpcService.heartBeat();
                    grpcService.shutdown();
                    Log.i("HeartBeat", "heart beat");
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {}
                }
                Log.i("HeartBeat", "??????????????????");
            }
        });
        send_heart_beat_thread.start();
    }
}