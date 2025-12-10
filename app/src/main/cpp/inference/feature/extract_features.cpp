//
// Created by Admin on 2025-11-05.
// 한 세그먼트에 대해 전체 특징 추출(Log-Mel Spectrogram, Chromagram, Tempogram)
//

#include "embedding_helper.h"
#include <stdexcept>
#include <algorithm> // std::copy 사용

using namespace NdkEssentiaEmbedding;

FullFeatures EmbeddingHelper::extractFeatures(
        const std::vector<float>& audio,
        const EmbeddingConfig& config
) {
    // 시간 측정
    RunTimerLogger timer("extractFeatures Function");

    // Python 코드의 반환 형태와 동일하게 구성 (key: 특징 이름, value: 특징 데이터)
    FullFeatures features;

    // --- 1. HPSS 분리 신호 준비 (y_h, y_p) ---
    std::vector<float> y_h; // Harmonic (크로마 추출용)
    std::vector<float> y_p; // Percussive (템포 추출용)

    if (config.use_hpss) {
        // HPSS 수행
        performHPSS(audio, y_h, y_p);
    } else {
        // HPSS를 사용하지 않으면 원본 신호를 복사
        y_h = audio;
        y_p = audio;
    }

    // --- 2. Log-Mel 추출 ---
    // (원본 오디오 y를 사용)
    std::vector<std::vector<float>> mel = computeLogMel(audio, config);
    if (!mel.empty()) {
        features["mel"] = std::move(mel);
    } else {
        LOGW("mel 이 비어있음");
    }

    // --- 3. Chroma CQT 추출 ---
    // (고조파 신호 y_h를 사용)
    std::vector<std::vector<float>> chroma = computeChroma(y_h, config);
    if (!chroma.empty()) {
        features["chroma"] = std::move(chroma);
    } else {
        LOGW("chroma 가 비어있음");
    }

    // --- 4. Tempo Vector 추출 ---
    // (타악기 신호 y_p를 사용)
    std::vector<float> tempo_vec = computeTempo(y_p, config);

    // 1D 특징(Tempo)을 FullFeatures 타입(2D: [1][L])으로 변환하여 저장합니다.
    if (!tempo_vec.empty()) {
        // **핵심 수정:** std::vector<std::vector<float>>를 명시적으로 생성하여 감쌉니다.
        std::vector<std::vector<float>> tempo_2d;
        tempo_2d.reserve(1); // 단일 행만 가짐
        tempo_2d.push_back(std::move(tempo_vec)); // 1D 벡터를 2D 벡터의 첫 번째 행으로 이동

        features["tempo"] = std::move(tempo_2d);
    } else {
        LOGW("tempo 가 비어있음");
    }

    return features;
}