//
// Created by glion on 2025-10-31.
// 오디오 파일 로드 - ffmpeg 사용하여 16-bit PCM 형태로 리샘플링 및 모노 변환
//

#include "embedding_helper.h"

// FFmpeg 헤더 (구현 파일에서만 필요)
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
}

using namespace NdkEssentiaEmbedding;


AudioData EmbeddingHelper::loadAudioFile(
        const std::string& filePath,
        const EmbeddingConfig& config) {
    // 시간 측정
    RunTimerLogger timer("loadAudioFile Function");

    // 반환할 구조체
    AudioData audioResult;

    // ------------------ (1) 초기화 및 스트림 찾기 ------------------
    AVFormatContext *formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, filePath.c_str(), nullptr, nullptr) < 0) {
        LOGE("Failed to open input file : %s", filePath.c_str());
        return audioResult;
    }

    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        LOGE("Failed to retrieve stream info");
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    int streamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        LOGE("Failed to find audio stream");
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    AVStream *audioStream = formatCtx->streams[streamIndex];
    const AVCodec *codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if (!codec) {
        LOGE("Failed to find decoder");
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOGE("Failed to allocate codec context");
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    if (avcodec_parameters_to_context(codecCtx, audioStream->codecpar) < 0) {
        LOGE("Failed to copy codec parameters");
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        LOGE("Failed to open codec");
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    // ✅ 리샘플링 컨텍스트 설정
    SwrContext *swrCtx = swr_alloc();
    AVChannelLayout out_ch_layout;
    if (config.isMono) {
        av_channel_layout_default(&out_ch_layout, 1);
        audioResult.numChannels = 1;
    } else {
        av_channel_layout_copy(&out_ch_layout, &codecCtx->ch_layout);
        audioResult.numChannels = out_ch_layout.nb_channels;
    }

    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_FLT;  // float32
    int out_sample_rate = static_cast<int>(config.sr);
    audioResult.sampleRate = out_sample_rate; // 💡 구조체에 값 할당

    av_opt_set_chlayout(swrCtx, "in_chlayout", &codecCtx->ch_layout, 0);
    av_opt_set_int(swrCtx, "in_sample_rate", codecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", codecCtx->sample_fmt, 0);

    av_opt_set_chlayout(swrCtx, "out_chlayout", &out_ch_layout, 0);
    av_opt_set_int(swrCtx, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", out_sample_fmt, 0);

    if (swr_init(swrCtx) < 0) {
        LOGE("Failed to initialize resampler");
        swr_free(&swrCtx);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return audioResult;
    }

    // ✅ 디코딩 루프
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    const int MAX_OUT_SAMPLES = 4096; // (예시: 4096 샘플)
    uint8_t **convertedData = nullptr;
    av_samples_alloc_array_and_samples(
            &convertedData, nullptr,
            out_ch_layout.nb_channels, MAX_OUT_SAMPLES,
            out_sample_fmt, 0);

    // --- 메인 디코딩 루프 ---
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == streamIndex) {
            if (avcodec_send_packet(codecCtx, packet) >= 0) {
                while (avcodec_receive_frame(codecCtx, frame) >= 0) {
                    // [핵심] 프레임을 리샘플러로 보냄
                    int converted = swr_convert(swrCtx, convertedData, MAX_OUT_SAMPLES,
                                                (const uint8_t **) frame->data, frame->nb_samples);
                    int numSamples = converted * out_ch_layout.nb_channels;
                    float *data = reinterpret_cast<float *>(convertedData[0]);
                    audioResult.samples.insert(audioResult.samples.end(), data, data + numSamples);
                }
            }
        }
        av_packet_unref(packet);
    }

    // 파일이 끝났으므로 디코더에 NULL 패킷 전송
    if (avcodec_send_packet(codecCtx, nullptr) >= 0) {
        while (avcodec_receive_frame(codecCtx, frame) >= 0) {
            // 마지막 남은 프레임 리샘플링
            int converted = swr_convert(swrCtx, convertedData, MAX_OUT_SAMPLES,
                                        (const uint8_t **) frame->data, frame->nb_samples);
            int numSamples = converted * out_ch_layout.nb_channels;
            float *data = reinterpret_cast<float *>(convertedData[0]);
            audioResult.samples.insert(audioResult.samples.end(), data, data + numSamples);
        }
    }

    // 디코더가 끝났으므로 리샘플러에 NULL 입력 전송 (입력 샘플 수 = 0)
    int converted;
    do {
        converted = swr_convert(swrCtx, convertedData, MAX_OUT_SAMPLES,
                                nullptr, 0); // <--- NULL 입력
        int numSamples = converted * out_ch_layout.nb_channels;
        float *data = reinterpret_cast<float *>(convertedData[0]);
        audioResult.samples.insert(audioResult.samples.end(), data, data + numSamples);
    } while (converted > 0); // 리샘플러가 0을 반환할 때까지 반복

    // ✅ 해제
    av_freep(&convertedData[0]);
    av_freep(&convertedData);

    // [수정 4] 채널 레이아웃 해제 (사소한 메모리 누수 방지)
    av_channel_layout_uninit(&out_ch_layout);

    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swrCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

    return audioResult;
}