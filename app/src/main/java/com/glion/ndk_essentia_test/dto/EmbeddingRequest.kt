package com.glion.ndk_essentia_test.dto

/**
 * Project : Resonance
 * File : EmbeddingRequest
 * Created by glion on 2025-11-11
 *
 * Description:
 * - 최종 임베딩 코사인 유사도 검증 API Request
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
data class EmbeddingRequest(
    val appEmbedding: FloatArray
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as EmbeddingRequest

        return appEmbedding.contentEquals(other.appEmbedding)
    }

    override fun hashCode(): Int {
        return appEmbedding.contentHashCode()
    }
}
