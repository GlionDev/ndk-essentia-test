package com.glion.ndk_essentia_test.dto

/**
 * Project : Resonance
 * File : BaseResponse
 * Created by glion on 2025-11-11
 *
 * Description:
 * - 유사도 검증 용도 API Response 기본 구조
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
data class BaseResponse<T, E>(
    val status: String,
    val message: String,
    val result: T?, // 성공 시 데이터
    val error: E?   // 오류 시 데이터
)
