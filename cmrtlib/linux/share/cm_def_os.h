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
#ifndef CMRTLIB_LINUX_SHARE_CM_DEF_OS_H_
#define CMRTLIB_LINUX_SHARE_CM_DEF_OS_H_

#include "cm_include.h"
#include "cm_common.h"

#ifndef ANDROID
#include "va/va.h"
#else
#include <va/va_android.h>
#define Display unsigned int
#endif

#include <cstdlib>
#include <cstring>
#include "pthread.h"


////////////////////////////////////////////////////////////////////////////////////
// MS-specific defines/typedefs, which are absent under Linux but still used
////////////////////////////////////////////////////////////////////////////////////
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free(ptr) free(ptr)
typedef uint8_t BOOLEAN, *PBOOLEAN;
////////////////////////////////////////////////////////////////////////////////////
// MS-specific defines/typedefs, which are absent under Linux but still used (End)
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent macros (Start)
////////////////////////////////////////////////////////////////////////////////////
#define CM_STRCPY(dst, sizeInBytes, src)       strcpy(dst, src)
#define CM_STRNCPY(dst, sizeOfDst, src, count) strncpy(dst, src, count)
#define CM_STRCAT(dst, sizeOfDst, src)       strcat(dst, src)
#define CM_GETENV(dst, name) dst = getenv(name)
#define CM_GETENV_FREE(dst)
#define CM_FOPEN(pFile, filename, mode) pFile = fopen(filename, mode)

#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif

#define SUCCEEDED(hr)   (hr == VA_STATUS_SUCCESS)
#define FAILED(hr)      (hr != VA_STATUS_SUCCESS)

typedef enum _VACMTEXTUREADDRESS {
    VACMTADDRESS_WRAP            = 1,
    VACMTADDRESS_MIRROR          = 2,
    VACMTADDRESS_CLAMP           = 3,
    VACMTADDRESS_BORDER          = 4,
    VACMTADDRESS_MIRRORONCE      = 5,

    VACMTADDRESS_FORCE_DWORD     = 0x7fffffff
} VACMTEXTUREADDRESS;

typedef enum _VACMTEXTUREFILTERTYPE {
    VACMTEXF_NONE            = 0,
    VACMTEXF_POINT           = 1,
    VACMTEXF_LINEAR          = 2,
    VACMTEXF_ANISOTROPIC     = 3,
    VACMTEXF_FLATCUBIC       = 4,
    VACMTEXF_GAUSSIANCUBIC   = 5,
    VACMTEXF_PYRAMIDALQUAD   = 6,
    VACMTEXF_GAUSSIANQUAD    = 7,
    VACMTEXF_CONVOLUTIONMONO = 8,    // Convolution filter for monochrome textures
    VACMTEXF_FORCE_DWORD     = 0x7fffffff
} VACMTEXTUREFILTERTYPE;
////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent macros (End)
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent definitions (Start)
////////////////////////////////////////////////////////////////////////////////////

#define VAExtModuleCMRT 2
#define CM_MAX_SURFACE2D_FORMAT_COUNT 47

inline void * CM_ALIGNED_MALLOC(size_t size, size_t alignment)
{
  return aligned_alloc(alignment, size);
}

inline void CM_ALIGNED_FREE(void * memory)
{
  free(memory);
}

// max resolution for surface 2D
#define CM_MAX_2D_SURF_WIDTH  16384
#define CM_MAX_2D_SURF_HEIGHT 16384

