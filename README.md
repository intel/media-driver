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

## Build Modes

This section summarizes key driver build modes which can be used in the different environment requirements:

| Build option | Default value | Dependencies | Comments |
|-|-|-|-|
| ENABLE_KERNELS | ON | N/A | Enable/Disable shaders during driver build |
| ENABLE_NONFREE_KERNELS | ON | ENABLE_KERNELS=ON | Enable/disable close source shaders (kernels) during the build |
| BUILD_KERNELS | OFF | ENABLE_KERNELS=ON, ENABLE_NONFREE_KERNELS=OFF | If enabled, rebuild open source shaders (kernels) from sources. Requires ENABLE_NONFREE_KERNELS=OFF |

With the above options it is possible to build (table assumes that non-listed options have default values):

| No. | Build option(s) | HW Features | Shaders (Kernels) | Comments |
|-|-|-|-|-|
| 1 | ENABLE_KERNELS=ON | Yes | Close source (pre-built) | **Default**, full feature driver |
| 2 | ENABLE_KERNELS=OFF | Yes | None | HW features only, include HW decoder, HW VDEnc Encoder (CQP mode) |
| 3 | ENABLE_NONFREE_KERNELS=OFF | Yes | Open source (pre-built) | HW features available in prev. mode (ENABLE_KERNELS=ON) and features supported by open source shaders |
| 4 | ENABLE_NONFREE_KERNELS=OFF BUILD_KERNELS=ON | Yes | Open source | Same as ENABLE_NONFREE_KERNELS=OFF driver, but shaders are rebuilt from sources |

## Supported Platforms

BDW (Broadwell)

SKL (Skylake)

BXT (Broxton) / APL (Apollo Lake)

KBL (Kaby Lake)

CFL (Coffee Lake)

WHL (Whiskey Lake)

CNL (Cannonlake)

ICL (Ice Lake)

## Default Driver Build Features

| CODEC      | BDW  |  SKL   | BXT/APL |   KBL   |  CFL  |  WHL  | CNL  |   ICL*  |
|------------|------|--------|---------|---------|-------|-------|------|---------|
| AVC        | D/Es | D/E/Es | D/E/Es  | D/E/Es  | D/E/Es| D/E/Es| D/Es | D/E/Es  |
| MPEG-2     | D/Es |  D/Es  | D       | D/Es    | D/Es  | D/Es  | D/Es | D/Es    |
| VC-1       | D    |   D    | D       | D       | D     | D     | D    | D       |
| JPEG       | D    |   D/E  | D/E     | D/E     | D/E   | D/E   | D/E  | D/E     |
| VP8        | D    |   D    | D       | D       | D     | D/Es  | D/Es | D/Es    |
| HEVC 8bit  |      |   D/Es | D/Es    | D/Es    | D/Es  | D/Es  | D/Es | D/E/Es  |
| HEVC 10bit |      |        | D       | D       | D     | D     | D/Es | D/E/Es  |
| VP9 8bit   |      |        | D       | D       | D     | D     | D    | D/E     |
| VP9 10bit  |      |        |         | D       | D     | D     | D    | D/E     |

D  - HW Decoding

E  - HW Encoding

Es - HW + Shader Encoding

