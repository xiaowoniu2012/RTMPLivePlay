//
//  BOBOAACEncoder.c
//  SayHey
//
//  Created by syyang on 15/5/29.
//  Copyright (c) 2015年 Mingliang Chen. All rights reserved.
//

#include "BOBOAACEncoder.h"


BOBOAACEncoder *encoder_alloc()
{
    BOBOAACEncoder *encoder = (BOBOAACEncoder *)malloc(sizeof(BOBOAACEncoder));

    
    
    if (encoder == NULL) {
        return NULL;
    }
    
    av_register_all();
    
    const char* out_file = "test.flv";
    encoder->pFormatCtx = avformat_alloc_context();
    encoder->pOutFmt = av_guess_format(NULL, out_file, NULL);
    encoder->pFormatCtx->oformat = encoder->pOutFmt;
    
    encoder->pOutFmt->audio_codec = AV_CODEC_ID_AAC;
    encoder->pOutFmt->video_codec = AV_CODEC_ID_NONE;
    
    encoder->pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    
    encoder->pStream  = avformat_new_stream(encoder->pFormatCtx, encoder->pCodec);
    
    encoder->pStream->id = encoder->pFormatCtx->nb_streams-1;
    
    encoder->pStream->index = 0;
    
    
    // codec context
    AVCodecContext *codecContext = encoder->pStream->codec;
    
    codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    codecContext->channels = 2;
    codecContext->channel_layout = av_get_default_channel_layout(2);
    codecContext->bit_rate = 64000;
    codecContext->sample_rate = 44100;
    encoder->pCodeCtx = codecContext;
    
    if (encoder->pOutFmt->flags & AVFMT_GLOBALHEADER) {
        
        encoder->pOutFmt->flags |= CODEC_FLAG_GLOBAL_HEADER;
        
    }
    
    //open codec
    
    if(avcodec_open2(encoder->pCodeCtx, encoder->pCodec, NULL)<0) {
        printf("failed to open encoder !\n");
        return NULL;
    }
    
    int ret = 0;
    
//    if (!(encoder->pFormatCtx->flags & AVFMT_NOFILE)) {
//        ret = avio_open(&encoder->pFormatCtx->pb, out_file, AVIO_FLAG_WRITE);
//        if (ret < 0) {
//            printf("Could not open '%s': %s\n", out_file,
//                 av_err2str(ret));
//            return NULL;
//        }
//    }
//    
    av_dump_format(encoder->pFormatCtx, 0, out_file, 1);
    
//    ret = avformat_write_header(encoder->pFormatCtx, NULL);
    
    if (ret<0) {
        printf("avformat_write_header ERROR! \n");
        return NULL;
    }
    
    return encoder;
    
}

