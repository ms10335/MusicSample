#include <iostream>
#include "miniaudio.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

#define MINIAUDIO_IMPLEMENTATION

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AVAudioFifo* fifo = reinterpret_cast<AVAudioFifo*>(pDevice->pUserData);
    av_audio_fifo_read(fifo, &pOutput, frameCount);
    (void)pInput;
}

int main(int, char**) {
    //1, open audio file
    AVFormatContext* format_ctx{nullptr};
    int ret = avformat_open_input(&format_ctx, "/home/marcins/Programowanie/CodersSchool/MusicSample/Way.mp3", nullptr, nullptr);
    if (ret < 0) {
        fprintf(stderr, "Unable to open media\n");
        return -1;
    }
    fprintf(stdout, "File audio is opened\n");
    ret = avformat_find_stream_info(format_ctx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "Unable to find stream info.\n");
        return -1;
    }
    fprintf(stderr, "Umber of streams inside of media %d\n", format_ctx->nb_streams);
    int index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (index < 0) {
        fprintf(stderr, "No audio streams inside of this file!\n");
        return -1;
    }
    AVStream* stream = format_ctx->streams[index];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        fprintf(stderr, "Unable to find decoder!\n");
        return -1;
    }

    AVCodecContext* codec_ctx{avcodec_alloc_context3(decoder)};
    avcodec_parameters_to_context(codec_ctx, stream->codecpar);
    ret = avcodec_open2(codec_ctx, decoder, nullptr);
    if (ret < 0) {
        fprintf(stderr, "Unable to open decoder!\n");
        return -1;
    }
    //2. decode the audio
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    SwrContext* resampler{swr_alloc_set_opts(nullptr, stream->codecpar->channel_layout, AV_SAMPLE_FMT_FLT,
                                             stream->codecpar->sample_rate, stream->codecpar->channel_layout,
                                             (AVSampleFormat)stream->codecpar->format, stream->codecpar->sample_rate, 0, nullptr)};

    AVAudioFifo* fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, stream->codecpar->channels, 1);

    while (av_read_frame(format_ctx, packet) == 0) {
        if (packet->stream_index != stream->index)
            continue;
        ret = avcodec_send_packet(codec_ctx, packet);
        if (ret < 0) {
            //AVERROR(EAGAIN) ==> Send the pacek again after getting frames out !
            if (ret != AVERROR(EAGAIN)) {
                //read frames
                fprintf(stderr, "Some decoding error occured");
            }
        }
        while ((ret = avcodec_receive_frame(codec_ctx, frame)) == 0) {
            //Resample the frame
            AVFrame* resampled_frames = av_frame_alloc();
            resampled_frames->sample_rate = frame->sample_rate;
            resampled_frames->channel_layout = frame->channel_layout;
            resampled_frames->channels = frame->channels;
            resampled_frames->format = AV_SAMPLE_FMT_FLT;

            ret = swr_convert_frame(resampler, resampled_frames, frame);
            av_frame_unref(frame);
            av_audio_fifo_write(fifo, (void**)resampled_frames->data, resampled_frames->nb_samples);
            av_frame_free(&resampled_frames);
        }
    }
    //3. play back audio
    ma_device_config deviceConfig;
    ma_device device;
/*
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = stream->codecpar->channels;
    deviceConfig.sampleRate = stream->codecpar->sample_rate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = fifo;

    avformat_close_input(&format_ctx);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    swr_free(&resampler);

    //init devive and start the device
    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -4;
    }

    ma_device_uninit(&device);
*/
   
    av_audio_fifo_free(fifo);
    getchar();
    return 0;
}