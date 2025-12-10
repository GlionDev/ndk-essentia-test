//
// Created by glion on 2025-11-05.
// hpss 수행
//

#include "embedding_helper.h"
#include <algorithmfactory.h>
#include <pool.h>
#include <cmath>
#include <stdexcept>
#include <algorithm> // std::copy 사용

using namespace NdkEssentiaEmbedding;
using namespace essentia;
using namespace essentia::standard;

/**
 * @brief Essentia를 사용하여 HPSS를 수행.
 * @param audio 입력 오디오
 * @param y_h 고조파(Harmonic) 신호 출력
 * @param y_p 타악기(Percussive) 신호 출력
 */
void EmbeddingHelper::performHPSS(
        const std::vector<float>& audio,
        std::vector<float>& y_h,
        std::vector<float>& y_p
) {
    AlgorithmFactory& factory = AlgorithmFactory::instance();

    // [수정 1] 스마트 포인터(unique_ptr)를 사용하여 자동 메모리 관리
    std::unique_ptr<Algorithm> hpss(factory.create("HPSS"));

    std::vector<Real> audio_real(audio.begin(), audio.end());
    std::vector<Real> y_h_real, y_p_real;

    hpss->input("signal").set(audio_real);
    hpss->output("harmonic").set(y_h_real);
    hpss->output("percussive").set(y_p_real);

    try {
        hpss->compute();

        y_h.assign(y_h_real.begin(), y_h_real.end());
        y_p.assign(y_p_real.begin(), y_p_real.end());

    } catch (const EssentiaException& e) {
        // 오류 발생 시 원본 신호를 H와 P로 반환 (Python의 except 블록 처리)
        y_h = audio;
        y_p = audio;
    }
}