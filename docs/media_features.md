# Media Features Summary

## Supported Decoding Output Format and Max Resolution

| Codec      | Type     | DG2/ATSM       | DG1/SG1        | TGLx           | ICL            | KBLx | BXTx | SKL  | BDW  |
|------------|--------- |----------------|----------------|----------------|----------------|------|------|------|------|
| AVC        | Output   | NV12           | NV12           | NV12           | NV12           | NV12 | NV12 | NV12 | NV12 |
|            | Max Res. | 4k             | 4k             | 4k             | 4k             | 4k   | 4k   | 4k   | 4k   |
| MPEG-2     | Output   | NV12           | NV12           | NV12           | NV12           | NV12 | NV12 | NV12 | NV12 |
|            | Max Res. | 2k             | 2k             | 2k             | 2k             | 2k   | 2k   | 2k   | 2k   |
| VC-1       | Output   |                | NV12           | NV12           | NV12           | NV12 | NV12 | NV12 | NV12 |
|            | Max Res. |                | 4k             | 4k             | 4k             | 4k   | 4k   | 4k   | 4k   |
| VP8        | Output   |                |                | NV12*          | NV12           | NV12 | NV12 | NV12 | NV12 |
|            | Max Res. |                |                | 4k*            | 4k             | 4k   | 4k   | 4k   | 4k   |
| HEVC 8bit  | Output   | NV12/YUY2/AYUV | NV12/YUY2/AYUV | NV12/YUY2/AYUV | NV12/YUY2/AYUV | NV12 | NV12 | NV12 |      |
|            | Max Res. | 16k            | 8k             | 8k             | 8k             | 8k   | 8k   | 8k   |      |
| HEVC 10bit | Output   | P010/Y210/Y410 | P010/Y210/Y410 | P010/Y210/Y410 | P010/Y210/Y410 | P010 | P010 |      |      |
|            | Max Res. | 16k            | 8k             | 8k             | 8k             | 8k   | 8k   |      |      |
| HEVC 12bit | Output   | P016/Y216/Y416 | P016/Y216/Y416 | P016/Y216/Y416 |                |      |      |      |      |
|            | Max Res. | 16k            | 8k             | 8k             |                |      |      |      |      |
| VP9 8bit   | Output   | NV12/AYUV      | NV12/AYUV      | NV12/AYUV      | NV12/AYUV      | NV12 | NV12 |      |      |
|            | Max Res. | 16k            | 8k             | 8k             | 8k             | 8k   | 4k   |      |      |
| VP9 10bit  | Output   | P010/Y410      | P010/Y410      | P010/Y410      | P010/Y410      | P010 |      |      |      |
|            | Max Res. | 16k            | 8k             | 8k             | 8k             | 8k   |      |      |      |
| VP9 12bit  | Output   | P016/Y216/Y416 | P016/Y216/Y416 | P016/Y216/Y416 |                |      |      |      |      |
|            | Max Res. | 16k            | 8k             | 8k             |                |      |      |      |      |
| AV1 8bit   | Output   | NV12           | NV12           | NV12           |                |      |      |      |      |
|            | Max Res. | 16k            | 8k             | 8k             |                |      |      |      |      |
| AV1 10bit  | Output   | P010           | P010           | P010           |                |      |      |      |      |
|            | Max Res. | 16k            | 8k             | 8k             |

- \* VP8 is only supported on TGL platform

### JPEG Decoding Format Support
| Input Format                                         | Output Format | Max resolution | Supported Platforms |
|------------------------------------------------------|---------------|----------------|---------------------|
| 4:2:0, 8-bit                                         | IMC3          | 16K            | BDW+                |
| 4:2:2, 8-bit                                         | 422H          | 16K            | BDW+                |
| 4:2:2, 8-bit                                         | 422V          | 16K            | BDW+                |
| 4:1:1, 8-bit                                         | 411P          | 16K            | BDW+                |
| 4:4:4, 8-bit                                         | 444P          | 16K            | BDW+                |
| 4:4:4, 8-bit                                         | RGBP / BGRP   | 16K            | BDW+                |
| 4:0:0, 8-bit                                         | 400P          | 16K            | BDW+                |
| 420 / 422H<br/>interleaved single scan               | YUY2          | 16K            | BDW+                |
| 420 / 422H<br/>interleaved single scan               | UYVY          | 16K            | BDW+                |
| 420 / 422H / 422<br/>interleaved single scanV        | NV12          | 16K            | BDW+                |
| 400/420/422H/444/RGB/BGR<br/>interleaved single scan | A8R8G8B8      | 16K            | SKL+                |

## Supported Encoding Input Format and Max Resolution

### Hardware Encoding, Low Power Encoding(VDEnc/Huc)

