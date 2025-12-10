//
// Created by glion on 2025-10-31.
// 오디오 디코딩 데이터에 대한 세그먼트 분할
//

#include "embedding_helper.h"
#include <cmath> // std::min 사용
#include <algorithm> // std::min 사용

using namespace NdkEssentiaEmbedding;
std::vector<std::vector<float>> EmbeddingHelper::segmenter(
        const AudioData &audioData,
        const EmbeddingConfig &config
) {
    // 시간 측정
    RunTimerLogger timer("segmenter Function");

    // 1. 샘플 단위로 변환 (Python 'int()'와 동일하게 절삭)
    const int segmentLengthSamples = static_cast<int>(config.seg_seconds * audioData.sampleRate);

    // [수정 3] Python의 max(1, ...) 적용
    const int hopLengthSamples = std::max(1, static_cast<int>(config.hop_seconds * audioData.sampleRate));

    const int totalSamples = audioData.samples.size();

    // Python: 'if not starts:'와 'if segmentLengthSamples <= 0'을 함께 처리
    if (segmentLengthSamples <= 0 || totalSamples == 0) {
        return {};
    }

    // 2. [수정 1] Python 'range(0, max(0, n_samples - length + 1), hop)'와 동일한 로직
    // (1) 모든 'starts' 인덱스부터 생성
    std::vector<int> starts;
    const int lastPossibleStart = std::max(0, totalSamples - segmentLengthSamples);

    for (int s = 0; s <= lastPossibleStart; s += hopLengthSamples) {
        starts.push_back(s);
    }

    // Python 'if not starts: starts = [0]' 로직
    // (오디오가 세그먼트보다 짧은 경우)
    if (starts.empty() && totalSamples > 0) {
        starts.push_back(0);
    }

    // 3. [수정 2] Python 'max_segments' 샘플링 로직
    if (config.segments_per_song > 0 && starts.size() > config.segments_per_song) {
        std::vector<int> sampled_starts;
        double step = static_cast<double>(starts.size()) / config.segments_per_song;

        for (int i = 0; i < config.segments_per_song; ++i) {
            // Python 'int(i * step)'과 동일
            int index = static_cast<int>(i * step);
            sampled_starts.push_back(starts[index]);
        }
        starts = std::move(sampled_starts); // 샘플링된 리스트로 교체
    }

    // 4. (3)에서 확정된 'starts' 리스트를 기반으로 실제 세그먼트 생성
    std::vector<std::vector<float>> segments;
    segments.reserve(starts.size()); // 메모리 미리 할당

    for (int start : starts) {
        // [수정 1] Python의 y[s:e] 슬라이스는 'end'가 길이를 초과해도 알아서 잘라줌
        // C++의 std::min이 그 역할을 정확히 수행함.
        const int end = std::min(start + segmentLengthSamples, totalSamples);

        auto start_it = audioData.samples.begin() + start;
        auto end_it = audioData.samples.begin() + end;

        // Python의 'y[s:e]' 슬라이싱
        segments.emplace_back(start_it, end_it);
    }

    return segments;
}