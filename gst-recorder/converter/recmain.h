#ifndef __REC_DATA_H__
#define __REC_DATA_H__

#include <gst/gst.h>
#include "recargs.h"
#include "recaudio.h"
#include "recvideo.h"

typedef struct _RecData
{
    GstElement *pipeline;
    GstElement *muxer;
    GstElement *sink;
    RecArgs *args;
    RecAudioData audio_data;
    RecVideoData video_data;
    GMainLoop *main_loop;
} RecData;

#define REC_CONVERT_VIDEO(data) data->args->video_packet_filename[0] != 0
#define REC_CONVERT_AUDIO(data) data->args->audio_packet_filename[0] != 0

int rec_init(RecData *data, RecArgs *args);
int rec_set_properties(RecData *data);
int rec_link(RecData *data);
int rec_start(RecData *data);

#endif