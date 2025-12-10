//
// Created by glion on 2025-11-05.
// 기본 Embedding 설정 구조체
//

#ifndef NDK_ESSENTIA_TEST_EMBEDDING_CONFIG_H
#define NDK_ESSENTIA_TEST_EMBEDDING_CONFIG_H

struct EmbeddingConfig {
    int sr = 44100;
    bool isMono = true;
    int mel_n_mels = 128;
    float mel_hop_ms = 25.0f;
    int chroma_bins = 12;
    int tempo_win = 160;
    float seg_seconds = 18.6f;
    float hop_seconds = 6.4f;
    int segments_per_song = 3;
    bool use_hpss = false;
};

#endif //NDK_ESSENTIA_TEST_EMBEDDING_CONFIG_H
