//
// Created by glion on 2025-10-31.
// NDK 단 로그 출력 정의
//

#ifndef NDK_ESSENTIA_TEST_LOG_UTIL_H
#define NDK_ESSENTIA_TEST_LOG_UTIL_H

#include <android/log.h>

#define LOG_TAG "glion_jni"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

#endif //NDK_ESSENTIA_TEST_LOG_UTIL_H
