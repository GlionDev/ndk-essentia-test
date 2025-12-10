package com.glion.ndk_essentia_test.dto

/**
 * Project : Resonance
 * File : SimilarityResult
 * Created by glion on 2025-11-11
 *
 * Description:
 * - 코사인 유사도 검증 완료 시 상세 결과 Data
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
data class SimilarityResult(
    val cosineSimilarity: Double,
    val isAboveThreshold: Boolean
)
