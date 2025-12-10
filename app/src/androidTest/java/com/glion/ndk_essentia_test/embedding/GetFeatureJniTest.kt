package com.glion.ndk_essentia_test.embedding

import android.content.Context
import android.util.Log
import androidx.test.core.app.ApplicationProvider
import com.glion.ndk_essentia_test.InferenceJniBridge
import com.glion.ndk_essentia_test.NetworkObject
import com.glion.ndk_essentia_test.dto.CosineSimilarityRequest
import kotlinx.coroutines.test.runTest
import org.junit.After
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.io.FileOutputStream

/**
 * Project : Resonance
 * File : GetFeatureJniTest
 * Created by glion on 2025-11-27
 *
 * Description:
 * temp : 테스트 - JNI 개별 특징 추출(포팅 정확도 확인용)
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
class GetFeatureJniTest {
    enum class FeatureType(val alias: String) {
        LogMel("L"), Chroma("C"), Tempo("T")
    }

    @After
    fun teardown() {
        // 캐시저장소 정리
        val context = ApplicationProvider.getApplicationContext<Context>()
        context.cacheDir.deleteRecursively()
    }

    @Test
    fun runPreprocess_getFeature_cosineSimilarity() = runTest {
        val context = ApplicationProvider.getApplicationContext<Context>()
        // 에셋의 오디오 파일 캐시 저장소로 복사
        val audioName = "sample.mp3"
        val cacheAudioFile = File(context.cacheDir, audioName)
        context.assets.open(audioName).use { input ->
            FileOutputStream(cacheAudioFile).use { output ->
                input.copyTo(output)
            }
        }

        // note : 테스트 타입을 바꿔주기
        val testType = FeatureType.LogMel

        val audioPath = cacheAudioFile.absolutePath
        val startTime = System.currentTimeMillis()
        val jniBridge = InferenceJniBridge()

        val feature = jniBridge.getFlattenFeature(audioPath, testType.alias)!!
        Log.v("glion", "feature's size : ${feature.size}")
        val body = CosineSimilarityRequest(feature, testType.alias)
        val result = NetworkObject.getService().cosineSimilarity(body)
        if(result.isSuccessful) {
            Log.d("glion", "checkCosineSimilarity result ::\n${result.body()}")
        }
        val processTime = System.currentTimeMillis() - startTime
        Log.i("glion", "테스트 코드 ${testType.name} 특징 얻기 소요시간 :: $processTime ms")
        assertTrue(feature!!.isNotEmpty())
    }
}