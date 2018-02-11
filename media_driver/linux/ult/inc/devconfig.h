/*
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef DEVCONFIG_H
#define DEVCONFIG_H

#include <string.h>
#include <stdint.h>

#define LOCAL_I915_PARAM_HAS_HUC 42
#define TEST_COUT   std::cout << "[ INFO     ] "

struct DeviceConfig
{
    uint64_t  aperture_size; //DRM_IOCTL_I915_GEM_GET_APERTURE
    //below is for DRM_IOCTL_I915_GETPARAM
    uint32_t  DeviceId;      //I915_PARAM_CHIPSET_ID
    uint32_t  revision;      //I915_PARAM_REVISION
    union {
        uint32_t flags; //Should put this before the struct, otherwise, it would be error in libdrm.
        struct {
            uint32_t has_exec2:1;           //I915_PARAM_HAS_EXECBUF2
            uint32_t has_bsd:1;             //I915_PARAM_HAS_BSD
            uint32_t has_blt:1;             //I915_PARAM_HAS_BLT
            uint32_t has_relaxed_fencing:1; //I915_PARAM_HAS_RELAXED_FENCING

            uint32_t has_wait_timeout:1;    //I915_PARAM_HAS_WAIT_TIMEOUT
            uint32_t has_llc:1;             //I915_PARAM_HAS_LLC
            uint32_t has_vebox:1;           //I915_PARAM_HAS_VEBOX
            uint32_t has_ext_mmap:1;        //I915_PARAM_MMAP_VERSION

            uint32_t has_exec_softpin:1;    //I915_PARAM_HAS_EXEC_SOFTPIN
            uint32_t has_bsd2:1;             //I915_PARAM_HAS_BSD
            uint32_t has_huc:1;             //I915_PARAM_HAS_HUC

            uint32_t reserved:21;
       };
    };

    //uint32_t mmap_version; //I915_PARAM_MMAP_VERSION
    int32_t num_fences_avail;           //I915_PARAM_NUM_FENCES_AVAIL
    int32_t aliasing_ppgtt;             //I915_PARAM_HAS_ALIASING_PPGTT
    int32_t subslice_total;             //I915_PARAM_SUBSLICE_TOTAL
    int32_t eu_total;                   //I915_PARAM_EU_TOTAL

    uint64_t edram_reg;
/*
    void operator = (DeviceConfig &Dev2){
        memcpy(this, &Dev2, sizeof(DeviceConfig));
    };*/
} const DeviceConfigTable [] = {
#define DEVICECONFIG( aper, devId, rev, flags, fences,ppgtt,subslice, eu, edram_reg ) { aper, devId, rev, flags, fences,ppgtt,subslice, eu, edram_reg},
/*
 *      aperture_size, DeviceId, flags(), num_fences_avail, aliasing_ppgtt, subslice_total, eu_total, edram_reg,
 */
DEVICECONFIG( 4286468096, 0x191e, 0x7, 0x01ff, 32, 3, 3, 24, 0 )    //SKL
DEVICECONFIG( 4267114496, 0x5a84, 0xb, 0x03df, 32, 3, 3, 18, 0x0 )  //BXT
DEVICECONFIG( 4248690688, 0x1606, 0x9, 0x03ff, 32, 3, 2, 12, 0x0 )   //BDW
DEVICECONFIG( 4259069952, 0x5a49, 0x3, 0x3ff, 32, 3, 2, 16, 0x0 )    //CNL

#undef DEVICECONFIG
    { 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
typedef struct DeviceConfig DeviceConfig_t;

typedef enum{
    igfxSKLAKE      = 0,
    igfxBROXTON     = 1,
    igfxBROADWELL   = 2,
    igfxCANNONLAKE  = 3,
    igfx_MAX
}Platform_t;

#endif //DEVCONFIG_H