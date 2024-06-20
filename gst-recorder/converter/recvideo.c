#include <stdio.h>
#include "recvideo.h"

static void decodebin_pad_added_handler(GstElement *src, GstPad *new_pad, RecVideoData *data);
static void set_video_dimension(gint width, gint height, RecVideoData *data);
static int get_gcd(long x, long y);
static void set_properties_and_link_elements(RecVideoData *data);

int rec_init_video(RecVideoData *data, RecArgs *args)
{
    data->filesrc = gst_element_factory_make("filesrc", "videofilesrc");
    data->pcapparse = gst_element_factory_make("pcapparse", "videopcapparse");
    data->capsfilter = gst_element_factory_make("capsfilter", "videocapsfilter");
    data->queue = gst_element_factory_make("queue", "videoqueue");
    data->depayload = gst_element_factory_make("rtpvp8depay", "videodepayload");
    data->decodebin = gst_element_factory_make("decodebin", "videodecodebin");
    data->aspectratiocrop = gst_element_factory_make("aspectratiocrop", "aspectratiocrop");
    data->videoscale = gst_element_factory_make("videoscale", "videoscale");
    data->encoder = gst_element_factory_make("x264enc", "videoencoder"); // consumes CPU a lot, GPU 쓰는 다른 플러그인 활용 가능

    if (!data->filesrc || !data->pcapparse || !data->capsfilter ||
            !data->queue || !data->depayload || !data->decodebin ||
            !data->aspectratiocrop || !data->videoscale || !data->encoder)
    {
        return -1;
    }

    data->max_width = args->video_max_width;
    data->max_height = args->video_max_height;

    if (data->max_width <= 0)
        data->max_width = 1920;
    if (data->max_height <= 0)
        data->max_height = 1440;

    return 0;
}

int rec_set_video_properties(RecVideoData *data, RecArgs *args)
{
    char *temp_string = (char *)malloc(1024 * sizeof(char));

    // Set src properties
    g_object_set(data->filesrc, "location", args->video_packet_filename, NULL);
    sprintf(temp_string, "application/x-rtp, media=(string)video, clock-rate=(int)90000, payload=(int)%d ,encoding-name=(string)VP8",
            args->video_payload_type);
    GstCaps *caps = gst_caps_from_string(temp_string);
    g_object_set(data->capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    free(temp_string);
    return 0;
}

void rec_add_video_elements(GstBin *bin, RecVideoData *data)
{
    gst_bin_add_many(bin, data->filesrc, data->pcapparse, data->capsfilter,
                     data->queue, data->depayload, data->decodebin,
                     data->aspectratiocrop, data->videoscale, data->encoder, NULL);
}

GstElement *rec_link_video_elements(RecVideoData *data)
{
    if (!gst_element_link_many(data->filesrc, data->pcapparse, data->capsfilter, data->queue, NULL) ||
            !gst_element_link_many(data->queue, data->depayload, data->decodebin, NULL) ||
            !gst_element_link_many(data->aspectratiocrop, data->videoscale, NULL))
    {
        return NULL;
    }

    g_signal_connect(data->decodebin, "pad-added", G_CALLBACK(decodebin_pad_added_handler), data);

    return data->encoder;
}

static void decodebin_pad_added_handler(GstElement *src, GstPad *new_pad, RecVideoData *data)
{
    GstPad *sink_pad = gst_element_get_static_pad(data->aspectratiocrop, "sink");
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

    g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

    if (gst_pad_is_linked(sink_pad))
    {
        g_print("Already linked.\n");
        gst_object_unref(sink_pad);
        return;
    }

    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);
    if (!g_str_has_prefix(new_pad_type, "video/x-raw"))
    {
        g_print("Not supported type '%s'.\n", new_pad_type);
        if (new_pad_caps != NULL)
            gst_caps_unref(new_pad_caps);
        gst_object_unref(sink_pad);
        return;
    }

    gint width, height = 0;
    gst_structure_get_int(new_pad_struct, "width", &width);
    gst_structure_get_int(new_pad_struct, "height", &height);
    set_video_dimension(width, height, data);
    set_properties_and_link_elements(data);

    ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
    {
        g_print("Type is '%s' but link failed.\n", new_pad_type);
    }
    else
    {
        g_print("Link succeeded (type '%s').\n", new_pad_type);
    }

    if (new_pad_caps != NULL)
        gst_caps_unref(new_pad_caps);
    gst_object_unref(sink_pad);
}

int get_gcd(long x, long y)
{
    if (y == 0)
        return x;
    else
        return get_gcd(y, x % y);
}

static void set_video_dimension(gint width, gint height, RecVideoData *data)
{
    if (width <= 10 || height <= 10)
    {
        data->width = data->max_width;
        data->height = data->max_height;
    }
    else
    {
        data->width = width;
        data->height = height;
    }

    gint gcd = get_gcd(data->width, data->height);
    data->aspectRatioX = data->width / gcd;
    data->aspectRatioY = data->height / gcd;

    if (width >= height && width > data->max_width)
    {
        data->width = data->max_width;
        data->height = data->max_width * data->aspectRatioY / data->aspectRatioX;
    }
    else if (width < height && height > data->max_height)
    {
        data->width = data->max_height * data->aspectRatioX / data->aspectRatioY;
        data->height = data->max_height;
    }

    if (data->width % 2 == 1) data->width--;
    if (data->height % 2 == 1) data->height--;

    g_print("Get video width %d height %d\n", width, height);
    g_print("Set video width %d height %d\n", data->width, data->height);
    g_print("Set video aspectRatioX %d aspectRatioY %d\n", data->aspectRatioX, data->aspectRatioY);
}

static void set_properties_and_link_elements(RecVideoData *data)
{
    g_object_set(data->aspectratiocrop, "aspect-ratio", data->aspectRatioX, data->aspectRatioY, NULL);
    char *temp_string = (char *)malloc(1024 * sizeof(char));
    sprintf(temp_string, "video/x-raw,width=%d,height=%d", data->width, data->height);
    GstCaps *caps = gst_caps_from_string(temp_string);
    free(temp_string);
    if (!gst_element_link_filtered(data->videoscale, data->encoder, caps))
    {
        g_printerr("Cannot link encoder.\n");
        gst_caps_unref(caps);
        exit(1);
    }
    gst_caps_unref(caps);
}