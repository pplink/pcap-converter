# Pcap Converter

## Development

- Build `gst-recorder`

  - Install dependencies
    - macOS
      - `$ brew install ninja`
      - `$ brew install gstreamer`
      - `$ pip3 install meson`
        - You need `pip3` to be installed
  - Build
    - `$ yarn build:gst`

- Install plugins
  - macOS
    - `$ brew install gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly`
    - homebrew's `gst-plugins-bad` currently does not include
      `voaacenc`. You need to do as following to install `libvo-aacenc`
      - `$ brew edit gst-plugins-bad`
      - Add `depends_on "libvo-aacenc"`
      - `$ brew reinstall --build-from-source gst-plugins-bad`

## Deployment

Refer to `Dockerfile` for configuration
