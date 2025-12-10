package com.glion.ndk_essentia_test

import com.glion.ndk_essentia_test.dto.BaseResponse
import com.glion.ndk_essentia_test.dto.CosineSimilarityRequest
import com.glion.ndk_essentia_test.dto.EmbeddingError
import com.glion.ndk_essentia_test.dto.EmbeddingRequest
import com.glion.ndk_essentia_test.dto.SimilarityResult
import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.POST

/**
 * Project : Resonance
 * File : Service
 * Created by glion on 2025-11-11
 *
 * Description:
 * - Api Service 명시
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
interface Service {
    @POST("embeddingCompare")
    suspend fun embeddingCompare(
        @Body body: EmbeddingRequest
    ) : Response<BaseResponse<SimilarityResult, EmbeddingError>>

    /**
     * temp : 테스트 - 특정 특징 추출하여 코사인 유사도 비교용
     */
    @POST("cosineSimilarity")
    suspend fun cosineSimilarity(
        @Body body: CosineSimilarityRequest
    ) : Response<BaseResponse<SimilarityResult, EmbeddingError>>
}