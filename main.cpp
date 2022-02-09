#include <iostream>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libswresample/swresample.h>
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
    ret = avcodec_open2(codec_ctx,decoder, nullptr);
    if (ret < 0 ) {
        fprintf(stderr, "Unable to open decoder!\n");
        return -1;
    }
    
    //2. decode the audio
    //3. play back audio 
}