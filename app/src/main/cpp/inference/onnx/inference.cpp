//
// Created by glion on 2025-11-06.
// ONNX 모델 추론
//

#include "embedding_helper.h"
#include <stdexcept>
#include <memory>
#include <algorithm> // std::copy 사용
#include <vector>    // std::vector 사용

using namespace NdkEssentiaEmbedding;

std::vector<std::vector<float>> EmbeddingHelper::runInference(
        const std::vector<Ort::Value>& inputTensors,
        const std::string& outputName
) {
    RunTimerLogger timer("runInference");

    // 1. 세션 유효성 검사
    if (!ort_session) {
        LOGE("ONNX session is not initialized. Call initOrtSession() first.");
        throw std::runtime_error("ONNX session is not initialized.");
    }

    // 2. 입력 노드 이름 가져오기
    // ONNX Runtime의 Run API는 입력 텐서와 함께 입력 노드의 "이름"을 필요로 합니다.
    std::vector<std::string> input_node_names;
    std::vector<const char*> input_names_char;
    Ort::AllocatorWithDefaultOptions allocator;

    size_t num_input_nodes = ort_session->GetInputCount();
    if (num_input_nodes != inputTensors.size()) {
        LOGE("Input tensor count mismatch. Model expects %zu, but %zu were provided.",
             num_input_nodes, inputTensors.size());
        throw std::runtime_error("Input tensor count mismatch.");
    }

    input_node_names.reserve(num_input_nodes);
    input_names_char.reserve(num_input_nodes);

    for (size_t i = 0; i < num_input_nodes; i++) {
        // 세션에서 직접 입력 노드 이름을 할당받아 사용
        auto input_name_ptr = ort_session->GetInputNameAllocated(i, allocator);
        input_node_names.push_back(input_name_ptr.get());
        input_names_char.push_back(input_node_names.back().c_str());
    }

    // 3. 출력 노드 이름 설정
    std::vector<const char*> output_names_char;
    output_names_char.push_back(outputName.c_str());

    // 4. 모델 추론 실행
    std::vector<Ort::Value> output_tensors;
    try {
        output_tensors = ort_session->Run(
                Ort::RunOptions{nullptr}, // 기본 실행 옵션
                input_names_char.data(),  // 입력 노드 이름 배열
                inputTensors.data(),      // 입력 Ort::Value 배열
                inputTensors.size(),      // 입력 수
                output_names_char.data(), // 출력 노드 이름 배열
                output_names_char.size()  // 출력 수
        );
    } catch (const Ort::Exception& e) {
        LOGE("ONNX inference failed: %s", e.what());
        throw; // 예외를 다시 던져 상위에서 처리할 수 있도록 함
    }

    // 5. 결과 텐서 처리
    if (output_tensors.empty() || !output_tensors[0].IsTensor()) {
        LOGE("Inference returned no valid output tensor.");
        throw std::runtime_error("Inference returned no valid output tensor.");
    }

    Ort::Value& output_tensor = output_tensors[0];

    // 6. 텐서 정보 (Shape, Type) 가져오기
    auto type_info = output_tensor.GetTensorTypeAndShapeInfo();
    auto shape = type_info.GetShape();

    // 주석에서 [V, D] 형태라고 했으므로 2D 텐서를 가정
    if (shape.size() != 2) {
        LOGE("Output tensor shape is not 2D. Expected [V, D], but got %zu dimensions.", shape.size());
        throw std::runtime_error("Unexpected output tensor shape.");
    }

    size_t V = shape[0]; // 벡터의 수
    size_t D = shape[1]; // 각 벡터의 차원

    // 7. 텐서 데이터 포인터 가져오기 (float 타입 가정)
    const float* output_data = output_tensor.GetTensorData<float>();

    // 8. 1D 배열을 2D C++ 벡터로 복사
    std::vector<std::vector<float>> result;
    result.reserve(V); // V개의 행을 위해 미리 공간 할당

    for (size_t i = 0; i < V; ++i) {
        // 현재 행(vector)의 시작 위치
        const float* row_start = output_data + (i * D);

        // C++ vector의 생성자를 사용하여 D개의 요소를 효율적으로 복사
        result.emplace_back(row_start, row_start + D);
    }

    LOGI("Inference successful. Output shape: [%zu, %zu]", V, D);
    return result;
}