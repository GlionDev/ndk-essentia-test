//
// Created by glion on 2025-11-05.
// EmbeddingHelper 클래스 생성자 / 소멸자 및 기타 구현
//

#include "embedding_helper.h"
#include <pool.h>
#include <essentia.h>
#include <cmath>


using namespace essentia;
using namespace essentia::standard;

using namespace NdkEssentiaEmbedding;

EmbeddingHelper::EmbeddingHelper() : ort_env(ORT_LOGGING_LEVEL_WARNING, "ResonnanceAppOrt") {
    initEssentia();
}

EmbeddingHelper::~EmbeddingHelper() {
    shutdownEssentia();
}

void EmbeddingHelper::initEssentia() {
    if (!essentiaInitialized) {
        essentia::init();
        essentiaInitialized = true;
    }
}

void EmbeddingHelper::shutdownEssentia() {
    if (essentiaInitialized) {
        essentia::shutdown();
        essentiaInitialized = false;
    }
}

bool EmbeddingHelper::initOrtSession(const std::string &model_path) {
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1); // 추론 스레드 설정

    // nnapi 가속기 추가(성능 최적화)
//    try {
//        session_options.AppendExecutionProvider_Nnapi({});
//        LOGI("NNAPI Execution Provider added.");
//    } catch (const Ort::Exception &e) {
//        LOGW("Failed to add NNAPI EP: %s", e.what());
//    }

    try {
        // ort_env를 사용하여 세션 객체를 생성하고 스마트 포인터에 저장합니다.
        ort_session = std::make_unique<Ort::Session>(ort_env, model_path.c_str(), session_options);
        LOGI("ONNX Session successfully initialized with model: %s", model_path.c_str());
        return true;
    } catch (const Ort::Exception& e) {
        LOGE("Failed to create ONNX Session: %s", e.what());
        ort_session.reset(); // 실패 시 세션 포인터 초기화
        return false;
    }
}