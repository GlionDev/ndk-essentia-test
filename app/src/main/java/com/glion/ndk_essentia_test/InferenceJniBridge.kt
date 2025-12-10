package com.glion.ndk_essentia_test

/**
 * Project : ndk-test
 * File : InferenceJniBridge
 * Created by glion on 2025-10-31
 *
 * Description:
 * - JNI Bridge
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
class InferenceJniBridge {
    companion object {
        // Used to load the 'ndk_test' library on application startup.
        init {
            System.loadLibrary("inference-jni-bridge")
        }
    }

    /**
     * 최종 임베딩 구하는 모든 파이프라인
     * @param path 오디오파일 경로
     * @param modelPath 모델 파일 경로
     */
    external fun allInferencePipeline(path: String, modelPath: String) : FloatArray?

    /**
     * temp : 테스트 - 특정 특징 추출하여 코사인 유사도 비교용
     * @param path 오디오 파일 경로
     * @param type 특징 타입(L : LogMel, C : Chroma, T : Tempo)
     */
    external fun getFlattenFeature(path: String, type: String) : FloatArray?
}