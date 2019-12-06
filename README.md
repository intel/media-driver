# Intel(R) Media Driver for VAAPI


## Introduction

The Intel(R) Media Driver for VAAPI is a new VA-API (Video Acceleration API)
user mode driver supporting hardware accelerated decoding, encoding, and
video post processing for GEN based graphics hardware.

## License

The Intel(R) Media Driver for VAAPI is distributed under the MIT license with
portions covered under the BSD 3-clause "New" or "Revised" License.
You may obtain a copy of the License at [MIT](https://opensource.org/licenses/MIT) & [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

## Prerequisites

For Ubuntu 16.04+

```
apt install autoconf libtool libdrm-dev xorg xorg-dev openbox libx11-dev libgl1-mesa-glx libgl1-mesa-dev
```

Equivalents for other distributions should work.

## Dependencies

* [Libva](https://github.com/intel/libva)
* [GmmLib](https://github.com/intel/gmmlib)(Please check [comparability with GmmLib](https://github.com/intel/media-driver/wiki/Comparability-with-GmmLib))

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
$ make -j"$(nproc)"
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

* BDW (Broadwell)
* SKL (Skylake)
* BXT (Broxton) / APL (Apollo Lake)
* KBLx (KBL/Kaby Lake; CFL/Coffe Lake; WHL/Whiskey Lake; CML/Comet Lake; AML/Amber Lake)
* ICL (Ice Lake)
* JSL (Jasper Lake)/EHL (Elkhart Lake)
* TGL (Tiger Lake)


## Components and Features

Media driver contains three components as below
* **Video decoding** calls hardware-based decoder([VDBox](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol08-media_vdbox.pdf))  which provides fully-accelerated hardware video decoding to release the graphics engine for other operations.
* **Video encoding** supports two modes, one calls hardware-based encoder([VDEnc](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol08-media_vdbox.pdf)/[Huc](https://01.org/linuxgraphics/downloads/firmware?langredirect=1)) to provide low power encoding, another one is hardware([PAK](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol08-media_vdbox.pdf))+shader(media kernel+[VME](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol04-configurations.pdf)) based encoding. User could choose the mode through VA-API.
* **Video processing** supports several popular features by hardware-based video processor([VEBox/SFC](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf)) and shader(media kernel) based solution together.

Media driver supports two build types as below
* **Full Feature Build** is default driver build, which supports all feature by hardware accelerator and close source shaders(media kernel binaries). Ubuntu [intel-media-va-driver-non-free](https://packages.ubuntu.com/disco/intel-media-va-driver-non-free) package is generated from this build type.
* **Free Kernel Build**, enables fully open source shaders(media kernels) and hardware features but the features would be limited. Ubuntu [intel-media-va-driver](https://packages.ubuntu.com/disco/intel-media-va-driver) package is generated from this build type.


### Decoding/Encoding Features

| CODEC | Build Types | BDW | SKL | BXT/APL | KBLx | ICL | EHL/JSL | TGL* |
|---|---|---|---|---|---|---|----|---|
| AVC | <u>Full Feature</u><br><i>Free Kernel</i> | <u>D/Es</u><br><i>D<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i>|
| MPEG-2 | <u>Full Feature</u><br><i>Free Kernel</i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> |<u>D/Es</u><br><i>D<i>|
| VC-1 | <u>Full Feature</u><br><i>Free Kernel</i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> |<u>D</u><br><i>&nbsp;<i>|
| JPEG | <u>Full Feature</u><br><i>Free Kernel</i> | <u>D</u><br><i>D<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E</u><br><i>D/E<i>|
| VP8 | <u>Full Feature</u><br><i>Free Kernel</i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> |<u>D</u><br><i>D<i> |
| HEVC 8bit | <u>Full Feature</u><br><i>Free Kernel</i> |  | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E/Es</u><br><i>D/E<i> |
| HEVC 8bit 422 | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  |  | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> |<u>D/Es</u><br><i>D<i>|
| HEVC 8bit 444 | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  |  | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |
| HEVC 10bit | <u>Full Feature</u><br><i>Free Kernel</i> |  |  | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> |
| HEVC 10bit 422 | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  |  | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i>| <u>D/Es</u><br><i>D<i> |
| HEVC 10bit 444 | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  |  | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E</u><br><i>D/E<i> |
| VP9 8bit | <u>Full Feature</u><br><i>Free Kernel</i> |  |  | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E</u><br><i>D/E<i> |
| VP9 8bit 444 | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  |  | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E</u><br><i>D/E<i> |
| VP9 10bit | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  | <u>D</u><br><i>D<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E</u><br><i>D/E<i> |
| VP9 10bit 444 | <u>Full Feature</u><br><i>Free Kernel</i> |  |  |  |  | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |<u>D/E</u><br><i>D/E<i> |

* D  - Hardware Decoding
* E  - Hardware Encoding, Low Power Encoding(VDEnc/Huc)
* Es - Hardware(PAK) + Shader(media kernel+VME) Encoding

\* TGL encoding is pending on i915 support on upstream, for more information, please check [Known Issues and Limitations #4, #5](https://github.com/intel/media-driver/blob/master/README.md#known-issues-and-limitations).

For more information, please refer to
* [Media Features Summary](https://github.com/intel/media-driver/blob/master/docs/media_features.md#media-features-summary)
    * [Supported Decoding Output Format and Max Resolution](https://github.com/intel/media-driver/blob/master/docs/media_features.md#supported-decoding-output-format-and-max-resolution)
    * [Supported Encoding Input Format and Max Resolution](https://github.com/intel/media-driver/blob/master/docs/media_features.md#supported-encoding-input-format-and-max-resolution)

### Video Processing Features

| Features | Build Types | BDW | SKL | BXT/APL | KBLx | ICL |EHL/JSL |TGL |
|---|---|---|---|---|----|---|---|---|
| Blending | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| CSC<br>(Color Space Conversion) | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| De-interlace | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i>  | <u>Yes</u><br><i>Yes</i> |<u>Yes*</u><br><i>Yes*</i> |<u>Yes</u><br><i>Yes</i> |
| De-noise | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |<u>No</u><br><i>&nbsp;</i> |<u>Yes</u><br><i>&nbsp;</i> |
| Luma Key | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| Mirroring | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| ProcAmp<br>(brightness,contrast,hue,saturation) | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| Rotation | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| Scaling | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| Sharpening | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>No</u><br><i>No</i> |<u>Yes</u><br><i>Yes</i> |
| STD/E<br>(Skin Tone Detect & Enhancement) | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |<u>No</u><br><i>&nbsp;</i> |<u>Yes</u><br><i>&nbsp;</i> |
| TCC<br>(Total Color Control) | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |<u>No</u><br><i>&nbsp;</i> |<u>Yes</u><br><i>&nbsp;</i> |
| Color fill | <u>Full Feature</u><br><i>Free Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| Chroma Siting | <u>Full Feature</u><br><i>Free Kernel</i> | &nbsp;<br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |<u>Yes</u><br><i>Yes</i> |
| HDR10 Tone Mapping | <u>Full Feature</u><br><i>Free Kernel</i> | &nbsp;<br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i>    | <u>Yes</u><br><i>&nbsp;</i> |<u>No</u><br><i>&nbsp;</i> |<u>Yes</u><br><i>&nbsp;</i> |

For more feature information, please refer to [Supported video processing csc/scaling format](https://github.com/intel/media-driver/blob/master/docs/media_features.md#supported-video-processing-cscscaling-format)


### Build Options

You could follow below build options to enable these two builds.
* **Full Feature Build**: ENABLE_KERNELS=ON(Default) ENABLE_NONFREE_KERNELS=ON(Default)
* **Free Kernel Build**: ENABLE_KERNELS=ON ENABLE_NONFREE_KERNELS=OFF
    * If trying to use pre-built open source kernel binaries, please add BUILD_KERNELS=OFF(Default).
    * If trying to rebuild open source kernel from source code, please add BUILD_KERNELS=ON.

\* EHL/JSL only support BOB DI

## Known Issues and Limitations

1. Intel(R) Media Driver for VAAPI is recommended to be built against gcc compiler v6.1
or later, which officially supported C++11.

2. SKL: Green or other incorrect color will be observed in output frames when using YV12/I420 as input format for csc/scaling/blending/rotation, etc. on Ubuntu 16.04 stock (with kernel 4.10). The issue can be addressed with the kernel patch: [WaEnableYV12BugFixInHalfSliceChicken7](https://cgit.freedesktop.org/drm-tip/commit/?id=0b71cea29fc29bbd8e9dd9c641fee6bd75f68274)

3. HuC firmware is needed for AVC/HEVC/VP9 low power encoding bitrate control, including CBR, VBR, etc. As of now, HuC firmware support is disabled in Linux kernels by default. Please, refer to i915 kernel mode driver documentation to learn how to enable it. Mind that HuC firmware support presents in the following kernels for the specified platforms:
   * APL/KBL: starting from kernel 4.11
   * CFL: starting from kernel 4.15
   * ICL: starting from kernel 5.2
   * EHL/JSL/TGL: [drm-tip](https://cgit.freedesktop.org/drm-tip), missed in <=5.4.3 kernels

4. TGL: preliminary support and may not be fully functional. It requires Linux kernel 5.4+ loaded with i915.force_probe=* parameter to be tried. [drm-tip] (https://cgit.freedesktop.org/drm-tip) should work with more support and bug fixes.

5. TGL VME encode has known issue [112377](https://bugs.freedesktop.org/show_bug.cgi?id=112377) with [drm-tip](https://cgit.freedesktop.org/drm-tip)

##### (*) Other names and brands may be claimed as property of others.
