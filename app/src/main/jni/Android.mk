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

# nativehandle
include $(CLEAR_VARS)
LOCAL_MODULE := nativehandle
LOCAL_SRC_FILES := com_androidcamera_NativeHandle.cpp \
					VideoChannel.cpp \
					AudioChannel.cpp
LOCAL_CONLYFLAGS := -std=c11
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := rtmp x264 faac
include $(BUILD_SHARED_LIBRARY)