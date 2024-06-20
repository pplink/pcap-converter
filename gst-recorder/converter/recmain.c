#include <stdio.h>
#include "recmain.h"

static void free_resources(RecData *data);
static void error_cb(GstBus *bus, GstMessage *msg, RecData *data);
static void eos_cb(GstBus *bus, GstMessage *msg, RecData *data);
static void message_cb(GstBus *bus, GstMessage *msg, RecData *data);
static void state_changed_cb(GstBus *bus, GstMessage *msg, RecData *data);

int rec_init(RecData *data, RecArgs *args)
{
    data->args = args;
    data->pipeline = gst_pipeline_new("pipeline");
    data->muxer = gst_element_factory_make("mp4mux", "muxer");
    data->sink = gst_element_factory_make("filesink", "sink");

    if (!data->pipeline || !data->muxer || !data->sink ||
            (REC_CONVERT_AUDIO(data) && rec_init_audio(&data->audio_data, data->args) != 0) ||
            (REC_CONVERT_VIDEO(data) && rec_init_video(&data->video_data, data->args) != 0))
    {
        return -1;
    }

    return 0;
}

int rec_set_properties(RecData *data)
{
    char *temp_string = (char *)malloc(1024 * sizeof(char));

    // Set pipeline properties
    g_object_set(GST_BIN(data->pipeline), "message-forward", TRUE, NULL);

    // Set muxer properties
    g_object_set(data->sink, "location", data->args->output_filename, NULL);
    g_object_set(data->muxer, "faststart", TRUE, NULL);
    g_object_set(data->muxer, "fragment-duration", 500, NULL);

    REC_CONVERT_VIDEO(data) ? rec_set_video_properties(&data->video_data, data->args) : 0;
    REC_CONVERT_AUDIO(data) ? rec_set_audio_properties(&data->audio_data, data->args) : 0;
    free(temp_string);
    return 0;
}

int rec_link(RecData *data)
{
    REC_CONVERT_AUDIO(data) ? rec_add_audio_elements(GST_BIN(data->pipeline), &data->audio_data) : 0;
    REC_CONVERT_VIDEO(data) ? rec_add_video_elements(GST_BIN(data->pipeline), &data->video_data) : 0;
    gst_bin_add_many(GST_BIN(data->pipeline), data->muxer, data->sink, NULL);

    if ((REC_CONVERT_VIDEO(data) && !gst_element_link_many(rec_link_video_elements(&data->video_data), data->muxer, NULL)) ||
            (REC_CONVERT_AUDIO(data) && !gst_element_link_many(rec_link_audio_elements(&data->audio_data), data->muxer, NULL)) ||
            !gst_element_link_many(data->muxer, data->sink, NULL))
    {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data->pipeline);
        return -1;
    }

    return 0;
}

int rec_start(RecData *data)
{
    GstBus *bus;

    bus = gst_element_get_bus(data->pipeline);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)error_cb, data);
    g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback)eos_cb, data);
    g_signal_connect(G_OBJECT(bus), "message", (GCallback)message_cb, data);
    g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)state_changed_cb, data);
    gst_object_unref(bus);

    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
    data->main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(data->main_loop);
    free_resources(data);

    return 0;
}

static void free_resources(RecData *data)
{
    gst_element_set_state(data->pipeline, GST_STATE_NULL);
    gst_object_unref(data->pipeline);
}

static void error_cb(GstBus *bus, GstMessage *msg, RecData *data)
{
    GError *err;
    gchar *debug_info;

    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);

    g_main_loop_quit(data->main_loop);
}

static void eos_cb(GstBus *bus, GstMessage *msg, RecData *data)
{
    g_print("End-Of-Stream reached.\n");
    g_main_loop_quit(data->main_loop);
}

static void message_cb(GstBus *bus, GstMessage *msg, RecData *data)
{
    switch (GST_MESSAGE_TYPE(msg))
    {

    case GST_MESSAGE_ELEMENT:
    {
        const GstStructure *structure = gst_message_get_structure(msg);

        if (gst_structure_has_name(structure, "GstBinForwarded"))
        {
            GstMessage *forward_msg = NULL;

            gst_structure_get(structure, "message", GST_TYPE_MESSAGE, &forward_msg, NULL);
            if (GST_MESSAGE_TYPE(forward_msg) == GST_MESSAGE_EOS)
            {
                g_print("Element %s sends EOS.\n",
                        GST_OBJECT_NAME(GST_MESSAGE_SRC(forward_msg)));
                g_main_loop_quit(data->main_loop);
            }
            gst_message_unref(forward_msg);
        }
        break;
    }
    default:
        break;
    }
}

static void state_changed_cb(GstBus *bus, GstMessage *msg, RecData *data)
{
    if (strncmp(GST_OBJECT_NAME(msg->src), "pipeline", 8) == 0)
    {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);

        g_print("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

        if (new_state == GST_STATE_PAUSED)
            g_print("message:ready\n");
    }
}