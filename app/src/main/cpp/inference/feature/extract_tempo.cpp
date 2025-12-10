//
// Created by glion on 2025-11-05.
// 한 세그먼트에 대해 Tempogram 추출
//

#include "embedding_helper.h"
#include <pool.h>
#include <essentia.h>
#include <cmath>
#include <vector>
#include <algorithmfactory.h>
#include <numeric>
#include <algorithm>

using namespace NdkEssentiaEmbedding;
using namespace essentia;
using namespace essentia::standard;

using EssentiaComplex = std::complex<Real>;

std::vector<float> EmbeddingHelper::computeTempo(
        const std::vector<float> &audio,
        const EmbeddingConfig &config
) {
    // Tempo 시간 측정
    RunTimerLogger timer("Extract Tempo");

    Real sampleRate = static_cast<Real>(config.sr);
    int tempoWin = config.tempo_win; // Python의 tempo_win (160)

    // [T-Align] 홉 길이 (LogMel과 동일)
    int hopLength = std::max(1, static_cast<int>(sampleRate * config.mel_hop_ms / 1000.0f));

    AlgorithmFactory &factory = AlgorithmFactory::instance();

    // --- 1. Onset Novelty Curve 생성 (Python의 onset_env) ---
    // (이 로직은 이전과 동일하며, 올바르게 동작합니다)

    const int frameSize = 2048;
    const int pad_width = frameSize / 2;
    std::vector<Real> padded_audio(pad_width, 0.0f);
    padded_audio.insert(padded_audio.end(), audio.begin(), audio.end());
    padded_audio.resize(padded_audio.size() + pad_width, 0.0f);

    std::unique_ptr<Algorithm> onsetFrameCutter(
            factory.create("FrameCutter",
                           "frameSize", frameSize, "hopSize", hopLength,
                           "lastFrameToEndOfFile", false, "startFromZero", true));
    std::unique_ptr<Algorithm> windowing(
            factory.create("Windowing", "type", "hann", "size", frameSize));
    std::unique_ptr<Algorithm> fft(factory.create("FFT"));
    std::unique_ptr<Algorithm> onsetDetection(
            factory.create("OnsetDetection", "method", "melflux", "sampleRate", sampleRate));

    std::vector<Real> frame, windowedFrame;
    std::vector<EssentiaComplex> complexSpectrum;
    std::vector<Real> magnitudeSpectrum, phaseSpectrum;
    Real currentOnsetStrength = 0.0;
    std::vector<Real> onsetNoveltyCurve; // 1D Onset Envelope [T]

    onsetFrameCutter->input("signal").set(padded_audio);
    onsetFrameCutter->output("frame").set(frame);
    windowing->input("frame").set(frame);
    windowing->output("frame").set(windowedFrame);
    fft->input("frame").set(windowedFrame);
    fft->output("fft").set(complexSpectrum);
    onsetDetection->input("spectrum").set(magnitudeSpectrum);
    onsetDetection->input("phase").set(phaseSpectrum);
    onsetDetection->output("onsetDetection").set(currentOnsetStrength);

    while (true) {
        onsetFrameCutter->compute();
        if (frame.empty()) break;
        windowing->compute();
        fft->compute();
        if (complexSpectrum.empty()) continue;

        magnitudeSpectrum.resize(complexSpectrum.size());
        std::transform(complexSpectrum.begin(), complexSpectrum.end(),
                       magnitudeSpectrum.begin(),
                       [](const EssentiaComplex& c){ return std::abs(c); });
        phaseSpectrum.resize(complexSpectrum.size());
        std::transform(complexSpectrum.begin(), complexSpectrum.end(),
                       phaseSpectrum.begin(),
                       [](const EssentiaComplex& c){ return std::arg(c); });

        onsetDetection->compute();
        onsetNoveltyCurve.push_back(currentOnsetStrength);
    }

    // --- 2. [수정] 수동 Tempogram 계산 (Windowed Auto-Correlation) ---
    // (Python: T = librosa.feature.tempogram(onset_envelope=onset_env, ...))

    // Librosa의 'tempogram' 기본 win_length
    const int num_lags = 384; // (Librosa 'tempogram' 기본 win_length)
    // Librosa의 'tempogram' 기본 hop_length (1)
    const int tempogram_hop_length = 1;

    // 2.1 1D 'onsetNoveltyCurve'를 자르기 위한 두 번째 FrameCutter
    std::unique_ptr<Algorithm> tempoFrameCutter(
            factory.create("FrameCutter",
                           "frameSize", num_lags, // 384
                           "hopSize", tempogram_hop_length,   // 1
                           "lastFrameToEndOfFile", false,
                           "startFromZero", true
            ));

    // 2.2 자기상관(Autocorrelation) 알고리즘
    std::unique_ptr<Algorithm> autoCorrelationAlgo(factory.create("AutoCorrelation"));

    // 버퍼
    std::vector<Real> onset_frame;      // 1D 온셋 프레임 [384]
    std::vector<Real> autocorr_vec;   // 1D 자기상관 결과 [384]
    std::vector<std::vector<Real>> tempogram2D; // 2D 템포그램 [384][Time]

    // (성능 최적화: 2D 벡터 미리 할당)
    size_t estimated_T = onsetNoveltyCurve.size() / tempogram_hop_length + 1;
    tempogram2D.reserve(estimated_T);

    // 알고리즘 연결
    tempoFrameCutter->input("signal").set(onsetNoveltyCurve);
    tempoFrameCutter->output("frame").set(onset_frame);

    autoCorrelationAlgo->input("array").set(onset_frame);
    autoCorrelationAlgo->output("autoCorrelation").set(autocorr_vec);

    // 2.3 Windowed Auto-Correlation 루프
    while (true) {
        tempoFrameCutter->compute();
        if (onset_frame.empty()) {
            break;
        }

        autoCorrelationAlgo->compute();

        // 2D 템포그램 생성
        tempogram2D.push_back(autocorr_vec);
    }

    // --- 3. 2D 템포그램을 1D로 평균 (Python: T.mean(axis=1)) ---
    std::vector<float> tempo_histogram_1d(num_lags, 0.0f); // 1D 템포 벡터 [384] (float으로 바로 선언)

    size_t num_frames = tempogram2D.size(); // T (시간 축)

    if (num_frames > 0) {
        // 3.1 모든 프레임의 값을 'lag'별로 더합니다 (열(column) 합계).
        for (size_t t = 0; t < num_frames; ++t) {
            // (안전 장치: 모든 autocorr_vec의 크기가 num_lags(384)라고 가정)
            if (tempogram2D[t].size() == num_lags) {
                for (int l = 0; l < num_lags; ++l) {
                    tempo_histogram_1d[l] += tempogram2D[t][l];
                }
            }
        }

        // 3.2 합계를 프레임 수(T)로 나누어 평균을 구합니다.
        for (int l = 0; l < num_lags; ++l) {
            tempo_histogram_1d[l] /= num_frames;
        }
    }


    // --- 4. 패딩/슬라이싱 (Python 로직과 100% 동일) ---
    // (이 로직은 이전 코드에서도 완벽했습니다)

    std::vector<float> finalTempoVector;

    if (tempo_histogram_1d.empty()) {
        finalTempoVector.assign(tempoWin, 0.0f); // 실패 시 0-벡터
        return finalTempoVector;
    }

    // Librosa 로직: [384] -> [160] (자르기)
    if (tempo_histogram_1d.size() >= tempoWin) {
        finalTempoVector.assign(tempo_histogram_1d.begin(), tempo_histogram_1d.begin() + tempoWin);
    } else {
        // (이 케이스는 librosa 기본값(384 > 160) 하에서는 거의 발생하지 않음)
        finalTempoVector.assign(tempo_histogram_1d.begin(), tempo_histogram_1d.end());
        finalTempoVector.resize(tempoWin, 0.0f);
    }

    return finalTempoVector;
}