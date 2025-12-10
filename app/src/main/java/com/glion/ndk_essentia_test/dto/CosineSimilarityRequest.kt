package com.glion.ndk_essentia_test.dto

/**
 * Project : Resonance
 * File : CosineSimilarityRequest
 * Created by glion on 2025-11-27
 *
 * Description:
 * - temp : 코사인 유사도 검사 API Request
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
data class CosineSimilarityRequest(
    val appFeature: FloatArray,
    val type: String
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as CosineSimilarityRequest

        if (!appFeature.contentEquals(other.appFeature)) return false
        if (type != other.type) return false

        return true
    }

    override fun hashCode(): Int {
        var result = appFeature.contentHashCode()
        result = 31 * result + type.hashCode()
        return result
    }
}
