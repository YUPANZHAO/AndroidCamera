# x264
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := x264
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

#faac
include $(CLEAR_VARS)
LOCAL_MODULE := faac
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libfaac.a
include $(PREBUILT_STATIC_LIBRARY)

# rtmp
include $(CLEAR_VARS)
LOCAL_MODULE := rtmp
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/librtmp.a
include $(PREBUILT_STATIC_LIBRARY)

# ffmpeg
include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libavcodec.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avdevice
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libavdevice.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avfilter
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libavfilter.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libavformat.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libavutil.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libswresample.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/libswscale.so
include $(PREBUILT_SHARED_LIBRARY)

# nativehandle
include $(CLEAR_VARS)
LOCAL_MODULE := nativehandle
LOCAL_SRC_FILES := com_androidcamera_NativeHandle.cpp \
					VideoChannel.cpp \
					AudioChannel.cpp \
					FFmpeg_Decoder.cpp \
					AACDecoder.cpp \
					H264Decoder.cpp \
					VideoCapture.cpp
LOCAL_CONLYFLAGS := -std=c11
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_LDLIBS := -llog -lz
LOCAL_STATIC_LIBRARIES := rtmp x264 faac
LOCAL_SHARED_LIBRARIES := avformat avcodec avutil avdevice avfilter swresample swscale
include $(BUILD_SHARED_LIBRARY)