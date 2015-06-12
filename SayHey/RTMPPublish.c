//
//  RTMPPublish.c
//  SayHey
//
//  Created by syyang on 15/6/9.
//  Copyright (c) 2015年 Mingliang Chen. All rights reserved.
//

#include "RTMPPublish.h"
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>

int push(const char *in_filename,const char *out_filename)
{
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL,*ofmt_ctx = NULL;
    AVPacket pkt;

    
    int videoindex = -1;
    int frame_index = 0;
    int64_t start_time = 0;
    
    
    
    
    av_register_all();
    avformat_network_init();
    
    int ret;
    
    ret = avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL);
    
    printf("1 \n");
    if (ret<0) {
        return 0;
    }
    avformat_find_stream_info(ifmt_ctx, NULL);
    
    printf("2 \n");
    for (int i=0; i<ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }

    
    
    
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    
    // output
    printf("3 \n");
    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename);
    
    ofmt = ofmt_ctx->oformat;
    
    for (int i=0; i<ifmt_ctx->nb_streams; i++) {
        AVStream *instream = ifmt_ctx->streams[i];
        AVStream *outstream = avformat_new_stream(ofmt_ctx, instream->codec->codec);
        
        ret = avcodec_copy_context(outstream->codec, instream->codec);
        
        outstream->codec->codec_tag = 0;
        
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            outstream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
        
    }
    
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    printf("4 \n");
    if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("error!");
            return 0;
        }
    }
    
    printf("5 \n");
    ret = avformat_write_header(ofmt_ctx, NULL);
    
    printf("6 \n");
    start_time = av_gettime();
    
    while (1) {
        AVStream *instream,*outstream;
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret<0) {
            printf("error!\n");
            break;
        }
        if(pkt.pts==AV_NOPTS_VALUE){
            //Write PTS
            AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts=pkt.pts;
            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }
    
        if(pkt.stream_index==videoindex){
            AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q={1,AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);
            
        }
        
        instream = ifmt_ctx->streams[pkt.stream_index];
        outstream = ofmt_ctx->streams[pkt.stream_index];
        
        pkt.pts = av_rescale_q_rnd(pkt.pts, instream->time_base, outstream->time_base, (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, instream->time_base, outstream->time_base, (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, instream->time_base, outstream->time_base);
        pkt.pos = -1;
        
        if (pkt.stream_index == videoindex) {
            printf("Send %8d video frames to output URL\n",frame_index);
            frame_index++;
        }

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        
        if (ret < 0) {
            printf( "Error muxing packet\n");
            break;
        }
        
        av_free_packet(&pkt);
    }
    printf("7 \n");
    av_write_trailer(ofmt_ctx);
    
    avformat_close_input(&ifmt_ctx);
    
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE)) {
        avio_close(ofmt_ctx->pb);
    }
    
    avformat_free_context(ofmt_ctx);
    
    
    return 0;
}