#include <jni.h>
#include <string>


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


extern "C"
JNIEXPORT void JNICALL
Java_com_wangyuelin_ffmpegdevelop_MainActivity_decode(JNIEnv *env, jobject instance,
                                                      jstring fileName_) {
    const char *fileName = env->GetStringUTFChars(fileName_, 0);

    //1.获取解码器
    //2.开始解码
    //3.使用surface渲染解码出来的图像

    //判断输入是否为空
    if (fileName == NULL) {
        LOGD("输入的文件路径为空\n");
        return;
    } else if (strlen(fileName) == 0) {
        LOGD("输入的文件路径为空\n");
        return;
    }

    AVFormatContext *pFmtContext;//格式上下文
    //初始化AVFormatContext
    pFmtContext = avformat_alloc_context();
    int ret = avformat_open_input(&pFmtContext, fileName, NULL, NULL);//地三个参数为空，ffmpeg会自动检测输入流的格式
    if (ret != 0) {
        LOGD("avformat_open_input 打开输入文件时失败");
        return;
    }

    //获取视频流的索引
    int streamIndex = av_find_best_stream(pFmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (streamIndex < 0) {
        LOGD("av_find_best_stream 查找视频流的索引失败\n");
        return;
    }

    AVCodecContext *avctx;
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx) {
        LOGD("avcodec_alloc_context3函数返回错误");
        return;
    }

    if (avcodec_parameters_to_context(avctx, pFmtContext->streams[streamIndex]->codecpar) <
        0) {//初始化AVCodecContext解码器上下文的相关值
        LOGD("avcodec_parameters_to_context 初始化解码器上下文出错");
        return;

    }

    //由解码器上下文就可以得到解码器的id
    AVCodec* codec;
    codec = avcodec_find_decoder(avctx->codec_id);
    if (!codec) {
        LOGD("avcodec_find_decoder 寻找解码器失败");
        return;
    }




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
