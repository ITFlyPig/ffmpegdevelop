package com.wangyuelin.ffmpegdevelop;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avutil");
        System.loadLibrary("avformat");
        System.loadLibrary("avcodec");
        System.loadLibrary("swscale");
        System.loadLibrary("swresample");
        System.loadLibrary("avfilter");

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        String filePath = "/storage/emulated/0/DCIM/Camera/test.mp4";
        File mp4 = new File(filePath);
        if (!mp4.exists()) {
            tv.setText("mp4文件不存在");
            return;
        }
        decode(filePath);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void decode(String fileName);
}
