#include "recmain.h"
#include "recargs.h"

RecData data;

void rec_sig_handler(int signo)
{
    g_print("Receive SIGINT, finish converting.\n");
    g_main_loop_quit(data.main_loop);
}

int main(int argc, char *argv[])
{
    RecArgs args;
    if (rec_parse_args(&argc, argv, &args) != 0)
    {
        g_printerr("Cannot parse args.\n");
        g_print("Usage : converter <video packet filename> <video payload type> <video max width> <video max height> <audio packet filename> <video payload type> <output filename>\n");
        exit(1);
    }

    gst_init(&argc, &argv);
    signal(SIGINT, (void *)rec_sig_handler);

    if (rec_init(&data, &args) != 0)
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    if (rec_set_properties(&data) != 0)
    {
        g_printerr("Fail to set properties.\n");
        return -1;
    }

    if (rec_link(&data) != 0)
    {
        g_printerr("Fail to link elements.\n");
        return -1;
    }

    if (rec_start(&data) != 0)
    {
        g_printerr("Fail to start converting.\n");
        return -1;
    }

    return 0;
}
