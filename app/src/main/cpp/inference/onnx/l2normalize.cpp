//
// Created by glion on 2025-11-07.
// L2 정규화 수행
//

#include "embedding_helper.h"
#include <cmath>     // std::sqrt
#include <numeric>   // std::accumulate

using namespace NdkEssentiaEmbedding;

void EmbeddingHelper::l2Normalize(std::vector<float> &vec) {
    RunTimerLogger timer("l2Normalize");
    const float epsilon = 1e-12f; // 0으로 나누기 방지를 위한 epsilon

    // 1. L2 norm (크기) 계산: sqrt(v[0]^2 + v[1]^2 + ...)
    float norm_sq = 0.0f; // norm의 제곱을 먼저 계산
    for (float val : vec) {
        norm_sq += val * val;
    }
    float norm = std::sqrt(norm_sq);

    // 2. 역수 계산 (나눗셈 대신 곱셈 사용)
    float inv_norm = 1.0f / (norm + epsilon);

    // 3. 벡터를 정규화
    for (float& val : vec) {
        val *= inv_norm;
    }
}