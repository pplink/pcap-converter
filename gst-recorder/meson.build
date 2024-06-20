project('media-recorder', 'c',
    default_options : ['buildtype=release'])
add_project_arguments('-DNDEBUG', language : 'c')

gstreamer = dependency('gstreamer-1.0')

converter_inc = include_directories('converter')
converter_src = [
    'converter/main.c', 'converter/recmain.c', 'converter/recargs.c',
    'converter/recaudio.c', 'converter/recvideo.c'
    ]
executable('converter', converter_src,
    include_directories : converter_inc,
    dependencies : gstreamer)
