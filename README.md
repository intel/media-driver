# Intel(R) Media Driver for VAAPI


## Introduction

The Intel(R) Media Driver for VAAPI is a new VA-API (Video Acceleration API)
user mode driver supporting hardware accelerated decoding, encoding, and
video post processing for GEN based graphics hardware.

## License

The Intel(R) Media Driver for VAAPI is distributed under the MIT license with
portions covered under the BSD 3-clause "New" or "Revised" License.
You may obtain a copy of the License at [MIT](https://opensource.org/licenses/MIT) & [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

## Building
For Ubuntu 16.04+

```
apt install autoconf libtool libdrm-dev xorg xorg-dev openbox libx11-dev libgl1-mesa-glx libgl1-mesa-dev
```

Equivalents for other distributions should work.

1. Build and install [LibVA](https://github.com/intel/libva)
2. Build and install [GmmLib](https://github.com/intel/gmmlib) following [GmmLib compatibility](https://github.com/intel/media-driver/wiki/Compatibility-with-GmmLib)
3. Get media repo and format the workspace folder as below (suggest the workspace to be a dedicated one for media driver build):
```
<workspace>
    |- media-driver
```
4. Create build_media new folder under your workspace
```
$ mkdir <workspace>/build_media
```
then the workspace looks like below
```
<workspace>
    |- media-driver
    |- build_media
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

- BDW (Broadwell)
- SKL (Skylake)
- BXTx (BXT: Broxton, APL: Apollo Lake, GLK: Gemini Lake)
- KBLx (KBL: Kaby Lake, CFL: Coffe Lake, WHL: Whiskey Lake, CML: Comet Lake, AML: Amber Lake)
- ICL (Ice Lake)
- JSL (Jasper Lake) / EHL (Elkhart Lake)
- TGLx (TGL: Tiger Lake, RKL: Rocket Lake, ADL-S/P/N: Alder Lake)
- DG1/SG1
- Alchemist(DG2)/ATSM


## Components and Features

Media driver contains three components as below
- **Video decoding** calls hardware-based decoder([VDBox](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol08-media_vdbox.pdf))  which provides fully-accelerated hardware video decoding to release the graphics engine for other operations.
- **Video encoding** supports two modes, one calls hardware-based encoder([VDEnc](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol08-media_vdbox.pdf)/[Huc](https://01.org/linuxgraphics/downloads/firmware?langredirect=1)) to provide low power encoding, another one is hardware([PAK](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol08-media_vdbox.pdf))+shader(media kernel+[VME](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol04-configurations.pdf)) based encoding. User could choose the mode through VA-API.
- **Video processing** supports several popular features by hardware-based video processor([VEBox/SFC](https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf)) and shader(media kernel) based solution together.

Media driver supports two build types as below
- **Full Feature Build** is default driver build, which supports all feature by hardware accelerator and close source shaders(media kernel binaries). Ubuntu [intel-media-va-driver-non-free](https://packages.ubuntu.com/disco/intel-media-va-driver-non-free) package is generated from this build type.
- **Free Kernel Build**, enables fully open source shaders(media kernels) and hardware features but the features would be limited. Ubuntu [intel-media-va-driver](https://packages.ubuntu.com/disco/intel-media-va-driver) package is generated from this build type.


### Decoding/Encoding Features


| CODEC | Build Types | DG2/ATSM | DG1/SG1 | TGLx | EHL/JSL | ICL | KBLx | BXTx | SKL | BDW |
|---|---|---|---|---|---|---|---|---|---|---|
| AVC | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/Es</u><br><i>D<i> |
| MPEG-2 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> |
| VC-1 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>&nbsp;</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> | <u>D</u><br><i>&nbsp;<i> |
| JPEG | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D</u><br><i>D<i> |
| VP8 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>&nbsp;</u><br><i>&nbsp;<i> | <u>&nbsp;</u><br><i>&nbsp;<i> | <u>D*</u><br><i>D*<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |
| HEVC 8bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> |  |
| HEVC 8bit 422 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> |  |  |  |  |
| HEVC 8bit 444 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |  |  |  |  |
| HEVC 10bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E/Es</u><br><i>D/E<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |
| HEVC 10bit 422 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> |  |  |  |  |
| HEVC 10bit 444 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |  |  |  |  |
| HEVC 12bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> | <u>D/Es</u><br><i>D<i> |  |  |  |  |  |  |
| HEVC 12bit 422 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |  |  |  |  |
| HEVC 12bit 444 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |  |  |  |  |
| VP9 8bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |
| VP9 8bit 444 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |  |  |  |  |
| VP9 10bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D</u><br><i>D<i> |  |  |  |
| VP9 10bit 444 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> | <u>D/E</u><br><i>D/E<i> |  |  |  |  |
| VP9 12bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |  |  |  |  |
| VP9 12bit 444 | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |  |  |  |  |
| AV1 8bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |  |  |  |  |
| AV1 10bit | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>D/E</u><br><i>D/E<i> | <u>D</u><br><i>D<i> | <u>D</u><br><i>D<i> |  |  |  |  |  |   |


- \* VP8 decoding is only supported on TGL platform
- D  - Hardware Decoding
- E  - Hardware Encoding, Low Power Encoding(VDEnc/Huc)
- Es - Hardware(PAK) + Shader(media kernel+VME) Encoding



For more decoding and encoding features information, please refer to
- [Media Features Summary](https://github.com/intel/media-driver/blob/master/docs/media_features.md#media-features-summary)
    - [Supported Decoding Output Format and Max Resolution](https://github.com/intel/media-driver/blob/master/docs/media_features.md#supported-decoding-output-format-and-max-resolution)
    - [Supported Encoding Input Format and Max Resolution](https://github.com/intel/media-driver/blob/master/docs/media_features.md#supported-encoding-input-format-and-max-resolution)

### Video Processing Features


| CODEC | Build Types | DG2/ATSM | DG1/SG1 | TGLx | EHL/JSL | ICL | KBLx | BXTx | SKL | BDW |
|---|---|---|---|---|---|---|---|---|---|---|
| Blending | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| CSC | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| De-interlace | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes*</u><br><i>Yes*</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| De-noise | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Luma Key | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Mirroring | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| ProcAmp | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Rotation | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Scaling | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Sharpening | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| STD/E | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| TCC | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Color fill | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> |
| Chroma Siting | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>Yes</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> |
| HDR10 TM | <u>Full-Feature</u><br><i>Free-Kernel</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | <u>Yes</u><br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> | &nbsp;<br><i>&nbsp;</i> |

- \* EHL/JSL only support BOB DI
- CSC: Color Space Conversion
- ProcAmp: brightness,contrast,hue,saturation
- STD/E: Skin Tone Detect & Enhancement
- TCC: Total Color Control
- HDR10 TM: HDR10 Tone Mapping


For more feature information, please refer to [Supported video processing csc/scaling format](https://github.com/intel/media-driver/blob/master/docs/media_features.md#supported-video-processing-cscscaling-format)


## Build Options

Media-driver supports few build types as described below. You could refer to
the following settings to enable them.
- **Full Feature Build**: ENABLE_KERNELS=ON(Default) ENABLE_NONFREE_KERNELS=ON(Default)
- **Free Kernel Build**: ENABLE_KERNELS=ON ENABLE_NONFREE_KERNELS=OFF
    - If trying to use pre-built open source kernel binaries, please add BUILD_KERNELS=OFF(Default).
    - If trying to rebuild open source kernel from source code, please add BUILD_KERNELS=ON.

Media-driver requires special i915 kernel mode driver (KMD) version to support the following
new platforms since upstream version of i915 KMD does not fully support them
(pending patches upstream):
- DG1/SG1
- Alchemist(DG2)/ATSM

By default, media-driver builds against upstream i915 KMD and will miss
support for the platforms listed above. To enable new platforms which
require special i915 KMD and specify `ENABLE_PRODUCTION_KMD=ON` (default: `OFF`)
build configuration option.

## Known Issues and Limitations

1. Intel(R) Media Driver for VAAPI is recommended to be built against gcc compiler v6.1
or later, which officially supported C++11.

2. SKL: Green or other incorrect color will be observed in output frames when using YV12/I420 as input format for csc/scaling/blending/rotation, etc. on Ubuntu 16.04 stock (with kernel 4.10). The issue can be addressed with the kernel patch: [WaEnableYV12BugFixInHalfSliceChicken7](https://cgit.freedesktop.org/drm-tip/commit/?id=0b71cea29fc29bbd8e9dd9c641fee6bd75f68274)

3. HuC firmware is necessary for AVC/HEVC/VP9 low power encoding bitrate control, including CBR, VBR, etc. As of now, HuC firmware support is disabled in Linux kernels by default. kernel command line option "i915.enable_guc=2" for HuC loading can be set e.g. by doing echo "options i915 enable_guc=2" > /etc/modprobe.d/i915.conf (as root). Mind that HuC firmware support presents in the following kernels for the specified platforms:
   - APL/KBL: starting from kernel 4.11
   - CFL: starting from kernel 4.15
   - ICL: starting from kernel 5.2
   - EHL/JSL: starting from kernel 5.8
   - TGL: starting from kernel 5.9
   - RKL/ADL-S/ADL-P: starting from kernel 5.17
   - ADL-N: [drm-tip](https://cgit.freedesktop.org/drm-tip)
   - DG1/SG1: [intel-gpu/kernel](https://github.com/intel-gpu/kernel)

##### (*) Other names and brands may be claimed as property of others.
