package com.glion.ndk_essentia_test.interceptor

import okhttp3.logging.HttpLoggingInterceptor

/**
 * Project : Resonance
 * File : logInterceptor
 * Created by glion on 2025-11-11
 *
 * Description:
 * - API 통신 시 로그 남기는 Interceptor
 *
 * Copyright @2025 Gangglion. All rights reserved
 */
val logInterceptor = HttpLoggingInterceptor(PrettyJsonLogger()).apply {
    level = /*if(BuildConfig.DEBUG) HttpLoggingInterceptor.Level.BODY else HttpLoggingInterceptor.Level.NONE*/ HttpLoggingInterceptor.Level.BASIC
}