| Codec      | Type         |  DG2/ATSM          | DG1/SG1            | TGLx               | ICL       | KBLx   | BXTx   | SKL    | BDW |
|------------|--------------|--------------------|--------------------|--------------------|-----------|--------|--------|--------|-----|
| AVC        | Input        | *More              | *More              | *More              | *More     | *More  | NV12   | NV12   |     |
|            | Max Res.     | 4k                 | 4k                 | 4k                 | 4k        | 4k     | 4k     | 4k     |     |
| JPEG       | Input/Output | **Note             | **Note             | **Note             | **Note    | **Note | **Note | **Note |     |
|            | Max Res.     | 16k                | 16k                | 16k                | 16k       | 16k    | 16k    | 16k    |     |
| HEVC 8bit  | Input        | NV12/YUY2/AYUV/RGB | NV12/YUY2/AYUV/RGB | NV12/YUY2/AYUV/RGB | NV12/AYUV |        |        |        |     |
|            | Max Res.     | 16k***             | 8k                 | 8k                 | 8k        |        |        |        |     |
| HEVC 10bit | Input        | P010/Y210/Y410/RGB | P010/Y210/Y410/RGB | P010/Y210/Y410/RGB | P010/Y410 |        |        |        |     |
|            | Max Res.     | 16k***             | 8k                 | 8k                 | 8k        |        |        |        |     |
| VP9 8bit   | Input        | NV12/AYUV          | NV12/AYUV          | NV12/AYUV          | NV12/AYUV |        |        |        |     |
|            | Max Res.     | 8k                 | 8k                 | 8k                 | 8k        |        |        |        |     |
| VP9 10bit  | Input        | P010/Y410          | P010/Y410          | P010/Y410          | P010/Y410 |        |        |        |     |
|            | Max Res.     | 8k                 | 8k                 | 8k                 | 8k        |        |        |        |     |
| AV1 8bit   | Input        | NV12               |                    |                    |           |        |        |        |     |
|            | Max Res.     | 8k                 |                    |                    |           |        |        |        |     |
| AV1 10bit  | Input        | P010               |                    |                    |           |        |        |        |     |
|            | Max Res.     | 8k                 |                    |                    |           |        |        |        |     |

- \*More: KBL/CFL/ICL/TGL AVC encoding supported input formats: NV12/YUY2/YUYV/YVYU/UYVY/AYUV/ARGB
- \**Note: JPEG encoding supports input format NV12/YUY2/UYVY/ABGR/Y8 and output format YUV400/YUV420/YUV422H_2Y/YUV444/RGB24.
- \***: 16k=16384x12288


### Hardware(PAK) + Shader(media kernel+VME) Encoding

| Codec      | Type     | DG1/SG1        | TGLx           | ICL       | KBLx | BXTx | SKL  | BDW  |
|------------|----------|----------------|----------------|-----------|------|------|------|------|
| AVC        | Input    | NV12           | NV12           | NV12      | NV12 | NV12 | NV12 | NV12 |
|            | Max Res. | 4k             | 4k             | 4k        | 4k   | 4k   | 4k   | 4k   |
| MPEG2      | Input    | NV12           | NV12           | NV12      | NV12 |      | NV12 | NV12 |
|            | Max Res. | 2k             | 2k             | 2k        | 2k   |      | 2k   | 2k   |
| VP8        | Input    | NV12           | NV12           | NV12      | NV12 |      |      |      |
|            | Max Res. | 4k             | 4k             | 4k        | 4k   |      |      |      |
| HEVC 8bit  | Input    | NV12/YUY2/AYUV | NV12/YUY2/AYUV | NV12/AYUV | NV12 | NV12 | NV12 |      |
|            | Max Res. | 8k             | 8k             | 8k        | 4k   | 4k   | 4k   |      |
| HEVC 10bit | Input    | P010/Y210/Y410 | P010/Y210/Y410 | P010/Y410 |      |      |      |      |
|            | Max Res. | 8k             | 8k             | 8k        |

## Supported Video Processing CSC/Scaling Format

| Platform      | Format | NV12 | YV12 | I420 | P010 | YUY2 | UYVY | Y210 | AYUV | Y410 | P016* | Y216* | Y416* | ARGB/ABGR | A2R10G10B10/A2B10G10R10 |
|---------------|--------|------|------|------|------|------|------|------|------|------|-------|-------|-------|-----------|-------------------------|
| BDW           | Input  | Y    | Y    | Y    |      | Y    |      |      |      |      |       |       |       | Y         | N                       |
|               | Output | Y    | Y    | Y    |      | Y    |      |      |      |      |       |       |       | Y         | N                       |
| SKL/BXTx/KBLx | Input  | Y    | Y    | Y    | Y    | Y    |      |      |      |      |       |       |       | Y         | N                       |
|               | Output | Y    | Y    | Y    |      | Y    |      |      |      |      |       |       |       | Y         | N                       |
| ICL           | Input  | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    |       |       |       | Y         | N                       |
|               | Output | Y    | Y    | Y    | Y    | Y    |      | Y    | Y    | Y    |       |       |       | Y         | Y                       |
| JSL/EHL       | Input  | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    |       |       |       | Y         | N                       |
|               | Output | Y    | Y    | Y    | Y    | Y    |      | Y    | Y    | Y    |       |       |       | Y         | Y                       |
| TGLx          | Input  | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y     | Y     | Y     | Y         | N                       |
|               | Output | Y    | Y    | Y    | Y    | Y    |      | Y    | Y    | Y    | Y     | Y     | Y     | Y         | Y                       |
| DG1/SG1       | Input  | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y     | Y     | Y     | Y         | N                       |
|               | Output | Y    | Y    | Y    | Y    | Y    |      | Y    | Y    | Y    | Y     | Y     | Y     | Y         | Y                       |
| DG2/ATSM      | Input  | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y    | Y     | Y     | Y     | Y         | N                       |
|               | Output | Y    | Y    | Y    | Y    | Y    |      | Y    | Y    | Y    | Y     | Y     | Y     | Y         | Y                       |

* \* For SFC path, there are NOT real 16bit, SFC precision is 12bit; For kernel path, we support real 16bit.
* TGL/DG1/SG1 capable of max resolution: 16K; ICL/JSL/EHL/SKL/BXT/KBL/BDW capable of max resolution: 4k.
* Composition path does not support conversion from BT2020 RGB to BT2020 YUV, BT2020->BT601/BT709, BT601/BT709 -> BT2020. So if some formats, RGB444 planar for example, are only supported by compositon path, make sure the color space isn't BT2020.

##### Resolution definition: 2k=2048x2048, 4k=4096x4096, 8k=8192x8192, 16k=16384x16384