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
2. Build and install gmmlib master
3. Get media repo and format the workspace folder as below (suggest the workspace to be a dedicated one for media driver build):
```
<workspace>
    |- media-driver
```
4. 
```
$ mkdir <workspace>/build_media
```
5. 
```
$ cd <workspace>/build_media
```
6. 
```
$ cmake ../media-driver
```
7. 
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

BXT (Broxton) / APL (Apollo Lake)

KBL (Kaby Lake)

CFL (Coffee Lake)

CNL (Cannonlake)

ICL (Ice Lake)

## Supported Codecs

| CODEC      | BDW  | SKL  | BXT/APL |   KBL   |   CFL   | CNL  |   ICL*  |
|------------|------|------|---------|---------|---------|------|---------|
| AVC        | D/E1 | D/E1 | D/E1/E2 | D/E1/E2 | D/E1/E2 | D/E1 | D/E1/E2 |
| MPEG-2     | D/E1 | D/E1 | D       | D/E1    | D/E1    | D/E1 | D/E1    |
| VC-1       | D    | D    | D       | D       | D       | D    | D       |
| JPEG       | D    | D/E2 | D/E2    | D/E2    | D/E2    | D/E2 | D/E2    |
| VP8        | D    | D    | D       | D       | D       | D/E1 | D/E1    |
| HEVC 8bit  |      | D/E1 | D/E1    | D/E1    | D/E1    | D/E1 | D/E1/E2 |
| HEVC 10bit |      |      | D       | D       | D       | D/E1 | D/E1/E2 |
| VP9 8bit   |      |      | D       | D       | D       | D    | D/E2    |
| VP9 10bit  |      |      |         | D       | D       | D    | D/E2    |

D  - decoding

E1 - VME based encoding

E2 - Low power encoding