BOBOAACEncoder *encoderAlloc(const char *path)
{
    
    BOBOAACEncoder *encoder = (BOBOAACEncoder *)malloc(sizeof(BOBOAACEncoder));
    av_register_all();
    avformat_network_init();
    
    int ret;
    AVFormatContext *ofmt_ctx;
    AVOutputFormat *oformat;
    AVCodecContext *codec_ctx;
    AVCodec         *codec;
    AVStream        *audioStream;
    AVPacket        pkt;
    AVFrame         *audioFrame;
    uint8_t         *frameBuff;
    
    
    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", path);
    oformat = ofmt_ctx->oformat;
    
    if (avio_open(&ofmt_ctx->pb, path,AVIO_FLAG_WRITE) < 0 ) {
        
        printf("failed to open output file \n");
        return NULL;
    }
    
    printf("1 \n");

    codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    
    audioStream = avformat_new_stream(ofmt_ctx, codec);
    
    codec_ctx = audioStream->codec;
    codec_ctx->codec_id = ofmt_ctx->audio_codec_id;
    codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    codec_ctx->sample_rate = 22050;
    codec_ctx->channel_layout = AV_CH_LAYOUT_MONO;
    codec_ctx->channels = av_get_channel_layout_nb_channels(codec_ctx->channel_layout);
//    codec_ctx->bit_rate = 64000;
    codec_ctx->codec = codec;
    if (oformat->flags & AVFMT_GLOBALHEADER) {
        codec_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    
    if (avcodec_open2(codec_ctx, codec, NULL)<0) {
        printf("open codec error \n");
        
        return NULL;
    }
    
    
    

    
    printf("2 \n");
    size_t buffersize = av_samples_get_buffer_size(NULL, codec_ctx->channels, codec_ctx->frame_size, codec_ctx->sample_fmt, 1);
    printf("buffersize =  %d \n",buffersize);
    

    

    
    
    printf("3  \n");
    avformat_write_header(ofmt_ctx, NULL);
    

    encoder->pCodeCtx = codec_ctx;
    encoder->pCodec = codec;
    encoder->pFormatCtx = ofmt_ctx;
    encoder->buffer_size = buffersize;
    encoder->pStream = audioStream;
    
    
    av_dump_format(ofmt_ctx, 0, path, 1);
    
    
    pthread_mutex_init(&encoder->mutex, NULL);
    
    return encoder;
    

}

static int  frameIndex =0;
int encoderAAC(BOBOAACEncoder *encoder,uint8_t *inputBuffer,int inputSize,char *outputBuffer,int *outSize)
{
    
 
//    pthread_mutex_lock(&encoder->mutex);
    
    AVFrame *frame = avcodec_alloc_frame();
    frame->nb_samples = encoder->pCodeCtx->frame_size;
    frame->format = encoder->pCodeCtx->sample_fmt;
    frame->sample_rate = encoder->pCodeCtx->sample_rate;
    frame->channels = encoder->pCodeCtx->channels;
    frame->pts = frameIndex*100;

    
    av_init_packet(&encoder->packet);
    
    int gotFrame = 0;

    
    
    int ret = avcodec_fill_audio_frame(frame, encoder->pCodeCtx->channels, AV_SAMPLE_FMT_S16, (uint8_t*)inputBuffer, encoder->buffer_size, 0);
    
    if (ret<0) {
        printf("avcodec_fill_audio_frame error !\n");
        av_frame_free(&frame);
        av_free_packet(&encoder->packet);
//        pthread_mutex_unlock(&encoder->mutex);
        return -1;
    }
    printf("111111");
    ret = avcodec_encode_audio2(encoder->pCodeCtx, &encoder->packet, frame, &gotFrame);
    printf("222222");
    if (ret < 0) {
        printf("encoder error! \n");
        av_frame_free(&frame);
        av_free_packet(&encoder->packet);
//        pthread_mutex_unlock(&encoder->mutex);
        return -1;
    }
    if (gotFrame == 1) {
         encoder->packet.stream_index = encoder->pStream->index;
        ret = av_interleaved_write_frame(encoder->pFormatCtx, &encoder->packet);
        frameIndex++;
        
        printf("encoder %d frame success \n",frameIndex);
    }
    
    av_frame_free(&frame);
    av_free_packet(&encoder->packet);
//    pthread_mutex_unlock(&encoder->mutex);
    return 1;
}

int encoder_pcm_to_aac(BOBOAACEncoder *encoder,void *pcmData,void *encodedData)
{
    if (encoder == NULL) {
        return -1;
    }
    
    
    AVFrame *frame = avcodec_alloc_frame();
    frame->nb_samples = encoder->pCodeCtx->frame_size;
    frame->format = encoder->pCodeCtx->sample_fmt;
    
    int ret = 0;
    
    int buffer_size = av_samples_get_buffer_size(NULL, encoder->pCodeCtx->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);
    
    AVPacket packet;
    
    av_new_packet(&packet, buffer_size);
    
    
    
    //pcm -> frame
    ret = avcodec_fill_audio_frame(frame, encoder->pCodeCtx->channels, AV_SAMPLE_FMT_S16, pcmData, buffer_size, 0);
    
    
    if (ret <0) {
        printf("avcodec_fill_audio_frame error! \n");
        return  -1;
    }
    
    int gotFrame;
    
    ret = avcodec_encode_audio2(encoder->pCodeCtx, &packet, frame, &gotFrame);
    
    if (ret <0) {
        printf("failed to encode! \n");
        return  -1;
    }
    
    if (gotFrame == 1) {
        printf("Succeed to encode 1 frame! \tsize:%5d\n",packet.size);
        packet.stream_index = encoder->pStream->index;
        memcpy(encodedData, packet.data, packet.size);
        av_free_packet(&packet);
        return 1;
        
    }else{
        return  -1;
    }
    
    return -1;
    
}