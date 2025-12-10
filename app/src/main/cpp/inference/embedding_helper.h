//
// Created by glion on 2025-11-05.
// EmbeddingHelper 클래스 선언 헤더파일. 각 구현은 별도의 cpp 에서 진행
//

#ifndef NDK_ESSENTIA_TEST_EMBEDDING_HELPER_H
#define NDK_ESSENTIA_TEST_EMBEDDING_HELPER_H

// std 컨테이너 사용을 위해 명시적으로 헤더 추가
#include <map>
#include <vector>
#include <string>
#include <onnxruntime_cxx_api.h>
#include <cstdint>

#include "common/log_util.h" // 로그 유틸리티 사용
#include "common/cal_runtime.h" // 시간 측정 유틸리티 사용
#include "struct/embedding_config.h"
#include "common/audio_data.h"

namespace NdkEssentiaEmbedding {
    // 모든 2D/1D 특징을 담을 컨테이너
    using FullFeatures = std::map<std::string, std::vector<std::vector<float>>>;

    class EmbeddingHelper {
    public:
        EmbeddingHelper();
        ~EmbeddingHelper();

        // 오디오 로드
        AudioData loadAudioFile(
                const std::string& filePath,
                const EmbeddingConfig& config = EmbeddingConfig()
        );

        // 세그먼트 분할
        std::vector<std::vector<float>> segmenter(
                const AudioData& audioData,
                const EmbeddingConfig& config = EmbeddingConfig()
        );

        // LogMel 추출
        std::vector<std::vector<float>> computeLogMel(
                const std::vector<float>& audio,
                const EmbeddingConfig& config = EmbeddingConfig()
        );

        // Chroma 추출
        std::vector<std::vector<float>> computeChroma(
                const std::vector<float>& audio,
                const EmbeddingConfig& config = EmbeddingConfig()
        );

        // tempo 추출
        std::vector<float> computeTempo(
                const std::vector<float>& audio,
                const EmbeddingConfig& config = EmbeddingConfig()
        );

        // 특징 추출
        FullFeatures extractFeatures(
                const std::vector<float>& audio,
                const EmbeddingConfig& config = EmbeddingConfig()
        );

        // 모델 초기화
        bool initOrtSession(const std::string& model_path);

        // temp : 특징 평탄화(테스트용)
        std::map<std::string, std::vector<float>> flattenFeature(
                const std::vector<FullFeatures> &allSegmentFeatures
        );

        // 입력 텐서 생성
        std::vector<Ort::Value> createInputTensors(
                const std::vector<FullFeatures> &allSegmentFeatures
        );

        // 모델 추론
        std::vector<std::vector<float>> runInference(
                const std::vector<Ort::Value> &inputTensors,
                const std::string &outputName
        );

        // L2 정규화
        void l2Normalize(std::vector<float>& vec);

        // 평균 구하기
        std::vector<float> meanPooling(const std::vector<std::vector<float>> &embeddings);

    private:
        void initEssentia();
        void shutdownEssentia();
        bool essentiaInitialized = false;
        void performHPSS(const std::vector<float>& audio, std::vector<float>& y_h, std::vector<float>& y_p);

        // ONNX 텐서 데이터를 저장할 멤버 변수
        std::vector<float> m_mel_buffer;
        std::vector<float> m_chr_buffer;
        std::vector<float> m_tmp_buffer;

        Ort::Env ort_env;
        std::unique_ptr<Ort::Session> ort_session = nullptr;
    };
}
#endif // NDK_ESSENTIA_TEST__HELPER_H