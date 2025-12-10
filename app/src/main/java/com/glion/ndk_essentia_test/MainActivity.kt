package com.glion.ndk_essentia_test

import android.content.Context
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.glion.ndk_essentia_test.databinding.ActivityMainBinding
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import kotlin.system.measureNanoTime

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var mContext: Context

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        mContext = this

        val jni = InferenceJniBridge()

        binding.btnStart.setOnClickListener {
            // 오디오 파일 읽기
            val audioSamplePath = getPathFromAssets("sample.mp3")

            // 모델 읽기
            val modelPath = getPathFromAssets("model.onnx")
            getPathFromAssets("model.onnx.data")

            val elapsed = measureNanoTime  {
                val embedding = jni.allInferencePipeline(audioSamplePath, modelPath)!!
                if(embedding.isEmpty()) Log.e("glion", "임베딩 얻기 실패. 사이즈가 0")
            }
            Log.d("glion", "총 소요시간 :: ${elapsed / 1_000_000} ms")
        }
    }

    private fun getPathFromAssets(assetName: String): String {
        val file = File(mContext.filesDir, assetName)
        if (!file.exists()) {
            try {
                mContext.assets.open(assetName).use { input ->
                    FileOutputStream(file).use { output ->
                        input.copyTo(output)
                    }
                }
            } catch (e: IOException) {
                throw RuntimeException("Failed to copy asset: $assetName", e)
            }
        }
        return file.absolutePath
    }
}