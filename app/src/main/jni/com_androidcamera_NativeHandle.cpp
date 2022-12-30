#include "com_androidcamera_NativeHandle.h"
#include "SafeQueue.hpp"
#include "VideoChannel.h"
#include <thread>
#include <android/log.h>
#include "AudioChannel.h"

// 视频参数
int _width;
int _height;
int _fps;
int _bitrate;

// 安全缓冲队列，存储解码出的RTMP包
SafeQueue<RTMPPacket*> packets;

// 视频编码通道
VideoChannel* video_channel = nullptr;

// 音频编码通道
AudioChannel* audio_channel = nullptr;

// 开始推流的时间戳
uint32_t push_start_time = 0;

// RTMP推流地址
string rtmp_push_path;

// 是否在推流
bool is_push_stream = 0;

auto RTMPPacketPackUpCallBack = [](RTMPPacket* packet) {
    if(packet) {
        // 将编码后的RTMP包塞入缓冲队列，等待推送
        packet->m_nTimeStamp = RTMP_GetTime() - push_start_time;
        packets.push(packet);
    }
};

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_initVideoChannel
(JNIEnv *env, jobject obj, jint width, jint height, jint fps, jint bitrate) {
    // 记录视频参数
    _width = width;
    _height = height;
    _fps = fps;
    _bitrate = bitrate;
    // 关闭上一个编码通道
    if(video_channel) {
        delete video_channel;
        video_channel = nullptr;
    } 
    if(audio_channel) {
        delete audio_channel;
        audio_channel = nullptr;
    }
    // 创建新的编码通道
    video_channel = new VideoChannel;
    video_channel->setVideoEncoderParams(_width, _height, _fps, _bitrate);
    video_channel->setRTMPPacketCallBack(RTMPPacketPackUpCallBack); 
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_setRtmpPushPath
(JNIEnv *env, jobject obj, jstring url) {
    const char * path = env->GetStringUTFChars(url, 0);
    // 记录RTMP推流地址
    rtmp_push_path = string(path);
    env->ReleaseStringUTFChars(url, path);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_startPush
(JNIEnv *env, jobject obj) {
    // 避免重复开启推流
    if(is_push_stream) return -1;
    __android_log_print(ANDROID_LOG_INFO, "RTMP", "开始创建推流线程");
    // 创建推流线程
    thread rtmp_push_thread([&]() {
        
        RTMP* rtmp = nullptr;
        RTMPPacket* packet = nullptr;
        
        do {
            rtmp = RTMP_Alloc();
            if(!rtmp) {
                __android_log_print(ANDROID_LOG_INFO, "RTMP", "申请 RTMP 内存失败");
                break;
            }
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "申请 RTMP 内存成功");

            RTMP_Init(rtmp);
            rtmp->Link.timeout = 5;

            int ret = RTMP_SetupURL(rtmp, const_cast<char*>(rtmp_push_path.c_str()));
            if(!ret) {
                __android_log_print(ANDROID_LOG_INFO, "RTMP", "设置 RTMP 推流服务器地址失败");
                break;
            }
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "设置 RTMP 推流服务器地址成功 %s", rtmp_push_path.c_str());

            RTMP_EnableWrite(rtmp);

            ret = RTMP_Connect(rtmp, NULL);
            if(!ret) {
                __android_log_print(ANDROID_LOG_INFO, "RTMP", "连接 RTMP 服务器失败");
                break;
            }
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "连接 RTMP 服务器成功");

            ret = RTMP_ConnectStream(rtmp, 0);
            if(!ret) {
                __android_log_print(ANDROID_LOG_INFO, "RTMP", "连接 RTMP 流失败");
                break;
            }
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "连接 RTMP 流成功");

            push_start_time = RTMP_GetTime();

            packets.setWork(1);

            is_push_stream = true;

            if(audio_channel) {
                RTMPPacketPackUpCallBack(audio_channel->getAudioDecodeInfo());
            }

            __android_log_print(ANDROID_LOG_INFO, "RTMP", "开始推流");
            while(is_push_stream) {
                packets.pop(packet);
                
                if(!packet) {
                    continue;
                }

                packet->m_nInfoField2 = rtmp->m_stream_id;
                
                ret = RTMP_SendPacket(rtmp, packet, 1);

                if(packet) {
                    RTMPPacket_Free(packet);
                    delete packet;
                    packet = nullptr;
                }
                
                if(!ret) {
                    __android_log_print(ANDROID_LOG_INFO, "RTMP", "RTMP 数据包推送失败");
                    break;
                }
            }
        }while(false);

        if(rtmp) {
            RTMP_Close(rtmp);
            RTMP_Free(rtmp);
        }

        if(packet) {
            RTMPPacket_Free(packet);
            delete packet;
            packet = nullptr;
        }
        
        __android_log_print(ANDROID_LOG_INFO, "RTMP", "推流线程结束");
    });
    rtmp_push_thread.detach();
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_stopPush
(JNIEnv *env, jobject obj) {
    // 释放编码通道
    if(video_channel) {
        delete video_channel;
        video_channel = nullptr;
    }
    if(audio_channel) {
        delete audio_channel;
        audio_channel = nullptr;
    }
    // 关闭推流线程
    is_push_stream = false;
    packets.setWork(0);
    // 清空缓冲队列
    packets.clear();
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_encodeOneFrame
(JNIEnv *env, jobject obj, jbyteArray buffer) {
    // 未开启推流或编码通道未初始化
    if(!is_push_stream || !video_channel) return -1;
    
    jbyte* data = env->GetByteArrayElements(buffer, NULL);
    // 编码一帧数据
    video_channel->encodeData((uint8_t*)data);
    env->ReleaseByteArrayElements(buffer, data, 0);

    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_setAudioEncodeParams
(JNIEnv *env, jobject obj, jint sampleRateInHz, jint channelCfg) {
    // 创建音频通道
    audio_channel = new AudioChannel;
    audio_channel->setRTMPPacketCallBack(RTMPPacketPackUpCallBack);
    // 设置音频编码参数
    audio_channel->setAudioEncodeParams(sampleRateInHz, channelCfg);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_getInputSamples
(JNIEnv *env, jobject obj) {
    if(audio_channel) {
        return audio_channel->getInputSamples();
    }
    return -1;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_encodeAudioData
(JNIEnv *env, jobject obj, jbyteArray buffer) {
    // 未开启推流或编码通道未初始化
    if(!is_push_stream || !audio_channel) return -1;
    
    jbyte* data = env->GetByteArrayElements(buffer, NULL);
    // 编码音频数据
    audio_channel->encodeAudioData((uint8_t*)data);
    env->ReleaseByteArrayElements(buffer, data, 0);

    return 0;
}