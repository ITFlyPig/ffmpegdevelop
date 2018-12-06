package com.wangyuelin.ffmpegdevelop;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;


public class VideoView  extends SurfaceView {

    private Context mContext;

    public VideoView(Context context) {
        super(context);
    }

    public VideoView(Context context, AttributeSet attrs) {
      super(context, attrs);
      mContext = context;
        init();

    }

    private void init() {
        SurfaceHolder holder = getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void player(final String input) {
        new Thread(new Runnable() {
            @Override
            public void run() {
//        绘制功能 不需要交给SurfaveView        VideoView.this.getHolder().getSurface()
                MainActivity mainActivity  = (MainActivity) mContext;
                mainActivity.decode(input, VideoView.this.getHolder().getSurface());
            }
        }).start();
    }

}
