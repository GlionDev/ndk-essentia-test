//
// Created by glion on 2025-11-04.
// 한 세그먼트에 대해 Log-Mel Spectrogram 추출
//

#include "embedding_helper.h"
#include <pool.h>
#include <essentia.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithmfactory.h>

using namespace NdkEssentiaEmbedding;
using namespace essentia;
using namespace essentia::standard;

std::vector<std::vector<float>> EmbeddingHelper::computeLogMel(
        const std::vector<float> &audio,
        const EmbeddingConfig &config
) {
    // LogMel 추출 시간 측정
    RunTimerLogger timer("Extract LogMel");

    // --- 1. 파라미터 계산 및 패딩 ---

    // Librosa 기본값
    const int n_fft = 2048;
    const int pad_width = n_fft / 2; // 1024

    // Librosa 'center=True' 모방 (제로 패딩)
    std::vector<Real> padded_audio(pad_width, 0.0f);
    padded_audio.insert(padded_audio.end(), audio.begin(), audio.end());
    // resize를 사용하여 후면 패딩 추가
    padded_audio.resize(padded_audio.size() + pad_width, 0.0f);

    // Python의 int() (절삭)와 일치시키기 위해 static_cast<int> 사용
    int hopLength = std::max(1, static_cast<int>(config.sr * config.mel_hop_ms / 1000.0f));

    AlgorithmFactory &factory = AlgorithmFactory::instance();

    // --- 2. Essentia 알고리즘 정의 (스마트 포인터 사용) ---

    // FrameCutter: 오디오를 프레임으로 자름
    std::unique_ptr<Algorithm> frameCutter(factory.create("FrameCutter",
                                                          "frameSize", n_fft,
                                                          "hopSize", hopLength,
                                                          "lastFrameToEndOfFile",false, // Librosa와 일치
                                                          "startFromZero", true
    ));

    // Windowing: Hann 윈도우 적용
    std::unique_ptr<Algorithm> windowing(factory.create("Windowing",
                                                        "type", "hann"
    ));

    // PowerSpectrum: Librosa 'power=2.0'과 일치
    std::unique_ptr<Algorithm> spectrum(factory.create("PowerSpectrum"));

    // MelBands: Mel Filterbank 적용
    std::unique_ptr<Algorithm> melBands(factory.create("MelBands",
                                                       "numberBands", config.mel_n_mels,
                                                       "sampleRate", config.sr,
                                                       "inputSize", n_fft / 2 + 1, // 1025
                                                       "lowFrequencyBound", 0.0f,
                                                       "highFrequencyBound", config.sr / 2.0f,
                                                       "normalize","unit_sum", // Librosa 'norm='slaney''
                                                       "warpingFormula","slaneyMel" // Librosa 'slaneyMel'
    ));

    // --- 3. 출력 벡터 [M][T] 준비 ---
    size_t M = config.mel_n_mels;
    std::vector<std::vector<float>> melSpectrogram(M);

    // 성능 최적화: T의 길이를 추정하여 미리 공간 할당
    size_t estimated_T = padded_audio.size() / hopLength + 1;
    for (size_t i = 0; i < M; ++i) {
        melSpectrogram[i].reserve(estimated_T);
    }

    // --- 4. 임시 버퍼 ---
    std::vector<Real> frame, windowedFrame, spec, melBandsOut;

    // --- 5. 알고리즘 파이프라인 수동 설정 (루프용) ---
    frameCutter->input("signal").set(padded_audio); // 패딩된 오디오 사용
    frameCutter->output("frame").set(frame);

    windowing->input("frame").set(frame);
    windowing->output("frame").set(windowedFrame);

    spectrum->input("signal").set(windowedFrame);
    spectrum->output("powerSpectrum").set(spec);

    melBands->input("spectrum").set(spec);
    melBands->output("bands").set(melBandsOut);

    // --- 6. 프레임 단위 계산 루프 ---
    while (true) {
        // FrameCutter 실행
        frameCutter->compute();

        // 프레임이 비어있으면(신호의 끝) 루프 종료
        if (frame.empty()) {
            break;
        }

        // Windowing -> PowerSpectrum -> MelBands 순차 실행
        windowing->compute();
        spectrum->compute();
        melBands->compute();

        // --- 7. 수동 PowerToDB 및 [M][T] 저장 ---
        // (Python: librosa.power_to_db(S + 1e-10))
        for (int m_idx = 0; m_idx < M; ++m_idx) {

            // 1. 음수 값을 0.0f로 클리핑(clipping) (PowerSpectrum 결과는 이론상 음수가 없지만 안전장치)
            float value = std::max(0.0f, melBandsOut[m_idx]);

            // 2. 0 또는 양수 값에 Epsilon(1e-10)을 더하고 로그 계산
            // (Python의 `S + 1e-10` 로직과 정확히 일치)
            float logMelValue = 10.0f * std::log10(value + 1e-10f);

            // [M][T] 형식으로 저장
            melSpectrogram[m_idx].push_back(logMelValue);
        }
    }

    // 스마트 포인터를 사용하므로 수동 delete가 필요 없습니다.

    return melSpectrogram;
}