typedef enum _VA_CM_FORMAT {

    VA_CM_FMT_UNKNOWN              =   0,

    VA_CM_FMT_A8R8G8B8             =  21,
    VA_CM_FMT_X8R8G8B8             =  22,
    VA_CM_FMT_A8                   =  28,
    VA_CM_FMT_A2B10G10R10          =  31,
    VA_CM_FMT_A8B8G8R8             =  32,
    VA_CM_FMT_R16G16UN             =  35,
    VA_CM_FMT_A16B16G16R16         =  36,
    VA_CM_FMT_A8P8                 =  40,
    VA_CM_FMT_P8                   =  41,
    VA_CM_FMT_R32U                 =  42,
    VA_CM_FMT_R8G8UN               =  49,
    VA_CM_FMT_L8                   =  50,
    VA_CM_FMT_A8L8                 =  51,
    VA_CM_FMT_R16UN                =  56,
    VA_CM_FMT_R16U                 =  57,
    VA_CM_FMT_V8U8                 =  60,
    VA_CM_FMT_R8UN                 =  61,
    VA_CM_FMT_R8U                  =  62,
    VA_CM_FMT_R32S                 =  71,
    VA_CM_FMT_D16                  =  80,
    VA_CM_FMT_L16                  =  81,
    VA_CM_FMT_R16F                 = 111,
    VA_CM_FMT_IA44                 = 112,
    VA_CM_FMT_A16B16G16R16F        = 113,
    VA_CM_FMT_R32F                 = 114,
    VA_CM_FMT_R32G32B32A32F        = 115,
    VA_CM_FMT_I420                 = VA_FOURCC('I','4','2','0'),
    VA_CM_FMT_P216                 = VA_FOURCC('P','2','1','6'),
    VA_CM_FMT_400P                 = VA_FOURCC('4','0','0','P'),
    VA_CM_FMT_Y8UN                 = VA_FOURCC('Y','8','U','N'),
    VA_CM_FMT_NV12                 = VA_FOURCC_NV12,
    VA_CM_FMT_UYVY                 = VA_FOURCC_UYVY,
    VA_CM_FMT_YUY2                 = VA_FOURCC_YUY2,
    VA_CM_FMT_444P                 = VA_FOURCC_444P,
    VA_CM_FMT_411P                 = VA_FOURCC_411P,
    VA_CM_FMT_422H                 = VA_FOURCC_422H,
    VA_CM_FMT_422V                 = VA_FOURCC_422V,
    VA_CM_FMT_411R                 = VA_FOURCC_411R,
    VA_CM_FMT_RGBP                 = VA_FOURCC_RGBP,
    VA_CM_FMT_BGRP                 = VA_FOURCC_BGRP,
    VA_CM_FMT_IMC3                 = VA_FOURCC_IMC3,
    VA_CM_FMT_YV12                 = VA_FOURCC_YV12,
    VA_CM_FMT_P010                 = VA_FOURCC_P010,
    VA_CM_FMT_P012                 = VA_FOURCC_P012,
    VA_CM_FMT_P016                 = VA_FOURCC_P016,
    VA_CM_FMT_P208                 = VA_FOURCC_P208,
    VA_CM_FMT_AYUV                 = VA_FOURCC_AYUV,
#if VA_CHECK_VERSION(1, 13, 0)
    VA_CM_FMT_XYUV                 = VA_FOURCC_XYUV,
#endif
    VA_CM_FMT_Y210                 = VA_FOURCC_Y210,
#if VA_CHECK_VERSION(1, 9, 0)
    VA_CM_FMT_Y212                 = VA_FOURCC_Y212,
#endif
    VA_CM_FMT_Y410                 = VA_FOURCC_Y410,
#if VA_CHECK_VERSION(1, 9, 0)
    VA_CM_FMT_Y412                 = VA_FOURCC_Y412,
#endif
    VA_CM_FMT_Y216                 = VA_FOURCC_Y216,
    VA_CM_FMT_Y416                 = VA_FOURCC_Y416,
    VA_CM_FMT_AI44                 = VA_FOURCC_AI44,

    VA_CM_FMT_MAX                   = 0xFFFFFFFF

} VA_CM_FORMAT;

#define CM_SURFACE_FORMAT                       VA_CM_FORMAT
#define CM_SURFACE_FORMAT_UNKNOWN               VA_CM_FMT_UNKNOWN
#define CM_SURFACE_FORMAT_A8R8G8B8              VA_CM_FMT_A8R8G8B8
#define CM_SURFACE_FORMAT_X8R8G8B8              VA_CM_FMT_X8R8G8B8
#define CM_SURFACE_FORMAT_A8B8G8R8              VA_CM_FMT_A8B8G8R8
#define CM_SURFACE_FORMAT_A8                    VA_CM_FMT_A8
#define CM_SURFACE_FORMAT_P8                    VA_CM_FMT_P8
#define CM_SURFACE_FORMAT_R32F                  VA_CM_FMT_R32F
#define CM_SURFACE_FORMAT_NV12                  VA_CM_FMT_NV12
#define CM_SURFACE_FORMAT_UYVY                  VA_CM_FMT_UYVY
#define CM_SURFACE_FORMAT_YUY2                  VA_CM_FMT_YUY2
#define CM_SURFACE_FORMAT_V8U8                  VA_CM_FMT_V8U8

