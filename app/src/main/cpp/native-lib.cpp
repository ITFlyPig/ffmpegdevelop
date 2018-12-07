#include <jni.h>
#include <string>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <android/log.h>
// 宏定义类似java 层的定义,不同级别的Log LOGI, LOGD, LOGW, LOGE, LOGF。 对就Java中的 Log.i log.d
#define LOG_TAG    "JNILOG" // 这个是自定义的LOG的标识
//#undef LOG // 取消默认的LOG
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG, __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL,LOG_TAG, __VA_ARGS__)


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"






extern "C"
JNIEXPORT void JNICALL
Java_com_wangyuelin_ffmpegdevelop_MainActivity_decode(JNIEnv *env, jobject instance,
                                                      jstring fileName_, jobject surface) {
    const char *fileName = env->GetStringUTFChars(fileName_, 0);

    //1.获取解码器
    //2.开始解码
    //3.使用surface渲染解码出来的图像

    //判断输入是否为空
    if (fileName == NULL) {
        LOGE("输入的文件路径为空\n");
        return;
    } else if (strlen(fileName) == 0) {
        LOGE("输入的文件路径为空\n");
        return;
    }

    char errorStr[1024];
//    av_register_all();

    AVFormatContext *pFmtContext;//格式上下文
    //初始化AVFormatContext
    pFmtContext = avformat_alloc_context();
    int ret = avformat_open_input(&pFmtContext, fileName, 0, NULL);//地三个参数为空，ffmpeg会自动检测输入流的格式
    if (ret != 0) {

        av_strerror(ret, errorStr, 1024);
        LOGE("avformat_open_input 打开输入文件时失败 失败码：%d 失败描述：%s", ret, errorStr);

        return;
    }
    if (avformat_find_stream_info(pFmtContext, 0) < 0) {
        LOGE("avformat_find_stream_info 获取视频信息失败");
        return;
    }

    //获取视频流的索引
    int videoStreamIndex = av_find_best_stream(pFmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoStreamIndex < 0) {
        LOGD("av_find_best_stream 查找视频流的索引失败\n");
        return;
    }
    LOGE("视频流的索引位置:%d", videoStreamIndex);

    AVCodecContext *avctx;
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx) {
        LOGE("avcodec_alloc_context3函数返回错误");
        return;
    }

    int format = pFmtContext->streams[videoStreamIndex]->codecpar->format;

    LOGE("像素的格式：%d", format);

    if (avcodec_parameters_to_context(avctx, pFmtContext->streams[videoStreamIndex]->codecpar) < 0) {//初始化AVCodecContext解码器上下文的相关值
        LOGD("avcodec_parameters_to_context 初始化解码器上下文出错");
        return;

    }

    //由解码器上下文就可以得到解码器的id
    AVCodec* codec;
    codec = avcodec_find_decoder(avctx->codec_id);
    if (!codec) {
        LOGE("avcodec_find_decoder 寻找解码器失败");
        return;
    }
    LOGE("解码器寻找成功");

    //初始化解码上下文﻿AVCodecContext，在其中调用codec -> init()初始化解码器
    ret = avcodec_open2(avctx, codec, NULL);
    if (ret < 0) {
        av_strerror(ret, errorStr, 1024);
        LOGE("avcodec_open2 初始化解码器失败，失败码：%d，失败提示：%s", ret, errorStr);
        return;
    }

    LOGE("视频的信息如下：");
    LOGE("视屏的格式%s", pFmtContext->iformat->name);
    LOGE("视屏的时长%lld", pFmtContext->duration);
    LOGE("视频的宽：%d 高：%d", avctx->width, avctx->height);
    LOGE("解码器的名称：%s", codec->name);

    //获取Native Window
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (nativeWindow == NULL) {
        LOGE("创建ANativeWindow失败");
        return;
    }

    //视频的宽和高
    int videoW = avctx->width;
    int videoH = avctx->height;

    //设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoW, videoH, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer windowBuffer;

    //AVPacket存储压缩后的数据，可能包含多帧
    AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!packet) {
        LOGE("申请AVPacket出错");

        return;
    }
    //AVFrame存储解码后一帧视频数据
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        LOGE("申请AVFrame出错");
        return;
    }
    //存储一帧解码为YUV420的数据
    AVFrame *pFrameRGB = av_frame_alloc();

    //计算一帧图像需要的缓冲区大小
    //返回存储一个image所需要的字节总数
    int byteNum = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avctx->width, avctx->height, 1);
    if (byteNum < 0) {
        LOGE("计算一个Image所需要的内存出错");
        return;
    }
    //申请缓冲区
    uint8_t * outBuffer = (uint8_t*)av_malloc(byteNum * sizeof(uint8_t));

    //使用申请的缓冲区填充frame，现在frame有存数据的地方了
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, outBuffer, AV_PIX_FMT_RGBA, avctx->width, avctx->height, 1);

    //用于转码或缩放的的参数（主要包含转码前和转码后的尺寸格式信息）
    LOGE("SwsContext开始赋值");

