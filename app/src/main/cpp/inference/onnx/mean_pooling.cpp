//
// Created by glion on 2025-11-07.
// 평균 구하기
//

#include "embedding_helper.h"

using namespace NdkEssentiaEmbedding;

std::vector<float> EmbeddingHelper::meanPooling(const std::vector<std::vector<float>> &embeddings) {

    RunTimerLogger timer("meanPooling");

    if (embeddings.empty() || embeddings[0].empty()) {
        LOGW("Cannot perform mean pooling on empty embeddings.");
        return {};
    }

    size_t V = embeddings.size(); // 세그먼트 수 (V)
    size_t D = embeddings[0].size(); // 임베딩 차원 (D)

    // [D] 크기의 벡터를 0으로 초기화
    std::vector<float> meanEmbedding(D, 0.0f);

    // 1. 모든 벡터를 합산
    for (const auto& vec : embeddings) {
        for (size_t i = 0; i < D; ++i) {
            meanEmbedding[i] += vec[i];
        }
    }

    // 2. 세그먼트 수(V)로 나누어 평균 계산
    float num_vectors_float = static_cast<float>(V);
    for (size_t i = 0; i < D; ++i) {
        meanEmbedding[i] /= num_vectors_float;
    }

    return meanEmbedding; // [D] 크기의 평균 벡터 반환
}