\* ICL encoding is pending on i915 support on upstream, for more information, please check [Known Issues and Limitations #5](https://github.com/intel/media-driver/blob/master/README.md#known-issues-and-limitations).


## Supported Decoding Features

Supported decoding output format and max resolution: 

(2k=2048x2048, 4k=4096x4096, 8k=8192x8192, 16k=16384x16384)

| Codec      | Type     | BDW  | SKL  | BXT/APL | KBL  | CFL  | CNL  | ICL            |
|------------|----------|------|------|---------|------|------|------|----------------|
| AVC        |Output    | NV12 | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12           |
|            |Max Res.  | 4k   | 4k   |  4k     | 4k   | 4k   | 4k   | 4k             |
| MPEG-2     |Output    | NV12 | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12           |
|            |Max Res.  | 2k   | 2k   |  2k     | 2k   | 2k   | 2k   | 2k             |
| VC-1       |Output    | NV12 | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12           |
|            |Max Res.  | 4k   | 4k   |  4k     | 4k   | 4k   | 4k   | 4k             |
| JPEG*      |Max Res.  | 16k  | 16k  |  16k    | 16k  | 16k  | 16k  | 16k            |
| VP8        |Output    | NV12 | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12           |
|            |Max Res.  | 4k   | 4k   |  4k     | 4k   | 4k   | 4k   | 4k             |
| HEVC 8bit  |Output    |      | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12/YUY2/AYUV |
|            |Max Res.  |      | 8k   |  8k     | 8k   | 8k   | 8k   | 8k             |
| HEVC 10bit |Output    |      |      |  P010   | P010 | P010 | P010 | P010/Y210/Y410 |
|            |Max Res.  |      |      |  8k     | 8k   | 8k   | 8k   | 8k             |
| VP9  8bit  |Output    |      |      |  NV12   | NV12 | NV12 | NV12 | NV12/AYUV      |
|            |Max Res.  |      |      |  4k     | 8k   | 8k   | 8k   | 8k             |
| VP9  10bit |Output    |      |      |         | P010 | P010 | P010 | P010/Y410      |
|            |Max Res.  |      |      |         | 8k   | 8k   | 8k   | 8k             |

\* JPEG output format: NV12/411P/422H/422V/444P/BGRP/RGBP/YUY2/ARGB


## Supported Encoding Features

### Low Power Encoding:

Supported input format and max resoultuion: 

(4k=4096x4096, 16k=16384x16384)

| Codec      | Type       | BDW  | SKL  | BXT/APL | KBL  | CFL  | CNL  | ICL***         |
|------------|------------|------|------|---------|------|------|------|----------------|
| AVC        |Input       |      |      |  NV12   | More*| More*|      | More*          |
|            |Max Res.    |      |      |  4k     | 4k   | 4k   |      | 4k             |
| JPEG       |Input/Output|      |Note**| Note**  |Note**|Note**|Note**| Note**         |
|            |Max Res.    |      | 16k  |  16k    | 16k  | 16k  | 16k  | 16k            |
| HEVC 8bit  |Input       |      |      |         |      |      |      | NV12/AYUV      |
|            |Max Res.    |      |      |         |      |      |      | 8K             |
| HEVC 10bit |Input       |      |      |         |      |      |      | P010/Y410      |
|            |Max Res.    |      |      |         |      |      |      | 8k             |
| VP9  8bit  |Input       |      |      |         |      |      |      | NV12/AYUV      |
|            |Max Res.    |      |      |         |      |      |      | 8k             |
| VP9  10bit |Input       |      |      |         |      |      |      | P010/Y410      |
|            |Max Res.    |      |      |         |      |      |      | 8k             |

\* KBL/CFL/ICL AVC encoding supported input formats: NV12/YUY2/YUYV/YVYU/UYVY/AYUV/ARGB

\** JPEG encoding supports input format NV12/YUY2/UYVY/AYUV/ABGR/Y8 and output format YUV400/YUV420/YUV422H_2Y/YUV444/RGB24. 

\*** ICL encoding is pending on i915 support on upstream, for more information, please check [Known Issues and Limitations #5](https://github.com/intel/media-driver/blob/master/README.md#known-issues-and-limitations).


### VME Based Encoding:

Supported input format and max resolution: 

(2k=2048x2048, 4k=4096x4096, 8k=8192x8192)

| Codec      | Type       | BDW  | SKL  | BXT/APL | KBL  | CFL  | CNL  | ICL*           |
|------------|------------|------|------|---------|------|------|------|----------------|
| AVC        |Input       | NV12 | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12           |
|            |Max Res.    | 4k   | 4k   |  4k     | 4k   | 4k   | 4k   | 4k             |
| MPEG2      |Input       | NV12 | NV12 |         | NV12 | NV12 | NV12 | NV12           |
|            |Max Res.    | 2k   | 2k   |         | 2k   | 2k   | 2k   | 2k             |
| VP8        |Input       |      |      |         |      |      | NV12 | NV12           |
|            |Max Res.    |      |      |         |      |      | 4k   | 4k             |
| HEVC 8bit  |Input       |      | NV12 |  NV12   | NV12 | NV12 | NV12 | NV12/AYUV      |
|            |Max Res.    |      | 8k   |  8k     | 8k   | 8k   | 8k   | 8k             |
| HEVC 10bit |Input       |      |      |         |      |      | NV12 | P010/Y410      |
|            |Max Res.    |      |      |         |      |      | 8k   | 8k             |

\* ICL encoding is pending on i915 support on upstream, for more information, please check [Known Issues and Limitations #5](https://github.com/intel/media-driver/blob/master/README.md#known-issues-and-limitations).

## Supported Video Processing

### Supported Feature List

| Video Processing                             | BDW | SKL | BXT/APL | KBL | CFL | CNL | ICL |
|----------------------------------------------|-----|-----|---------|-----|-----|-----|-----|
| Blending                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| CSC (Color Space Conversion)                 |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| De-interlace                                 |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| De-noise                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Luma Key                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Mirroring                                    |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| ProcAmp (brightness,contrast,hue,saturation) |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Rotation                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Scaling                                      |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Sharpening                                   |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| STD/E (Skin Tone Detect & Enhancement)       |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| TCC (Total Color Control)                    |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Color fill                                   |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |
| Chroma Siting                                |  N  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |

### CSC/Scaling Format List

|    Platform       | Format | NV12 | YV12 | I420 | P010 | YUY2 | UYVY | Y210 | AYUV | Y410 |
|-------------------|--------|------|------|------|------|------|------|------|------|------|
|      BDW          | Input  |  Y   |  Y   |  Y   |      |  Y   |      |      |      |      |
|                   | Output |  Y   |  Y   |  Y   |      |  Y   |      |      |      |      |
|SKL/BXT/APL/KBL/CFL| Input  |  Y   |  Y   |  Y   |  Y   |  Y   |      |      |      |      |
|                   | Output |  Y   |  Y   |  Y   |      |  Y   |      |      |      |      |
|      ICL          | Input  |  Y   |  Y   |  Y   |  Y   |  Y   |  Y   |  Y   |  Y   |  Y   |
|                   | Output |  Y   |  Y   |  Y   |  Y   |  Y   |      |  Y   |  Y   |  Y   |


## Known Issues and Limitations

1. Intel(R) Media Driver for VAAPI is recommended to be built against gcc compiler v6.1
or later, which officially supported C++11.

2. SKL: Green or other incorrect color will be observed in output frames when using
YV12/I420 as input format for csc/scaling/blending/rotation, etc. on Ubuntu 16.04 stock
(with kernel 4.10). The issue can be addressed with the kernel patch:
WaEnableYV12BugFixInHalfSliceChicken7 [commit 0b71cea29fc29bbd8e9dd9c641fee6bd75f6827](https://cgit.freedesktop.org/drm-tip/commit/?id=0b71cea29fc29bbd8e9dd9c641fee6bd75f68274)

3. HuC firmware is needed for AVC low power encoding bitrate control, including CBR, VBR, etc. As of now, HuC firmware support is disabled in Linux kernels by default. Please, refer to i915 kernel mode driver documentation to learn how to enable it. Mind that HuC firmware support presents in the following kernels for the specified platforms:
   * APL/KBL: starting from kernel 4.11
   * CFL: starting from kernel 4.15

4. ICL: preliminary support and may not be fully functional. It requires Linux kernel 4.17+ loaded with i915.alpha_support=1 parameter to be tried.

5. ICL encoding has two known issues:
   * Low power encoding: known [issue#328](https://github.com/intel/media-driver/issues/328) needs kernel patch which is under review.
   * VME based encoding: known [issue#267](https://github.com/intel/media-driver/issues/267), which requires media driver patch [PR#271](https://github.com/intel/media-driver/pull/271) and kernel driver patch [Per context dynamic (sub)slice power-gating](https://patchwork.freedesktop.org/series/48194/).


##### (*) Other names and brands may be claimed as property of others.

