//
// Created by glion on 2025-11-06.
// 입력 텐서 생성
//

#include "embedding_helper.h"

using namespace NdkEssentiaEmbedding;

std::vector<Ort::Value> EmbeddingHelper::createInputTensors(
        const std::vector<FullFeatures>& allSegmentFeatures
        ) {
    // 텐서 생성 시간 측정
    RunTimerLogger timer("createInputTensors");

    size_t V = allSegmentFeatures.size(); // V: 배치 크기 (세그먼트 수)
    if (V == 0) {
        return {}; // 데이터 없음
    }

    // 첫 번째 세그먼트의 특징을 기준으로 텐서의 크기(Shape)를 결정합니다.
    const FullFeatures& firstFeatures = allSegmentFeatures[0];

    // Mel 특징 크기: [M, T]
    const auto& melFeature = firstFeatures.at("mel");
    size_t M = melFeature.size();
    size_t T = melFeature[0].size();

    // Chroma 특징 크기: [C, T]
    const auto& chrFeature = firstFeatures.at("chroma");
    size_t C = chrFeature.size();

    // Tempo 특징 크기: [1, L]
    const auto& tmpFeature = firstFeatures.at("tempo");
    size_t L = tmpFeature[0].size();

    // [수정 1] 로컬 벡터 대신 멤버 변수의 크기를 조절(resize)합니다.
    // 이 메모리는 EmbeddingHelper 객체가 살아있는 동안 유지됩니다.
    m_mel_buffer.resize(V * M * T); // [V, M, T]
    m_chr_buffer.resize(V * C * T); // [V, C, T]
    m_tmp_buffer.resize(V * L);     // [V, L]

    // 데이터 복사 (배치 차원 [V]으로 쌓기)
    for (size_t v = 0; v < V; ++v) {
        const FullFeatures& features = allSegmentFeatures[v];

        // --- Mel 텐서 복사 ---
        const auto& currentMel = features.at("mel");
        for (size_t m = 0; m < M; ++m) {
            for (size_t t = 0; t < T; ++t) {
                size_t index = v * (M * T) + m * T + t;
                m_mel_buffer[index] = currentMel[m][t]; // 🟢 멤버 변수에 저장
            }
        }

        // --- Chroma 텐서 복사 ---
        const auto& currentChr = features.at("chroma");
        for (size_t c = 0; c < C; ++c) {
            for (size_t t = 0; t < T; ++t) {
                size_t index = v * (C * T) + c * T + t;
                m_chr_buffer[index] = currentChr[c][t]; // 🟢 멤버 변수에 저장
            }
        }

        // --- Tempo 텐서 복사 ---
        const auto& currentTmp = features.at("tempo");
        for (size_t l = 0; l < L; ++l) {
            size_t index = v * L + l;
            m_tmp_buffer[index] = currentTmp[0][l]; // 🟢 멤버 변수에 저장
        }
    }

    // ONNX 텐서 생성을 위한 메모리 정보
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
            OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<Ort::Value> input_tensors;
    input_tensors.reserve(3); // 3개의 텐서를 담을 공간 미리 할당

    int64_t V_64 = (int64_t)V;

    // --- 1. Mel 텐서 생성 ---
    std::vector<int64_t> mel_shape = {V_64, (int64_t)M, (int64_t)T};
    input_tensors.push_back(Ort::Value::CreateTensor<float>(
            memory_info,
            m_mel_buffer.data(), // [수정 2] 멤버 변수의 메모리 주소 사용
            m_mel_buffer.size(),
            mel_shape.data(),
            mel_shape.size()
    ));

    // --- 2. Chroma 텐서 생성 ---
    std::vector<int64_t> chr_shape = {V_64, (int64_t)C, (int64_t)T};
    input_tensors.push_back(Ort::Value::CreateTensor<float>(
            memory_info,
            m_chr_buffer.data(), // [수정 2] 멤버 변수의 메모리 주소 사용
            m_chr_buffer.size(),
            chr_shape.data(),
            chr_shape.size()
    ));

    // --- 3. Tempo 텐서 생성 ---
    std::vector<int64_t> tmp_shape = {V_64, (int64_t)L};
    input_tensors.push_back(Ort::Value::CreateTensor<float>(
            memory_info,
            m_tmp_buffer.data(), // [수정 2] 멤버 변수의 메모리 주소 사용
            m_tmp_buffer.size(),
            tmp_shape.data(),
            tmp_shape.size()
    ));

    return input_tensors;
}