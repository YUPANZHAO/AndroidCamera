#include "com_androidcamera_NativeHandle.h"
#include "SafeQueue.hpp"
#include "VideoChannel.h"
#include <thread>
#include <android/log.h>
#include "AudioChannel.h"
#include <unistd.h>
#include "VideoCapture.h"
#include "H264Decoder.h"
#include "AACDecoder.h"
#include <android/native_window_jni.h>

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

            int audio_packet_count = 1;
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "开始推流");
            while(is_push_stream) {
                packets.pop(packet);
                
                if(!packet) {
                    continue;
                }

                packet->m_nInfoField2 = rtmp->m_stream_id;
                
                ret = RTMP_SendPacket(rtmp, packet, 1);
                
                if(packet->m_packetType == RTMP_PACKET_TYPE_AUDIO) {
                    if(audio_packet_count % 10 == 0) {
                        RTMPPacketPackUpCallBack(audio_channel->getAudioDecodeInfo());
                    }
                    audio_packet_count++;
                }

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

//========================== 拉流流程 ============================

JavaVM* g_VM = nullptr;
bool m_NeedDetach = false;

VideoCapture* video_capture = nullptr;
H264Decoder* video_decoder = nullptr;
AACDecoder* audio_decoder = nullptr;

string rtmp_pull_url;

BYTE adts_header[7];

const int MAX_VIDEO_BUF = 1024 * 1024 * 2;

bool is_pulling = false;

JNIEnv* env;
jobject videoObject;
jobject audioObject;
jclass videoClass;
jclass audioClass;
jmethodID videoCBId;
jmethodID audioCBId;
jbyteArray video_buffer = nullptr;
jbyteArray audio_buffer = nullptr;
jbyte* video_data;
jbyte* audio_data;

ANativeWindow* native_window = nullptr;
ANativeWindow_Buffer out_buffer;

void stop_pull_stream() {
    is_pulling = false;
    if(video_capture) {
        video_capture->stop();
    }
}

void thread_task() {
    __android_log_print(ANDROID_LOG_INFO, "RTMP", "开启拉流线程");
    // 初始化
    video_capture = new VideoCapture;
    video_decoder = new H264Decoder;
    audio_decoder = new AACDecoder;
    // 获取H264，交由解码器
    video_capture->setNaluCB([&](NALU_TYPE type, BYTE* data, UINT32 len) {
        __android_log_print(ANDROID_LOG_INFO, "RTMP", "video type: %d, data len: %d", type, len);
        if(type == NALU_TYPE_IDR) {
            BYTE nalu_header [] = { 0x00, 0x00, 0x01 };
            video_decoder->receiveData(nalu_header, 3);
        }else {
            BYTE nalu_header [] = { 0x00, 0x00, 0x00, 0x01 };
            video_decoder->receiveData(nalu_header, 4);
        }
        video_decoder->receiveData(data, len);
    });
    // 获取AAC，交给AAC解码器
    video_capture->setAudioCB([&](AUDIO_TYPE type, AUDIO_CHANNEL_TYPE channelCfg, BYTE* data, UINT32 len) {
        __android_log_print(ANDROID_LOG_INFO, "RETURNPCM", "audio type: %d, data len: %d", type, len);
        if(type == AUDIO_INFO) {
            UINT32 num0 = data[0] & 0x7;
            UINT32 num1 = data[1] & 0x80;
            UINT32 sampleIdx = ((num0 << 1) | (num1 >> 7));
            UINT32 channelsIdx = ((data[1] & 0x78) >> 3);
            BYTE temp [] = { 0xFF, 0xF1, 0x40, 0x00, 0x00, 0x1F, 0xFC };
            temp[2] |= (sampleIdx << 2) & 0x3C;
            temp[2] |= ((channelsIdx & 0x4) >> 2) & 0x1;
            temp[3] |= ((channelsIdx & 0x3) << 6) & 0xC0;
            memcpy(adts_header, temp, 7);
        }else {
            UINT32 all_len = 7 + len;
            BYTE num0 = ((all_len & 0x1800) >> 11) & 0x3;
            BYTE num1 = ((all_len & 0x7F8) >> 3) & 0xFF;
            BYTE num2 = ((all_len & 0x7) << 5) & 0xE0;
            adts_header[3] &= 0xFC;
            adts_header[4] &= 0x00;
            adts_header[5] &= 0x1F;
            adts_header[3] |= num0;
            adts_header[4] |= num1;
            adts_header[5] |= num2;
            audio_decoder->receiveData(adts_header, 7);
            audio_decoder->receiveData(data, len);
        }
    });
    // 获取到H264解码器返回的原始帧数据，
    ANativeWindow_acquire(native_window);
    video_decoder->setFrameCallBack([&](BYTE* data, UINT32 len, UINT32 width, UINT32 height, AVPixelFormat pix_fmt) {
        __android_log_print(ANDROID_LOG_INFO, "RTMP", "get RGBA frame data len: %d\n", len);
        // 渲染RGBA
        ANativeWindow_setBuffersGeometry(native_window, width, height, WINDOW_FORMAT_RGBA_8888);
        int ret = ANativeWindow_lock(native_window, &out_buffer, NULL);
        __android_log_print(ANDROID_LOG_INFO, "RTMP", "ANativeWindow_lock ret: %d\n", ret);
        int oneLineBytes = (out_buffer.stride << 2);
        int srcStride = len;
        for(int i=0; i < height; ++i) {
            memcpy((BYTE*)out_buffer.bits + i * oneLineBytes, data + i * srcStride, srcStride);
        }
        ANativeWindow_unlockAndPost(native_window);
    });
    // 获取到AAC解码器返回的原始数据，提交至JAVA层
    audio_decoder->setPCMCallBack([&](BYTE* data, UINT32 len, UINT32 sampleRate, UINT32 channels) {
        __android_log_print(ANDROID_LOG_INFO, "RETURNPCM", "data len: %d, sampleRate: %d, channels: %d\n", len, sampleRate, channels);
        if(!audio_buffer) audio_buffer = env->NewByteArray(len);
        audio_data = env->GetByteArrayElements(audio_buffer, NULL); 
        memcpy(audio_data, data, len);
        env->ReleaseByteArrayElements(audio_buffer, audio_data, 0);
        env->CallIntMethod(audioObject, audioCBId, audio_buffer, sampleRate, channels);
    });
    // 开启拉流线程
    video_capture->setRtmpURL(rtmp_pull_url);
    video_capture->start(); 
    __android_log_print(ANDROID_LOG_INFO, "RTMP", "拉流线程结束");
    stop_pull_stream();
    // 释放资源
    if(video_capture) {
        video_capture->stop();
        delete video_capture;
        video_capture = nullptr;
    }
    if(video_decoder) {
        delete video_decoder;
        video_decoder = nullptr;
    }
    if(audio_decoder) {
        delete audio_decoder;
        audio_decoder = nullptr;
    }
    // 释放缓冲区
    if(video_buffer) {
        env->DeleteLocalRef(video_buffer);
        video_buffer = nullptr;
    }
    if(audio_buffer) {
        env->DeleteLocalRef(audio_buffer);
        audio_buffer = nullptr;
    }
    if(native_window) {
        ANativeWindow_release(native_window);
        native_window = nullptr;
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_pullStream
(JNIEnv *envv, jobject obj, jstring rtmpUrl, jobject surface, jobject audioListner) {
    if(is_pulling) return -1;
    is_pulling = true;
    
    // 保存拉流地址
    const char *url = envv->GetStringUTFChars(rtmpUrl, NULL);
    rtmp_pull_url = string(url);
    envv->ReleaseStringUTFChars(rtmpUrl, url);
    // 获取javaVM，以便线程中获取env
    envv->GetJavaVM(&g_VM);
    // 生成对象的全局应用
    videoObject = envv->NewGlobalRef(surface);
    audioObject = envv->NewGlobalRef(audioListner);
    // 获取native_windows
    native_window = ANativeWindow_fromSurface(envv, surface);
    __android_log_print(ANDROID_LOG_INFO, "SurfaceEnd", "Start %d", native_window);
    //  创建线程
    std::thread pullStream([&]() {
        // 获取当前native线程是否有没有被附加到jvm环境中
        int getEnvStat = g_VM->GetEnv((void**)&env, JNI_VERSION_1_6);
        if(getEnvStat == JNI_EDETACHED) {
            // 如果没有， 主动附加到jvm环境中，获取到env
            if(g_VM->AttachCurrentThread(&env, NULL) != 0) {
                is_pulling = false;
                return;
            }
            m_NeedDetach = true;
        }
        // 获取到要回调的类
        audioClass = env->GetObjectClass(audioObject);
        if(audioClass == 0) {
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "Unable to find class");
            g_VM->DetachCurrentThread();
            is_pulling = false;
            return;
        }
        // 获取要回调的方法ID
        audioCBId = env->GetMethodID(audioClass, "receiveOneFrame", "([BII)I");
        if(audioCBId == NULL) {
            __android_log_print(ANDROID_LOG_INFO, "RTMP", "Unable to find method id");
            is_pulling = false;
            return;
        }
        // 线程任务
        thread_task();
        //释放当前线程
        if(m_NeedDetach) {
            g_VM->DetachCurrentThread();
            m_NeedDetach = false;
        }
        //释放你的全局引用的接口，生命周期自己把控
        env->DeleteGlobalRef(videoObject);
        env->DeleteGlobalRef(audioObject);
        env = nullptr;
        __android_log_print(ANDROID_LOG_INFO, "SurfaceEnd", "End");
    });
    // 线程分离
    pullStream.detach();
    // 设定了返回值一定要返回，否则会产生SIGSEGV信号
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_stopPullStream
(JNIEnv *env, jobject obj) {
    stop_pull_stream();
    return 0;
}