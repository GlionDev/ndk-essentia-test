package com.glion.ndk_essentia_test

import com.glion.ndk_essentia_test.interceptor.logInterceptor
import okhttp3.OkHttpClient
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

/**
 * Project : ndk-essentia-test
 * File : NetworkObject
 * Created by glion on 2025-12-10
 *
 * Description:
 * - 추후 기입
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
object NetworkObject {
    fun getService() : Service {
        val okhttpClient = OkHttpClient.Builder()
            .addNetworkInterceptor(logInterceptor)
            .build()

        return Retrofit.Builder()
            .baseUrl("http://localhost:8080")
            .client(okhttpClient)
            .addConverterFactory(GsonConverterFactory.create())
            .build()
            .create(Service::class.java)
    }
}