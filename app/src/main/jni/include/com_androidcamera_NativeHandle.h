/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_androidcamera_NativeHandle */

#ifndef _Included_com_androidcamera_NativeHandle
#define _Included_com_androidcamera_NativeHandle
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    initVideoChannel
 * Signature: (IIII)I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_initVideoChannel
  (JNIEnv *, jobject, jint, jint, jint, jint);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    setRtmpPushPath
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_setRtmpPushPath
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    startPush
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_startPush
  (JNIEnv *, jobject);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    stopPush
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_stopPush
  (JNIEnv *, jobject);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    encodeOneFrame
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_encodeOneFrame
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    setAudioEncodeParams
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_setAudioEncodeParams
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    getInputSamples
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_getInputSamples
  (JNIEnv *, jobject);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    encodeAudioData
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_encodeAudioData
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    pullStream
 * Signature: (Ljava/lang/String;Lcom/androidcamera/NativeHandle/OnVideoListener;Lcom/androidcamera/NativeHandle/OnAudioListener;)I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_pullStream
  (JNIEnv *, jobject, jstring, jobject, jobject);

/*
 * Class:     com_androidcamera_NativeHandle
 * Method:    stopPullStream
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_androidcamera_NativeHandle_stopPullStream
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