#define CM_SURFACE_FORMAT_R8_UINT               VA_CM_FMT_R8U
#define CM_SURFACE_FORMAT_R16_SINT              VA_CM_FMT_A8L8
#define CM_SURFACE_FORMAT_R16_UINT              VA_CM_FMT_R16U
#define CM_SURFACE_FORMAT_D16                   VA_CM_FMT_D16
#define CM_SURFACE_FORMAT_L16                   VA_CM_FMT_L16
#define CM_SURFACE_FORMAT_A16B16G16R16          VA_CM_FMT_A16B16G16R16
#define CM_SURFACE_FORMAT_R10G10B10A2           VA_CM_FMT_A2B10G10R10
#define CM_SURFACE_FORMAT_A16B16G16R16F         VA_CM_FMT_A16B16G16R16F
#define CM_SURFACE_FORMAT_R32G32B32A32F         VA_CM_FMT_R32G32B32A32F

#define CM_SURFACE_FORMAT_444P                  VA_CM_FMT_444P
#define CM_SURFACE_FORMAT_422H                  VA_CM_FMT_422H
#define CM_SURFACE_FORMAT_422V                  VA_CM_FMT_422V
#define CM_SURFACE_FORMAT_411P                  VA_CM_FMT_411P
#define CM_SURFACE_FORMAT_411R                  VA_CM_FMT_411R
#define CM_SURFACE_FORMAT_RGBP                  VA_CM_FMT_RGBP
#define CM_SURFACE_FORMAT_BGRP                  VA_CM_FMT_BGRP
#define CM_SURFACE_FORMAT_IMC3                  VA_CM_FMT_IMC3
#define CM_SURFACE_FORMAT_YV12                  VA_CM_FMT_YV12
#define CM_SURFACE_FORMAT_P010                  VA_CM_FMT_P010
#define CM_SURFACE_FORMAT_P016                  VA_CM_FMT_P016
#define CM_SURFACE_FORMAT_P208                  VA_CM_FMT_P208
#define CM_SURFACE_FORMAT_AYUV                  VA_CM_FMT_AYUV
#define CM_SURFACE_FORMAT_Y210                  VA_CM_FMT_Y210
#define CM_SURFACE_FORMAT_Y410                  VA_CM_FMT_Y410
#define CM_SURFACE_FORMAT_Y216                  VA_CM_FMT_Y216
#define CM_SURFACE_FORMAT_Y416                  VA_CM_FMT_Y416

#define CM_SURFACE_FORMAT_IA44                  VA_CM_FMT_IA44
#define CM_SURFACE_FORMAT_AI44                  VA_CM_FMT_AI44
#define CM_SURFACE_FORMAT_I420                  VA_CM_FMT_I420
#define CM_SURFACE_FORMAT_P216                  VA_CM_FMT_P216
#define CM_SURFACE_FORMAT_400P                  VA_CM_FMT_400P
#define CM_SURFACE_FORMAT_R16_FLOAT             VA_CM_FMT_R16F
#define CM_SURFACE_FORMAT_Y8_UNORM              VA_CM_FMT_Y8UN
#define CM_SURFACE_FORMAT_A8P8                  VA_CM_FMT_A8P8
#define CM_SURFACE_FORMAT_R32_SINT              VA_CM_FMT_R32S
#define CM_SURFACE_FORMAT_R32_UINT              VA_CM_FMT_R32U
#define CM_SURFACE_FORMAT_R8G8_UNORM            VA_CM_FMT_R8G8UN
#define CM_SURFACE_FORMAT_R8_UNORM              VA_CM_FMT_R8UN
#define CM_SURFACE_FORMAT_R16G16_UNORM          VA_CM_FMT_R16G16UN
#define CM_SURFACE_FORMAT_R16_UNORM             VA_CM_FMT_R16UN


#define CM_TEXTURE_ADDRESS_TYPE                 VACMTEXTUREADDRESS
#define CM_TEXTURE_ADDRESS_WRAP                 VACMTADDRESS_WRAP
#define CM_TEXTURE_ADDRESS_MIRROR               VACMTADDRESS_MIRROR
#define CM_TEXTURE_ADDRESS_CLAMP                VACMTADDRESS_CLAMP
#define CM_TEXTURE_ADDRESS_BORDER               VACMTADDRESS_BORDER
#define CM_TEXTURE_ADDRESS_MIRRORONCE           VACMTADDRESS_MIRRORONCE

