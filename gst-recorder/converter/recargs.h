#ifndef __REC_ARGS_H__
#define __REC_ARGS_H__

typedef struct _RecArgs
{
    char *video_packet_filename;
    int video_payload_type;
    int video_max_width;
    int video_max_height;

    char *audio_packet_filename;
    int audio_payload_type;

    char* output_filename;
} RecArgs;

int rec_parse_args(int *argc, char *argv[], RecArgs *recArgs);

#endif