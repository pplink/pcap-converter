#ifndef __REC_VIDEO_H__
#define __REC_VIDEO_H__

#include <gst/gst.h>
#include "recargs.h"

typedef struct _RecVideoData
{
    GstElement *filesrc;
    GstElement *pcapparse;
    GstElement *capsfilter;
    GstElement *queue;
    GstElement *depayload;
    GstElement *decodebin;
    GstElement *aspectratiocrop;
    GstElement *videoscale;
    GstElement *encoder;

    gint max_width;
    gint max_height;
    gint width;
    gint height;
    gint aspectRatioX;
    gint aspectRatioY;
} RecVideoData;

int rec_init_video(RecVideoData *data, RecArgs *args);
int rec_set_video_properties(RecVideoData *data, RecArgs *args);
void rec_add_video_elements(GstBin *bin, RecVideoData *data);
GstElement *rec_link_video_elements(RecVideoData *data);

#endif