# Intel(R) Media Driver for VAAPI


## Introduction

The Intel(R) Media Driver for VAAPI is a new VA-API (Video Acceleration API)
user mode driver supporting hardware accelerated decoding, encoding, and
video post processing for GEN based graphics hardware.

## License

The Intel(R) Media Driver for VAAPI is distributed under the MIT license with
portions covered under the BSD 3-clause "New" or "Revised" License.
You may obtain a copy of the License at:

https://opensource.org/licenses/MIT

&

https://opensource.org/licenses/BSD-3-Clause

## Prerequisites

For Ubuntu 16.04 and above

\# apt install autoconf libtool libdrm-dev xorg xorg-dev openbox libx11-dev libgl1-mesa-glx libgl1-mesa-dev

Equivalents for other distributions should work.

## Dependencies

Libva - https://github.com/intel/libva

GmmLib - https://github.com/intel/gmmlib (please check https://github.com/intel/media-driver/wiki/Comparability-with-GmmLib)

## Building

1. Build and install libva master
2. Get gmmlib and media repo and format the workspace folder as below (suggest the workspace to be a dedicated one for media driver build):
```
<workspace>
    |- gmmlib
    |- media-driver
```
3. 
```
$ mkdir <workspace>/build_media
```
4. 
```
$ cd <workspace>/build_media
```
5. 
```
$ cmake ../media-driver
```
6. 
```
$ make -j8
```

## Install

```
$ sudo make install
```
This will install the following files (e.g. on Ubuntu):
```
-- Installing: /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
-- Installing: /etc/profile.d/intel-media.sh
-- Installing: /usr/lib/x86_64-linux-gnu/igfxcmrt64.so
```

For iHD_drv_video.so please export related LIBVA environment variables.
```
export LIBVA_DRIVERS_PATH=<path-contains-iHD_drv_video.so>
export LIBVA_DRIVER_NAME=iHD
```

## Supported Platforms

BDW (Broadwell)

SKL (Skylake)

BXT (Broxton) / APL (Apollolake)

CNL (Cannonlake)

## Supported Codecs

| CODEC      | BDW | SKL | BXT/APL | CNL |
|------------|-----|-----|---------|-----|
| H.264      | D/E | D/E |   D/E   | D/E |
| MPEG-2     | D/E | D/E |    D    | D/E |
| VC-1       |  D  |  D  |    D    |  D  |
| JPEG       |  D  | D/E |   D/E   | D/E |
| VP8        |  D  |  D  |    D    | D/E |
| HEVC       |     | D/E |   D/E   | D/E |
| HEVC 10bit |     |     |    D    | D/E |
| VP9        |     |     |    D    | D/E |
| VP9 10bit  |     |     |         |  D  |


## Known Issues and Limitations

1. Intel(R) Media Driver for VAAPI is recommended to be built against gcc compiler v6.1
or later, which officially supported C++11.

2. SKL: Green or other incorrect color will be observed in output frames when using
YV12/I420 as input format for csc/scaling/blending/rotation, etc. on Ubuntu 16.04 stock
(with kernel 4.10). The issue can be addressed with the kernel patch:
WaEnableYV12BugFixInHalfSliceChicken7 [commit 0b71cea29fc29bbd8e9dd9c641fee6bd75f6827](https://cgit.freedesktop.org/drm-tip/commit/?id=0b71cea29fc29bbd8e9dd9c641fee6bd75f68274)

3. APL: BRC functionalities requiring HuC for AVC low power encoding require 4.11 or
later kernel to work.

4. CNL: Functionalities requiring HuC including AVC BRC for low power encoding, HEVC low
power encoding, and VP9 low power encoding require the kernel patch for GuC support to work.

5. CNL: HEVC encoding does not support P frame.

##### (*) Other names and brands may be claimed as property of others.

