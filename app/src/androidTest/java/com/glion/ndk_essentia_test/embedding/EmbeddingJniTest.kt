package com.glion.ndk_essentia_test.embedding

import android.content.Context
import android.util.Log
import androidx.test.core.app.ApplicationProvider
import com.glion.ndk_essentia_test.InferenceJniBridge
import kotlinx.coroutines.test.runTest
import org.junit.After
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.io.FileOutputStream

/**
 * Project : Resonance
 * File : EmbeddingJniTest
 * Created by glion on 2025-11-10
 *
 * Description:
 * - JNI 를 통한 임베딩 테스트
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
class EmbeddingJniTest {

    @After
    fun teardown() {
        // 캐시저장소 정리
        val context = ApplicationProvider.getApplicationContext<Context>()
        context.cacheDir.deleteRecursively()
    }

    @Test
    fun runInference_getEmbedding() = runTest {
        val context = ApplicationProvider.getApplicationContext<Context>()
        // 에셋의 오디오 파일 캐시 저장소로 복사
        val audioName = "sample.mp3"
        val cacheAudioFile = File(context.cacheDir, audioName)
        context.assets.open(audioName).use { input ->
            FileOutputStream(cacheAudioFile).use { output ->
                input.copyTo(output)
            }
        }

        // 에셋의 모델파일 캐시 저장소로 복사
        val modelName = "model.onnx"
        val cachedModelFile = File(context.cacheDir, modelName)
        context.assets.open(modelName).use { input ->
            FileOutputStream(cachedModelFile).use { output ->
                input.copyTo(output)
            }
        }
        val modelDataName = "model.onnx.data"
        context.assets.open(modelDataName).use { input ->
            FileOutputStream(File(context.cacheDir, modelDataName)).use { output ->
                input.copyTo(output)
            }
        }

        val audioPath = cacheAudioFile.absolutePath
        val modelPath = cachedModelFile.absolutePath

        // 전체 과정 시간 테스트
        val startTime = System.currentTimeMillis()
        val jniBridge = InferenceJniBridge()
        val embed = jniBridge.allInferencePipeline(audioPath, modelPath)
        Log.v("glion", "Embedding ::\n${embed!!.joinToString(",")}")
        val processTime = System.currentTimeMillis() - startTime
        Log.i("glion", "테스트 코드 임베딩 얻기 소요시간 :: $processTime ms")
        assertTrue(embed.isNotEmpty())
    }
}