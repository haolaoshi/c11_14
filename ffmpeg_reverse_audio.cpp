#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
}

#define OUTPUT_SAMPLES_PER_SECOND 44100
#define OUTPUT_AUDIO_FORMAT AV_SAMPLE_FMT_S16
#define OUTPUT_CHANNELS 2

int reverse_audio(const std::string& inputFilePath, const std::string& outputFilePath) {
    // 注册FFmpeg库中的组件
    av_register_all();

    // 打开输入音频文件并读取音频流信息
    AVFormatContext* inputFormatContext = nullptr;
    if (avformat_open_input(&inputFormatContext, inputFilePath.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "无法打开输入文件。" << std::endl;
        return -1;
    }
    if (avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
        std::cerr << "无法获取输入文件的流信息。" << std::endl;
        avformat_close_input(&inputFormatContext);
        return -1;
    }

    // 寻找音频流
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < inputFormatContext->nb_streams; i++) {
        if (inputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        std::cerr << "找不到音频流。" << std::endl;
        avformat_close_input(&inputFormatContext);
        return -1;
    }

    // 获取音频解码器上下文
    AVCodecContext* inputCodecContext = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(inputCodecContext, inputFormatContext->streams[audioStreamIndex]->codecpar);
    AVCodec* inputCodec = avcodec_find_decoder(inputCodecContext->codec_id);
    if (inputCodec == nullptr) {
        std::cerr << "找不到合适的解码器。" << std::endl;
        avcodec_free_context(&inputCodecContext);
        avformat_close_input(&inputFormatContext);
        return -1;
    }
    if (avcodec_open2(inputCodecContext, inputCodec, nullptr) < 0) {
        std::cerr << "无法打开解码器。" << std::endl;
        avcodec_free_context(&inputCodecContext);
        avformat_close_input(&inputFormatContext);
        return -1;
    }

    // 创建输出音频上下文
    AVFormatContext* outputFormatContext = nullptr;
    if (avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, outputFilePath.c_str()) < 0) {
        std::cerr << "无法创建输出文件。" << std::endl;
        avcodec_free_context(&inputCodecContext);
        avformat_close_input(&inputFormatContext);
        return -1;
    }

    // 设置输出音频流信息
    AVStream* outputAudioStream = avformat_new_stream(outputFormatContext, nullptr);
    if (outputAudioStream == nullptr) {
        std::cerr << "无法创建输出音频流。" << std::endl;
        avcodec_free_context(&inputCodecContext);
        avformat_close_input(&inputFormatContext);
        avformat_free_context(outputFormatContext);
        return -1;
    }
    outputAudioStream->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
    outputAudioStream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    outputAudioStream->codecpar->sample_rate = OUTPUT_SAMPLES_PER_SECOND;
    outputAudioStream->codecpar->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    outputAudioStream->codecpar->channels = OUTPUT_CHANNELS;
    outputAudioStream->codecpar->bit_rate = av_get_bytes_per_sample(OUTPUT_AUDIO_FORMAT) * OUTPUT_SAMPLES_PER_SECOND * OUTPUT_CHANNELS * 8;

    // 设置输出音频编码器并打开编码器
    AVCodec* outputCodec = avcodec_find_encoder(outputAudioStream->codecpar->codec_id);
    AVCodecContext* outputCodecContext = avcodec_alloc_context3(outputCodec);
    avcodec_parameters_to_context(outputCodecContext, outputAudioStream->codecpar);
    if (avcodec_open2(outputCodecContext, outputCodec, nullptr) < 0) {
        std::cerr << "无法打开输出音频编码器。" << std::endl;
        avcodec_free_context(&inputCodecContext);
        avformat_close_input(&inputFormatContext);
        avformat_free_context(outputFormatContext);
        return -1;
    }

    // 打开输出文件并写入头部信息
    if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&outputFormatContext->pb, outputFilePath.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "无法打开输出文件。" << std::endl;
            avcodec_free_context(&inputCodecContext);
            avformat_close_input(&inputFormatContext);
            avformat_free_context(outputFormatContext);
            return -1;
        }
    }
    if (avformat_write_header(outputFormatContext, nullptr) < 0) {
        std::cerr << "无法写入输出文件头部信息。" << std::endl;
        avcodec_free_context(&inputCodecContext);
        avformat_close_input(&inputFormatContext);
        avformat_free_context(outputFormatContext);
        return -1;
    }

    // 分配音频重采样上下文
    SwrContext* resampleContext = swr_alloc();
    av_opt_set_int(resampleContext, "in_channel_count", inputCodecContext->channels, 0);
    av_opt_set_int(resampleContext, "out_channel_count", OUTPUT_CHANNELS, 0);
    av_opt_set_int(resampleContext, "in_channel_layout", inputCodecContext->channel_layout, 0);
    av_opt_set_int(resampleContext, "out_channel_layout", av_get_default_channel_layout(OUTPUT_CHANNELS), 0);
    av_opt_set_int(resampleContext, "in_sample_rate", inputCodecContext->sample_rate, 0);
    av_opt_set_int(resampleContext, "out_sample_rate", OUTPUT_SAMPLES_PER_SECOND, 0);
    av_opt_set_sample_fmt(resampleContext, "in_sample_fmt", inputCodecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(resampleContext, "out_sample_fmt", OUTPUT_AUDIO_FORMAT, 0);
    swr_init(resampleContext);

    // 分配音频FIFO缓冲队列
    AVAudioFifo* audioFifo = av_audio_fifo_alloc(OUTPUT_AUDIO_FORMAT, OUTPUT_CHANNELS, 1);

    // 定义一帧音频大小和缓冲
    constexpr int frameSize = 1024;
    uint8_t** inputFrameData = static_cast<uint8_t**>(av_malloc(sizeof(uint8_t*) * OUTPUT_CHANNELS));
    inputFrameData[0] = static_cast<uint8_t*>(av_malloc(frameSize));
    inputFrameData[1] = static_cast<uint8_t*>(av_malloc(frameSize));
    int outputFrameSize = av_rescale_rnd(swr_get_delay(resampleContext, OUTPUT_SAMPLES_PER_SECOND) + frameSize,
        OUTPUT_SAMPLES_PER_SECOND, inputCodecContext->sample_rate, AV_ROUND_UP);
    uint8_t** outputFrameData = static_cast<uint8_t**>(av_malloc(sizeof(uint8_t*) * OUTPUT_CHANNELS));
    outputFrameData[0] = static_cast<uint8_t*>(av_malloc(outputFrameSize));
    outputFrameData[1] = static_cast<uint8_t*>(av_malloc(outputFrameSize));

    // 读取音频帧并进行反向处理
    AVPacket packet;
    av_init_packet(&packet);

    while (av_read_frame(inputFormatContext, &packet) >= 0) {
        if (packet.stream_index == audioStreamIndex) {
            // 解码音频帧
            AVFrame* inputFrame = av_frame_alloc();
            if (inputFrame == nullptr) {
                std::cerr << "无法分配输入音频帧。" << std::endl;
                break;
            }
            int frameFinished;
            avcodec_decode_audio4(inputCodecContext, inputFrame, &frameFinished, &packet);

            if (frameFinished) {
                // 将音频帧重采样为目标格式
                swr_convert(resampleContext, outputFrameData, outputFrameSize / OUTPUT_CHANNELS,
                    const_cast<const uint8_t**>(inputFrame->data), inputFrame->nb_samples);
                int outputFrames = av_rescale_rnd(swr_get_delay(resampleContext, OUTPUT_SAMPLES_PER_SECOND) + outputFrameSize / OUTPUT_CHANNELS,
                    OUTPUT_SAMPLES_PER_SECOND, inputCodecContext->sample_rate, AV_ROUND_UP);

                // 将重采样后的帧写入缓冲队列
                av_audio_fifo_realloc(audioFifo, av_audio_fifo_size(audioFifo) + outputFrames);
                av_audio_fifo_write(audioFifo, reinterpret_cast<void**>(outputFrameData), outputFrames);
            }

            av_frame_free(&inputFrame);
        }

        av_packet_unref(&packet);
    }

    // 初始化输出包
    AVPacket outputPacket;
    av_init_packet(&outputPacket);
    outputPacket.data = nullptr;
    outputPacket.size = 0;

    // 从缓冲队列读取反转后的音频帧并写入文件
    int outputBufferSize = outputFrameSize / OUTPUT_CHANNELS;
    int outputSamplesCount = av_rescale_rnd(av_audio_fifo_size(audioFifo),
        OUTPUT_SAMPLES_PER_SECOND, inputCodecContext->sample_rate, AV_ROUND_UP);
    av_new_packet(&outputPacket, OUTPUT_CHANNELS * outputSamplesCount * av_get_bytes_per_sample(OUTPUT_AUDIO_FORMAT));

    while (av_audio_fifo_size(audioFifo) >= outputBufferSize) {
        av_audio_fifo_read(audioFifo, reinterpret_cast<void**>(outputFrameData), outputBufferSize);

        swr_convert(resampleContext, inputFrameData, frameSize / OUTPUT_CHANNELS,
            const_cast<const uint8_t**>(outputFrameData), outputSamplesCount);

        memcpy(outputPacket.data, inputFrameData[0], outputPacket.size);

        // 写入音频包
        outputPacket.pts = av_rescale_q(outputAudioStream->nb_frames, outputAudioStream->time_base, outputAudioStream->codec->time_base);
        av_interleaved_write_frame(outputFormatContext, &outputPacket);
    }

    // 写入剩余的包
    int bytesPerSample = av_get_bytes_per_sample(OUTPUT_AUDIO_FORMAT);
    outputPacket.size = av_audio_fifo_size(audioFifo) * OUTPUT_CHANNELS * bytesPerSample;
    if (outputPacket.size > 0) {
        av_audio_fifo_read(audioFifo, reinterpret_cast<void**>(outputFrameData), outputBufferSize);

        swr_convert(resampleContext, inputFrameData, frameSize / OUTPUT_CHANNELS,
            const_cast<const uint8_t**>(outputFrameData), outputSamplesCount);

        memcpy(outputPacket.data, inputFrameData[0], outputPacket.size);

        // 写入音频包
        outputPacket.pts = av_rescale_q(outputAudioStream->nb_frames, outputAudioStream->time_base, outputAudioStream->codec->time_base);
        av_interleaved_write_frame(outputFormatContext, &outputPacket);
    }

    // 写入输出文件尾部
    av_write_trailer(outputFormatContext);

    // 释放资源
    av_audio_fifo_free(audioFifo);
    swr_free(&resampleContext);
    av_free(inputFrameData[0]);
    av_free(inputFrameData[1]);
    av_free(inputFrameData);
    av_free(outputFrameData[0]);
    av_free(outputFrameData[1]);
    av_free(outputFrameData);
    avcodec_free_context(&inputCodecContext);
    avcodec_free_context(&outputCodecContext);
    avformat_close_input(&inputFormatContext);
    avformat_free_context(outputFormatContext);

    std::cout << "音频转换完成！" << std::endl;

    return 0;
}

int main() {
    std::string inputFilePath = "input.mp3"; // 输入音频文件路径
    std::string outputFilePath = "output_reversed.mp3"; // 输出音频文件路径

    int result = reverse_audio(inputFilePath, outputFilePath);

    if (result != 0) {
        std::cerr << "音频转换失败！" << std::endl;
    }

    return 0;
}