//    if (avctx->pix_fmt == NULL) {
//        LOGE("pix_fmt为空" );
//        return;
//    }

    struct SwsContext* swsContext = sws_getContext(
             avctx->width,
             avctx->height,
             avctx->pix_fmt,
             avctx->width,
             avctx->height,
             AV_PIX_FMT_RGBA,
             SWS_BICUBIC,
             NULL, NULL, NULL
     );
    LOGE("SwsContext赋值结束");

    /*******开始解码*******/

    LOGE("开始循环");
    while (av_read_frame(pFmtContext, packet) >= 0) { //读取一个未解码的数包
        if (packet->stream_index == videoStreamIndex) {//只需要视频数据(因为可能是音频或者字母数据)
            //解码压缩的数据，得到显示需要的像素数据
            ret = avcodec_send_packet(avctx, packet);
            if (ret) {
                av_strerror(ret, errorStr, 1024);
                LOGE("avcodec_send_packet发送视频包出错 错误码：%d,错误信息：%s", ret, errorStr);
                continue;
            }

        }

        //读取解码后的帧
//        int recvRet = avcodec_receive_frame(avctx, frame);
//        if (ret) {
//            av_strerror(ret, errorStr, 1024);
//            LOGE("avcodec_receive_frame接受帧出错 错误码：%d,错误信息：%s", ret, errorStr);
//        }
        while (avcodec_receive_frame(avctx, frame) == 0) {
            LOGE("avcodec_send_packet接受到帧，开始解析");

            ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

            //开始解码一帧为指定格式
            sws_scale(swsContext, (uint8_t const *const *)frame->data, frame->linesize, 0, avctx->height, pFrameRGB->data, pFrameRGB->linesize);

            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            int dstStride = windowBuffer.stride * 4; //计算一行windowBuffer中一行的数据量
            uint8_t *src = (uint8_t * )(pFrameRGB->data[0]);
            //实际内存一行数量
            int srcStride = pFrameRGB->linesize[0];


            LOGE("源Frame的宽：%d 高:%d，显示窗口的宽：%d 高%d", frame->width, frame->height, windowBuffer.width, windowBuffer.height);

            // 由于window的stride和帧的stride不同,因此需要逐行复制
            int h;
            for (h = 0; h < videoH; h++) {
                memcpy( dst + h * dstStride, src + h * srcStride, srcStride);
            }


            ANativeWindow_unlockAndPost(nativeWindow);


        }

        av_packet_unref(packet);

    }
    av_free(outBuffer);
    av_free(pFrameRGB);
    av_free(frame);

    avcodec_close(avctx);
    avformat_close_input(&pFmtContext);

    ANativeWindow_release(nativeWindow);

    env->ReleaseStringUTFChars(fileName_, fileName);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_wangyuelin_ffmpegdevelop_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    char str[25];
    sprintf(str, "%d", avcodec_version());
    std::string hello = "Hello from C++";
    return env->NewStringUTF(str);
}


#ifdef __cplusplus
};
#endif
