//
//  BOBOAACEncoder.h
//  SayHey
//
//  Created by syyang on 15/5/29.
//  Copyright (c) 2015年 Mingliang Chen. All rights reserved.
//

#ifndef __SayHey__BOBOAACEncoder__
#define __SayHey__BOBOAACEncoder__

#include <stdio.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#include "pthread.h"

struct BOBOAACEncoder {
    AVFormatContext *pFormatCtx;
    AVOutputFormat  *pOutFmt;
    
    AVCodecContext  *pCodeCtx;
    AVCodec         *pCodec;

    AVStream        *pStream;
    int buffer_size;
    uint16_t *samples;
    
    AVFrame     *pFrame;
    AVPacket    packet;
    
    int sample_rate;
    int channels;
    int bit_rate;
    int sample_fmt;
    pthread_mutex_t mutex;
    
};

typedef struct BOBOAACEncoder BOBOAACEncoder;


//BOBOAACEncoder *encoder_alloc();
int encoder_pcm_to_aac(BOBOAACEncoder *encoder,void *pcmData,void *encodedData);


BOBOAACEncoder *encoder_alloc();
BOBOAACEncoder *encoderAlloc(const char *path);

int encoderAAC(BOBOAACEncoder *encoder,uint8_t *inputBuffer,int inputSize,char *outputBuffer,int *outSize);
#endif /* defined(__SayHey__BOBOAACEncoder__) */
