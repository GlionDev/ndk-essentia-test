//
// Created by glion on 2025-11-05.
// 시간 측정 클래스 헤더파일 - 생성자와 소멸자를 통해 시간 측정
//

#ifndef NDK_ESSENTIA_TEST_CAL_RUNTIME_H
#define NDK_ESSENTIA_TEST_CAL_RUNTIME_H

#include <chrono> // 실행시간 측정을 위함
#include "common/log_util.h" // 로그 유틸리티 사용
using namespace std::chrono;

class RunTimerLogger {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    RunTimerLogger(const std::string& taskName) :
            taskName_(taskName),
            start_(Clock::now()) {}

    // 소멸자에서 시간 측정 및 로깅을 수행 (RAII)
    ~RunTimerLogger() {
        auto end = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);

        // 1. 나노초를 밀리초로 변환
        double milliseconds = static_cast<double>(duration.count()) / 1000000.0;

        // 2. LOGD로 출력
        LOGD("NDK 단 %s 소요시간 :: %.3f ms", taskName_.c_str(), milliseconds);
    }

private:
    std::string taskName_;
    TimePoint start_;
};

#endif //NDK_ESSENTIA_TEST_CAL_RUNTIME_H
