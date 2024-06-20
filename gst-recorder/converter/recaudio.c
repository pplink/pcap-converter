#include <stdio.h>
#include "recaudio.h"

int rec_init_audio(RecAudioData *data, RecArgs *args)
{
    data->filesrc = gst_element_factory_make("filesrc", "audiofilesrc");
    data->pcapparse = gst_element_factory_make("pcapparse", "audiopcapparse");
    data->capsfilter = gst_element_factory_make("capsfilter", "audiocapsfilter");
    data->queue = gst_element_factory_make("queue", "audioqueue");
    data->depayload = gst_element_factory_make("rtpopusdepay", "audiodepayload");
    data->decoder = gst_element_factory_make("opusdec", "audiodecoder");
    data->audiomixer = gst_element_factory_make("audiomixer", "audiomixer");
    data->encoder = gst_element_factory_make("voaacenc", "audioencoder");

    if (!data->filesrc || !data->pcapparse || !data->capsfilter || !data->queue || !data->depayload ||
            !data->decoder || !data->audiomixer || !data->encoder)
    {
        return -1;
    }

    return 0;
}

int rec_set_audio_properties(RecAudioData *data, RecArgs *args)
{
    char *temp_string = (char *)malloc(1024 * sizeof(char));

    // Set src properties
    g_object_set(data->filesrc, "location", args->audio_packet_filename, NULL);
    sprintf(temp_string, "application/x-rtp, media=(string)audio, clock-rate=(int)48000, payload=(int)%d ,encoding-name=(string)OPUS",
            args->audio_payload_type);
    GstCaps *caps = gst_caps_from_string(temp_string);
    g_object_set(data->capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    free(temp_string);
    return 0;
}

void rec_add_audio_elements(GstBin *bin, RecAudioData *data)
{
    gst_bin_add_many(bin, data->filesrc, data->pcapparse, data->capsfilter,
                     data->queue, data->depayload, data->decoder, data->audiomixer, 
                     data->encoder, NULL);
}

GstElement *rec_link_audio_elements(RecAudioData *data)
{
    if (!gst_element_link_many(data->filesrc, data->pcapparse, data->capsfilter, data->queue, NULL) ||
            !gst_element_link_many(data->queue, data->depayload, data->decoder, data->audiomixer, data->encoder, NULL))
    {
        return NULL;
    }

    return data->encoder;
}
