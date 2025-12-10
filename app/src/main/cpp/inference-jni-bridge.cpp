#include <jni.h>
#include <string>

#include "embedding_helper.h"

using namespace NdkEssentiaEmbedding;

// 모든 과정 JNI 함수
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_glion_ndk_1essentia_1test_InferenceJniBridge_allInferencePipeline(
        JNIEnv* env,
        jobject thiz,
        jstring filePath_,ㅕㅠ
        jstring modelPath_
) {
    try {
        // 전체 과정 시간 측정
        RunTimerLogger timer("allInferencePipeline");

        EmbeddingHelper resonanceEmd = EmbeddingHelper();

        // 1. JNI 입력 처리
        const char *filePath = env->GetStringUTFChars(filePath_, nullptr);
        std::string cppFilePath(filePath);
        env->ReleaseStringUTFChars(filePath_, filePath);

        const char *modelPath = env->GetStringUTFChars(modelPath_, nullptr);

        // 2. 순수 C++ 함수 호출
        AudioData audioResults = resonanceEmd.loadAudioFile(cppFilePath);

        // 3. 세그먼트 분할
        std::vector<std::vector<float>> segments = resonanceEmd.segmenter(audioResults);

        // 4. 세그먼트 별 특징 추출
        // 모든 세그먼트의 FullFeatures(Mel, Chroma, Tempo)를 저장할 컨테이너
        std::vector<FullFeatures> allSegmentFeatures;
        // 모든 세그먼트 순회 및 특징 추출
        for (const auto& segment : segments) {
            // segment는 std::vector<float> 타입이며, extractFeatures의 첫 번째 인자로 사용됩니다.

            // 각 세그먼트에 대해 LogMel, Chroma, Tempo 특징을 동시에 추출합니다.
            // config는 세그멘터에서 사용한 것과 동일한 객체를 사용하거나,
            // 필요에 따라 이전에 정의한 설정을 사용합니다.
            FullFeatures features = resonanceEmd.extractFeatures(segment);

            // 결과 저장
            allSegmentFeatures.push_back(std::move(features));
        }
        // 모델 초기화
        resonanceEmd.initOrtSession(modelPath);
        env->ReleaseStringUTFChars(modelPath_, modelPath);

        // 5. ONNX 모델 입력 텐서 생성
        std::vector<Ort::Value> inputTensors = resonanceEmd.createInputTensors(allSegmentFeatures);
        // 6. ONNX 모델 추론
        std::vector<std::vector<float>> embeddingVector = resonanceEmd.runInference(inputTensors, "embedding");
        // 7. ONNX 임베딩 후처리
        // 7-1. 세그먼트별 정규화
        for (auto& vec : embeddingVector) { // [V] 만큼 반복
            resonanceEmd.l2Normalize(vec); // 1D 벡터용 헬퍼 함수 호출
        }
        // 7-2. 평균 풀링
        std::vector<float> finalEmbedding = resonanceEmd.meanPooling(embeddingVector);
        if (finalEmbedding.empty()) {
            throw std::runtime_error("Mean pooling resulted in an empty vector.");
        }
        // 7-3. 최종 정규화
        resonanceEmd.l2Normalize(finalEmbedding);

        // 8. 최종 embedding 반환
        size_t finalSize = finalEmbedding.size();
        jfloatArray javaResultArray = env->NewFloatArray(static_cast<jsize>(finalSize));
        if (javaResultArray == nullptr) {
            throw std::runtime_error("Failed to create new jfloatArray (Out of Memory).");
        }
        env->SetFloatArrayRegion(javaResultArray, 0, static_cast<jsize>(finalSize), finalEmbedding.data());
        return javaResultArray;
    }
    catch (const std::exception& e) {
        // C++ 예외를 Java의 'java.lang.RuntimeException'으로 변환하여 던집니다.
        // Kotlin/Java의 try-catch에서 이 예외를 잡을 수 있습니다.
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
        return nullptr; // Java/Kotlin 측에 null을 반환
    }
    catch (...) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), "Unknown C++ exception occurred in JNI.");
        return nullptr;
    }
}

// temp : 테스트 - 특정 특징 추출하여 코사인 유사도 비교용
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_glion_ndk_1essentia_1test_InferenceJniBridge_getFlattenFeature(
        JNIEnv* env,
        jobject thiz,
        jstring filePath_,
        jstring type_) {
    try {
        // 전체 과정 시간 측정
        RunTimerLogger timer("allInferencePipeline");

        EmbeddingHelper resonanceEmd = EmbeddingHelper();

        // 1. JNI 입력 처리
        const char *filePath = env->GetStringUTFChars(filePath_, nullptr);
        std::string cppFilePath(filePath);
        env->ReleaseStringUTFChars(filePath_, filePath);

        const char *typePtr = env->GetStringUTFChars(type_, nullptr);
        std::string type(typePtr);
        env->ReleaseStringUTFChars(type_, typePtr);


        // 2. 순수 C++ 함수 호출
        AudioData audioResults = resonanceEmd.loadAudioFile(cppFilePath);

        // 3. 세그먼트 분할
        std::vector<std::vector<float>> segments = resonanceEmd.segmenter(audioResults);

        // 4. 세그먼트 별 특징 추출
        // 모든 세그먼트의 FullFeatures(Mel, Chroma, Tempo)를 저장할 컨테이너
        std::vector<FullFeatures> allSegmentFeatures;
        // 모든 세그먼트 순회 및 특징 추출
        for (const auto& segment : segments) {
            // segment는 std::vector<float> 타입이며, extractFeatures의 첫 번째 인자로 사용됩니다.

            // 각 세그먼트에 대해 LogMel, Chroma, Tempo 특징을 동시에 추출합니다.
            // config는 세그멘터에서 사용한 것과 동일한 객체를 사용하거나,
            // 필요에 따라 이전에 정의한 설정을 사용합니다.
            FullFeatures features = resonanceEmd.extractFeatures(segment);

            // 결과 저장
            allSegmentFeatures.push_back(std::move(features));
        }

        // 5. 특징 전체 1차원 배열로 평탄화
        std::map<std::string, std::vector<float>> flattenFeatures = resonanceEmd.flattenFeature(allSegmentFeatures);
        std::vector<float> resultAtType;
        // 6. 타입에 맞게 값 할당
        if(type == "L") {
            resultAtType = flattenFeatures["LogMel"];
        } else if(type == "C") {
            resultAtType = flattenFeatures["Chroma"];
        } else if(type == "T") {
            resultAtType = flattenFeatures["Tempo"];
        } else {
            throw std::invalid_argument("Invalid type received : " + type);
        }

        // 7. 최종 값 floatArray 로 리턴
        size_t finalSize = resultAtType.size();
        jfloatArray javaResultArray = env->NewFloatArray(static_cast<jsize>(finalSize));
        if (javaResultArray == nullptr) {
            throw std::runtime_error("Failed to create new jfloatArray (Out of Memory).");
        }
        env->SetFloatArrayRegion(javaResultArray, 0, static_cast<jsize>(finalSize), resultAtType.data());
        return javaResultArray;

    }
    catch (const std::exception& e) {
        // C++ 예외를 Java의 'java.lang.RuntimeException'으로 변환하여 던집니다.
        // Kotlin/Java의 try-catch에서 이 예외를 잡을 수 있습니다.
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
        return nullptr; // Java/Kotlin 측에 null을 반환
    }
    catch (...) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), "Unknown C++ exception occurred in JNI.");
        return nullptr;
    }
}
