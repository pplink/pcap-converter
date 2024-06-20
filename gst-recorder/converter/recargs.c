#include <gst/gst.h>
#include <stdlib.h>
#include "recargs.h"

int rec_parse_args(int *argc, char *argv[], RecArgs *recArgs)
{
    if (*argc <= 7)
    {
        return -1;
    }

    g_print("args\n");
    g_print("  video_packet_filename: %s\n", argv[1]);
    g_print("  video_payload_type: %s\n", argv[2]);
    g_print("  video_max_width: %s\n", argv[3]);
    g_print("  video_max_height: %s\n", argv[4]);
    g_print("  audio_packet_filename: %s\n", argv[5]);
    g_print("  audio_payload_type: %s\n", argv[6]);
    g_print("  output_filename: %s\n", argv[7]);

    recArgs->video_packet_filename = argv[1];
    recArgs->video_payload_type = atoi(argv[2]);
    recArgs->video_max_width = atoi(argv[3]);
    recArgs->video_max_height = atoi(argv[4]);
    recArgs->audio_packet_filename = argv[5];
    recArgs->audio_payload_type = atoi(argv[6]);
    recArgs->output_filename = argv[7];

    return 0;
}