#用来指定 CMake 最低版本为3.4.1，如果没指定，执行 cmake 命令时可能会出错
cmake_minimum_required(VERSION 3.4.1)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp
        src/main/cpp/decode_video.c)



include_directories(libs/include)

set(DIR ../../../../libs)

message(${DIR})


# FFmpeg编译出了6个库,这里添加----avutil
add_library(avutil
        SHARED
        IMPORTED )
set_target_properties( avutil
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/armeabi/libavutil.so)

# FFmpeg编译出了6个库,这里添加----swresample
add_library( swresample
        SHARED
        IMPORTED )
set_target_properties( swresample
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/armeabi/libswresample.so )

# FFmpeg编译出了6个库,这里添加----avcodec
add_library( avcodec
        SHARED
        IMPORTED )
set_target_properties( avcodec
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/armeabi/libavcodec.so )

# FFmpeg编译出了6个库,这里添加----avfilter
add_library( avfilter
        SHARED
        IMPORTED)
set_target_properties( avfilter
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/armeabi/libavfilter.so )

# FFmpeg编译出了6个库,这里添加----swscale
add_library( swscale
        SHARED
        IMPORTED)
set_target_properties( swscale
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/armeabi/libswscale.so )

# FFmpeg编译出了6个库,这里添加----avformat
add_library( avformat
        SHARED
        IMPORTED)
set_target_properties( avformat
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/armeabi/libavformat.so )


find_library(log-lib
             log )



target_link_libraries( # Specifies the target library.
        native-lib

        # 这里需要注意下,下面这些ffmpeg的so库编译是有先后顺序的
        # 下面的顺序是没问题的,如果想验证编译顺序,可以自己变换顺序试试.
        avutil
        avformat
        avcodec
        swscale
        swresample
        avfilter

        ${log-lib} )