#define CM_TEXTURE_FILTER_TYPE                  VACMTEXTUREFILTERTYPE
#define CM_TEXTURE_FILTER_TYPE_NONE             VACMTEXF_NONE
#define CM_TEXTURE_FILTER_TYPE_POINT            VACMTEXF_POINT
#define CM_TEXTURE_FILTER_TYPE_LINEAR           VACMTEXF_LINEAR
#define CM_TEXTURE_FILTER_TYPE_ANISOTROPIC      VACMTEXF_ANISOTROPIC
#define CM_TEXTURE_FILTER_TYPE_FLATCUBIC        VACMTEXF_FLATCUBIC
#define CM_TEXTURE_FILTER_TYPE_GAUSSIANCUBIC    VACMTEXF_GAUSSIANCUBIC
#define CM_TEXTURE_FILTER_TYPE_PYRAMIDALQUAD    VACMTEXF_PYRAMIDALQUAD
#define CM_TEXTURE_FILTER_TYPE_GAUSSIANQUAD     VACMTEXF_GAUSSIANQUAD
#define CM_TEXTURE_FILTER_TYPE_CONVOLUTIONMONO  VACMTEXF_CONVOLUTIONMONO
////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent definitions (End)
////////////////////////////////////////////////////////////////////////////////////

typedef enum _AdapterInfoType
{
    Description,                    //    char Description[ 256 ];
    VendorId,                       //    uint32_t VendorId;
    DeviceId,                       //    uint32_t DeviceId;
    SubSysId,                       //    uint32_t SubSysId;
    Revision,                       //    uint32_t Revision;
    DedicatedVideoMemory,           //    uint32_t DedicatedVideoMemory;
    DedicatedSystemMemory,          //    uint32_t DedicatedSystemMemory;
    SharedSystemMemory,             //    uint32_t SharedSystemMemory;
    MaxThread,                      //    uint32_t hardware thread count
    EuNumber,                       //    uint32_t EU count
    TileNumber,                     //    uint32_t Tile count
    Reserved                        //    uint32_t
} AdapterInfoType;

typedef enum _REGISTRATION_OP
{
    REG_IGNORE          = 0,
    REG_REGISTER        = 1,
    REG_UNREGISTER      = 2,
    REG_REGISTER_INDEX  = 3     // Register surface for Cm
} REGISTRATION_OP;

class CSync
{
public:
    CSync() { pthread_mutex_init(&m_criticalSection, nullptr); }
    ~CSync() { pthread_mutex_destroy(&m_criticalSection); }
    void Acquire() {  pthread_mutex_lock(&m_criticalSection); }
    void Release() {pthread_mutex_unlock(&m_criticalSection); }

private:
    pthread_mutex_t m_criticalSection;
};

//The communication function for CM to call into UMD,  get function pointer by libVA::vaGetLibFunc()
typedef VAStatus (__cdecl *pvaCmExtSendReqMsg)(VADisplay dpy, void *moduleType,
                                             uint32_t *inputFunId,  void *inputData,  uint32_t *inputDataLen,
                         uint32_t *outputFunId, void *outputData, uint32_t *outputDataLen);

typedef struct _CM_CREATESURFACE2D_PARAM
{
    uint32_t    width;                     // [in] width of 2D texture in pixel
    uint32_t    height;                    // [in] height of 2D texture in pixel
    CM_SURFACE_FORMAT   format;             // [in] DXGI format of 2D texture
    union
    {
        uint32_t index2DinLookupTable;       // [in] surface 2d's index in look up table.
        uint32_t vaSurfaceID;              // [in] libva-surface 2d's index in media driver
    };
    VASurfaceID *vaSurface;                  // [in] Pointer to a Libva Surface.
    void        *cmSurface2DHandle;         // [out] pointer of CmSurface2D used in driver
    bool        isCmCreated;
    int32_t     returnValue;               // [out] the return value from driver
    bool        isLibvaCreated;            // [in] if the surface created via libva
    void        *vaDpy;                     // [in] VaDisplay used to free va sruface
}CM_CREATESURFACE2D_PARAM, *PCM_CREATESURFACE2D_PARAM;

//The communication function for CM to call into UMD,  get function pointer by libVA::vaGetLibFunc()
typedef VAStatus (__cdecl *pvaCmExtSendReqMsg)(
                            VADisplay dpy,
                            void *moduleType,
                            uint32_t *inputFunId,
                            void *inputData,
                            uint32_t *inputDataLen,
                            uint32_t *outputFunId,
                            void *outputData,
                            uint32_t *outputDataLen);

typedef VADisplay (*pfVAGetDisplayDRM) (int32_t fd);    //vaGetDisplayDRM from libva-drm.so

#ifndef CMRT_NOINLINE
#define CMRT_NOINLINE __attribute__((noinline))
#endif

#ifdef _DEBUG
#define CmAssert(expr)        \
    if( !(expr) )             \
    {                         \
    __builtin_trap();         \
    }
#else
#define CmAssert(expr)
#endif

typedef void *HMODULE;

#endif  // #ifndef CMRTLIB_LINUX_SHARE_CM_DEF_OS_H_
