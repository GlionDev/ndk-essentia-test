//
// Created by glion on 2025-10-31.
// 오디오 데이터 구조체
//

#ifndef NDK_ESSENTIA_TEST_DATA_H
#define NDK_ESSENTIA_TEST_DATA_H

#include <vector>

/**
 * 로드된 오디오 데이터와 메타 정보를 담는 구조체
 */
struct AudioData {
    // 리샘플링된 오디오 샘플 (float32)
    std::vector<float> samples;

    // 최종 샘플 레이트 (Hz)
    float sampleRate = 0;

    // 최종 채널 수 (모노: 1, 스테레오: 2)
    int numChannels = 0;

    // 기본 생성자
    AudioData() = default;
};

#endif //NDK_ESSENTIA_TEST_DATA_H
