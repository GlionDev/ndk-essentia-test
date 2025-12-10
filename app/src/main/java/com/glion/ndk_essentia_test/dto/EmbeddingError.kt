package com.glion.ndk_essentia_test.dto

import com.google.gson.annotations.SerializedName

/**
 * Project : Resonance
 * File : EmbeddingError
 * Created by glion on 2025-11-11
 *
 * Description:
 * - 코사인 유사도 검증 도중 오류 발생 시 Data
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
data class EmbeddingError(
    @SerializedName("app_dim")
    val appDim: Int,
    @SerializedName("server_dim")
    val serverDim: Int
)
