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

Libva - https://github.com/01org/libva

GmmLib - https://github.com/intel/gmmlib

## Building

1. Build and install libva master [commit df544cd5a31e54d4cbd33a391795a8747ddaf789](https://github.com/01org/libva/commit/df544cd5a31e54d4cbd33a391795a8747ddaf789)
2. Get gmmlib and media repo and format the workspace folder as below:
```
<workspace>
    |- gmmlib
    |- media-driver
```
3. 
```
$ mkdir <workspace>/build
```
4. 
```
$ cd <workspace>/build
```
5. 
```
$ cmake ../media-driver \
-DCMAKE_INSTALL_PREFIX=/usr \
-DMEDIA_VERSION="2.0.0" \
-DBUILD_ALONG_WITH_CMRTLIB=1 \
-DBS_DIR_GMMLIB=`pwd`/../gmmlib/Source/GmmLib/ \
-DBS_DIR_COMMON=`pwd`/../gmmlib/Source/Common/ \
-DBS_DIR_INC=`pwd`/../gmmlib/Source/inc/ \
-DBS_DIR_MEDIA=`pwd`/../media-driver
```
Alternatively, copy 
```
<workspace>/media-driver/unified_cmake.sh
```
into
```
<workspace>/build
```
then run
```
$ ./unified_cmake.sh
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

| CODEC      | D/E | Platform(s)         |
|------------|-----|---------------------|
| H.264      |  D  | BDW/SKL/BXT/APL/CNL |
| H.264      |  E  | BDW/SKL/BXT/APL/CNL |
| MPEG-2     |  D  | BDW/SKL/BXT/APL/CNL |
| MPEG-2     |  E  | BDW/SKL/CNL         |
| VC-1       |  D  | BDW/SKL/BXT/APL/CNL |
| JPEG       |  D  | BDW/SKL/BXT/APL/CNL |
| JPEG       |  E  | SKL/BXT/APL/CNL     |
| VP8        |  D  | BDW/SKL/BXT/APL/CNL |
| VP8        |  E  | CNL                 |
| HEVC       |  D  | SKL/BXT/APL/CNL     |
| HEVC       |  E  | SKL/BXT/APL/CNL     |
| HEVC 10bit |  D  | BXT/APL/CNL         |
| HEVC 10bit |  E  | CNL                 |
| VP9        |  D  | BXT/APL/CNL         |
| VP9        |  E  | CNL                 |
| VP9 10bit  |  D  | CNL                 |


## Known Issues and Limitations

1. SKL: Green or other incorrect color will be observed in output frames when using
YV12/I420 as input format for csc/scaling/blending/rotation, etc. on Ubuntu 16.04 stock
(with kernel 4.10). The issue can be addressed with the kernel patch:
WaEnableYV12BugFixInHalfSliceChicken7 [commit 0b71cea29fc29bbd8e9dd9c641fee6bd75f6827](https://cgit.freedesktop.org/drm-tip/commit/?id=0b71cea29fc29bbd8e9dd9c641fee6bd75f68274)

2. CNL: Functionalities requiring HuC including AVC BRC for low power encoding, HEVC low power encoding, and VP9 low power encoding are pending on the kernel patch for GuC support which is expected in Q1’2018.

3. BXT/APL: Limited validation was performed; product quality expected in Q1’2018.

##### (*) Other names and brands may be claimed as property of others.

