package com.androidcamera;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.widget.EditText;

public class SettingActivity extends AppCompatActivity {

    private EditText et_rtmpPushUrl;
    private EditText et_width;
    private EditText et_height;
    private EditText et_fps;
    private EditText et_bitrate;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);

        init();
    }

    private void init() {
        // 去除默认标题栏
        ActionBar actionBar=getSupportActionBar();
        if(actionBar!=null){
            actionBar.hide();
        }
        // findViewById
        et_rtmpPushUrl = findViewById(R.id.setting_et_rtmp_push_url);
        et_width = findViewById(R.id.setting_et_width);
        et_height = findViewById(R.id.setting_et_height);
        et_fps = findViewById(R.id.setting_et_fps);
        et_bitrate = findViewById(R.id.setting_et_bitrate);
        // init value
        et_rtmpPushUrl.setText(GlobalInfo.rtmpPushUrl);
        et_width.setText(String.valueOf(GlobalInfo.width));
        et_height.setText(String.valueOf(GlobalInfo.height));
        et_fps.setText(String.valueOf(GlobalInfo.fps));
        et_bitrate.setText(String.valueOf(GlobalInfo.bitrate));
    }

    @Override
    protected void onStop() {
        super.onStop();
        GlobalInfo.rtmpPushUrl = et_rtmpPushUrl.getText().toString();
        GlobalInfo.width = Integer.valueOf(et_width.getText().toString());
        GlobalInfo.height = Integer.valueOf(et_height.getText().toString());
        GlobalInfo.fps = Integer.valueOf(et_fps.getText().toString());
        GlobalInfo.bitrate = Integer.valueOf(et_bitrate.getText().toString());
    }

}