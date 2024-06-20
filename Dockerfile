FROM ubuntu:18.04

# Prevent dialogs
ENV DEBIAN_FRONTEND=noninteractive

# Install Node 16.x
RUN apt-get update \
 && apt-get install -y curl \
 && curl -sL https://deb.nodesource.com/setup_16.x | bash - \
 && apt-get install -y nodejs

# Install dependencies (gstreamer and build tools)
RUN apt-get update && apt-get install -y awscli \
    libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio \
    python3-pip ninja-build
RUN apt-get install -y software-properties-common
RUN pip3 install meson

# Build gst-recorder
WORKDIR /media-infra/packages/media-converter/gst-recorder
COPY packages/media-converter/gst-recorder/build.sh .
COPY packages/media-converter/gst-recorder/meson.build .
COPY packages/media-converter/gst-recorder/converter converter/
RUN ./build.sh
