# 프로젝트의 CMake 최소 버전
cmake_minimum_required(VERSION 3.22.1)

# 프로젝트 명
project("ndk_essentia_test")

# Essentia, onnxruntime, eigen 경로 설정
set(ESSENTIA_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/essentia)
set(ONNXRUNTIME_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include/onnxruntime)
set(EIGEN_INCLUDE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/include/external/eigen)

# ffmpeg prefab 설정
find_package(ffmpeg REQUIRED CONFIG)

# essentia 정의 및 연결
add_library(essentia SHARED IMPORTED)

set_target_properties(
        essentia
        PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libessentia.so
        INTERFACE_INCLUDE_DIRECTORIES "${ESSENTIA_INCLUDE_DIR}"
)

# onnxruntime 정의 및 연결
add_library(onnxruntime SHARED IMPORTED)
set_target_properties(
        onnxruntime
        PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libonnxruntime.so
        INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
)

# 메인 JNI 라이브러리 정의 - cpp 파일 연결
add_library(inference-jni-bridge SHARED
        ${CMAKE_CURRENT_LIST_DIR}/inference/embedding_helper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/load/audio_loader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/load/audio_segmenter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/load/audio_perform_hpss.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/feature/extract_logmel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/feature/extract_chroma.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/feature/extract_tempo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/feature/extract_features.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/onnx/make_tensor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/onnx/inference.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/onnx/l2normalize.cpp
        ${CMAKE_CURRENT_LIST_DIR}/inference/onnx/mean_pooling.cpp
        # temp : 테스트 - 특정 특징 추출하여 코사인 유사도 비교용
        ${CMAKE_CURRENT_LIST_DIR}/inference/test/flatten_feature.cpp
        inference-jni-bridge.cpp
)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.

# 링크(종속성 연결) - 추가한 cpp 라이브러리를 연결
target_link_libraries(inference-jni-bridge
        # List libraries link to the target library
        ## essentia 연결
        essentia
        # ffmpeg prefab 연결
        ffmpeg::ffmpeg
        # onnx 연결
        onnxruntime
        android
        log)

# 추가 include directory 지정
target_include_directories(
        inference-jni-bridge
        PRIVATE
        ${EIGEN_INCLUDE_ROOT_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/inference
)