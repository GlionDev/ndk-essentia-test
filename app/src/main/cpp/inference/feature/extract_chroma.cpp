//
// Created by glion on 2025-11-04.
// 한 세그먼트에 대해 Chromagram 추출
//

#include "embedding_helper.h"
#include <pool.h>
#include <algorithmfactory.h>
#include <vector>
#include <string>
#include <memory>
#include <essentiamath.h>
#include <essentiautil.h>
#include <complex>

using namespace NdkEssentiaEmbedding;
using namespace essentia;
using namespace essentia::standard;

using EssentiaComplex = std::complex<Real>;

std::vector<std::vector<float>> EmbeddingHelper::computeChroma(
        const std::vector<float> &audio,
        const EmbeddingConfig &config
) {
    // Chroma 추출 시간 측정
    RunTimerLogger timer("Extract Chroma");
    // 1. 파라미터 (Librosa CQT와 유사성을 위해 8192 권장)
    int frameSize = 8192;
    int hopSize = 512;
    int sampleRate = config.sr;
    int chromaBins = config.chroma_bins; // 12

    // 2. Essentia 알고리즘 생성
    AlgorithmFactory& factory = AlgorithmFactory::instance();
    Algorithm* windowing = factory.create("Windowing", "type", "hann", "size", frameSize);
    Algorithm* spectrum = factory.create("Spectrum", "size", frameSize);

    std::vector<Real> frame(frameSize);
    std::vector<Real> windowedFrame;
    std::vector<Real> spectrumData;

    windowing->input("frame").set(frame);
    windowing->output("frame").set(windowedFrame);
    spectrum->input("frame").set(windowedFrame);
    spectrum->output("spectrum").set(spectrumData);

    // 3. Gaussian Filter Bank 미리 계산
    int spectrumSize = frameSize / 2 + 1;
    std::vector<std::vector<float>> filterBank(chromaBins, std::vector<float>(spectrumSize, 0.0f));

    float freqResolution = (float)sampleRate / frameSize;
    float refFreq = 440.0f;
    float width = 1.0f; // Librosa 기본값과 유사한 확산 정도

    for (int bin = 0; bin < spectrumSize; ++bin) {
        float freq = bin * freqResolution;
        if (freq < 32.7f) continue; // C1 미만 무시

        float midiNote = 69 + 12 * std::log2(freq / refFreq);

        for (int k = 0; k < chromaBins; ++k) {
            float dist = std::fmod(midiNote - k, 12.0f);
            if (dist < -6.0f) dist += 12.0f;
            if (dist > 6.0f) dist -= 12.0f;

            float weight = std::exp(-0.5f * std::pow(dist / width, 2));

            // 가우시안 필터 적용 (Thresholding으로 속도 최적화)
            if (weight > 0.01f) {
                filterBank[k][bin] += weight;
            }
        }
    }

    // 4. 처리 루프
    std::vector<std::vector<float>> chromagram(chromaBins);
    int estimatedFrames = (audio.size() - frameSize) / hopSize + 1;
    if (estimatedFrames > 0) {
        for (auto& row : chromagram) row.reserve(estimatedFrames);
    }

    if (audio.size() >= frameSize) {
        for (size_t i = 0; i + frameSize <= audio.size(); i += hopSize) {

            // A. FFT 수행
            std::copy(audio.begin() + i, audio.begin() + i + frameSize, frame.begin());
            windowing->compute();
            spectrum->compute();

            // B. Filter Bank 적용 (Spectrum -> Chroma)
            std::vector<float> currentFrame(chromaBins, 0.0f);
            float maxVal = 0.0f; // 정규화를 위한 최댓값 찾기

            for (int k = 0; k < chromaBins; ++k) {
                float energy = 0.0f;
                const std::vector<float>& filters = filterBank[k];

                for (int bin = 0; bin < spectrumSize; ++bin) {
                    if (filters[bin] > 0.0f) {
                        energy += filters[bin] * spectrumData[bin];
                    }
                }
                currentFrame[k] = energy;

                // 최댓값 갱신
                if (energy > maxVal) maxVal = energy;
            }

            // C. [핵심] Max Normalization (Librosa 기본 동작 구현)
            // 최댓값으로 나누어 0~1 사이로 스케일링
            if (maxVal < 1e-9f) maxVal = 1.0f; // 0 나누기 방지

            for (int k = 0; k < chromaBins; ++k) {
                chromagram[k].push_back(currentFrame[k] / maxVal);
            }
        }
    }

    delete windowing;
    delete spectrum;

    return chromagram;
}