\* ICL encoding is pending on i915 support on upstream, for more information, please check [Known Issues and Limitations #5](https://github.com/intel/media-driver/blob/master/README.md#known-issues-and-limitations).


| Video Processing                             | BDW | SKL | BXT/APL | KBL | CFL | WHL | CNL | ICL |
|----------------------------------------------|-----|-----|---------|-----|-----|-----|-----|-----|
| Blending                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| CSC (Color Space Conversion)                 |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| De-interlace                                 |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| De-noise                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Luma Key                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Mirroring                                    |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| ProcAmp (brightness,contrast,hue,saturation) |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Rotation                                     |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Scaling                                      |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Sharpening                                   |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| STD/E (Skin Tone Detect & Enhancement)       |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| TCC (Total Color Control)                    |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Color fill                                   |  Y  |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| Chroma Siting                                |     |  Y  |    Y    |  Y  |  Y  |  Y  |  Y  |  Y  |
| HDR10 Tone Mapping                           |     |     |         |     |     |     |     |  Y  |

For detail feature information, you can access [Media Features](https://github.com/intel/media-driver/blob/master/docs/media_features.md).

## HW Media Features

| Media Features | BDW  | SKL | BXT/APL | KBL | CFL | WHL | CNL | ICL |
|----------------|------|-----|---------|-----|-----|-----|-----|-----|
| AVC            |   D  |  D  | D/E     | D/E | D/E | D/E |  D  | D/E |
| MPEG-2         |   D  |  D  | D       | D   | D   | D   |  D  | D   |
| VC-1           |   D  |  D  | D       | D   | D   | D   |  D  | D   |
| JPEG           |   D  | D/E | D/E     | D/E | D/E | D/E | D/E | D/E |
| VP8            |   D  |  D  | D       | D   | D   | D   |  D  | D   |
| HEVC 8bit      |      |  D  | D       | D   | D   | D   |  D  | D/E |
| HEVC 10bit     |      |     | D       | D   | D   | D   |  D  | D/E |
| VP9 8bit       |      |     | D       | D   | D   | D   |  D  | D/E |
| VP9 10bit      |      |     |         | D   | D   | D   |  D  | D/E |

D  - HW Decoding

E  - HW Encoding, VDEnc CQP mode only, BRC mode is pending on i915 support on upstream.


## Open Source Shader Media Features

| Media Features                               | BDW | SKL | BXT/APL | KBL | CFL | WHL | CNL | ICL |
|----------------------------------------------|-----|-----|---------|-----|-----|-----|-----|-----|
| Blending                                     |     |     |         |     |     |     |     |  Y  |
| CSC (Color Space Conversion)                 |     |     |         |     |     |     |     |  Y  |
| De-interlace                                 |     |     |         |     |     |     |     |  Y  |
| Luma Key                                     |     |     |         |     |     |     |     |  Y  |
| Mirroring                                    |     |     |         |     |     |     |     |  Y  |
| ProcAmp (brightness,contrast,hue,saturation) |     |     |         |     |     |     |     |  Y  |
| Rotation                                     |     |     |         |     |     |     |     |  Y  |
| Scaling                                      |     |     |         |     |     |     |     |  Y  |
| Sharpening                                   |     |     |         |     |     |     |     |  Y  |
| Color fill                                   |     |     |         |     |     |     |     |  Y  |
| Chroma Siting                                |     |     |         |     |     |     |     |  Y  |


## Close Source Shader Media Codec Features

All Open Source Shaders listed in the previous paragraph are still available, but Close Source Media Codec Shaders provide few additional features listed below.

| Encode Features | BDW | SKL | BXT/APL | KBL | CFL | WHL | CNL | ICL* |
|-----------------|-----|-----|---------|-----|-----|-----|-----|------|
| AVC             |  Es |  Es |   Es    |  Es |  Es |  Es |  Es |  Es  |
| MPEG-2          |  Es |  Es |         |  Es |  Es |  Es |  Es |  Es  |
| VP8             |     |     |         |     |     |  Es |  Es |  Es  |
| HEVC 8bit       |     |  Es |   Es    |  Es |  Es |  Es |  Es |  Es  |
| HEVC 10bit      |     |     |         |     |     |     |  Es |  Es  |

Es - HW + Shader Encoding

\* ICL encoding is pending on i915 support on upstream, for more information, please check [Known Issues and Limitations #5](https://github.com/intel/media-driver/blob/master/README.md#known-issues-and-limitations).




## Known Issues and Limitations

1. Intel(R) Media Driver for VAAPI is recommended to be built against gcc compiler v6.1
or later, which officially supported C++11.

2. SKL: Green or other incorrect color will be observed in output frames when using YV12/I420 as input format for csc/scaling/blending/rotation, etc. on Ubuntu 16.04 stock (with kernel 4.10). The issue can be addressed with the kernel patch: WaEnableYV12BugFixInHalfSliceChicken7 [commit 0b71cea29fc29bbd8e9dd9c641fee6bd75f6827](https://cgit.freedesktop.org/drm-tip/commit/?id=0b71cea29fc29bbd8e9dd9c641fee6bd75f68274)

3. HuC firmware is needed for AVC low power encoding bitrate control, including CBR, VBR, etc. As of now, HuC firmware support is disabled in Linux kernels by default. Please, refer to i915 kernel mode driver documentation to learn how to enable it. Mind that HuC firmware support presents in the following kernels for the specified platforms:
   * APL/KBL: starting from kernel 4.11
   * CFL: starting from kernel 4.15

4. ICL: preliminary support and may not be fully functional. It requires Linux kernel 4.17+ loaded with i915.alpha_support=1 parameter to be tried.

5. ICL encoding has two known issues:
   * Low power encoding: known [issue#328](https://github.com/intel/media-driver/issues/328) needs kernel patch which is under review.
   * VME based encoding: known [issue#267](https://github.com/intel/media-driver/issues/267), which requires media driver patch [PR#271](https://github.com/intel/media-driver/pull/271) and kernel driver patch [Per context dynamic (sub)slice power-gating](https://patchwork.freedesktop.org/series/48194/).


##### (*) Other names and brands may be claimed as property of others.
