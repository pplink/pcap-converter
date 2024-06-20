#ifndef __REC_AUDIO_H__
#define __REC_AUDIO_H__

#include <gst/gst.h>
#include "recargs.h"

typedef struct _RecAudioData
{
    GstElement *filesrc;
    GstElement *pcapparse;
    GstElement *capsfilter;
    GstElement *queue;
    GstElement *depayload;
    GstElement *decoder;
    GstElement *audiomixer;
    GstElement *encoder;
} RecAudioData;

int rec_init_audio(RecAudioData *data, RecArgs *args);
int rec_set_audio_properties(RecAudioData *data, RecArgs *args);
void rec_add_audio_elements(GstBin *bin, RecAudioData *data);
GstElement *rec_link_audio_elements(RecAudioData *data);

#endif