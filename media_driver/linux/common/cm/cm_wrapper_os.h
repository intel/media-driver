/*
* Copyright (c) 2007-2017, Intel Corporation
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
//!
//! \file      cm_wrapper_os.h
//! \brief     Contains various Linux-dependent data structures and functions
//!            for executing commands from cmrtlib.
//!

#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMWRAPPEROS_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMWRAPPEROS_H_

#include "cm_def.h"
#include "media_libva_common.h"  // for VADriverContextP

#define CM_OSAL_SURFACE_FORMAT uint32_t
#define CM_SURFACE_FORMAT_R8U  62
#define CM_SURFACE_FORMAT_R16U 57

//////////////////////////////////////////////////////////////////////////////////////
// Thin CMRT definition -- START
//////////////////////////////////////////////////////////////////////////////////////

typedef struct _CM_CREATECMDEVICE_PARAM
{
    uint32_t                    devCreateOption;            // [in]  Dev create option
    pCallBackReleaseVaSurface   callbackReleaseVaSurf;        // [in]  Function Pointer to free Libva surface
    void                        *deviceHandle;           // [out] pointer to handle in driver
    uint32_t                    version;                   // [out] the Cm version
    uint32_t                    driverStoreEnabled;        // [out] DriverStoreEnable flag
    int32_t                     returnValue;               // [out] the return value from CMRT@UMD
}CM_CREATECMDEVICE_PARAM, *PCM_CREATECMDEVICE_PARAM;

typedef struct _CM_CREATESURFACE2D_PARAM
{
    uint32_t    width;                     // [in] width of 2D texture in pixel
    uint32_t    height;                    // [in] height of 2D texture in pixel
    CM_OSAL_SURFACE_FORMAT format;          // [in] 2D texture format in OS layer.

    union
    {
        uint32_t    indexInLookupTable;   // [in] surface 2d's index in look up table.
        uint32_t    vaSurfaceID;          // [in] libva-surface 2d's index in media driver
    };

    void        *vaSurface;                   // [in] Pointer to Libva Surface
    void        *cmSurface2DHandle;         // [out] pointer of CmSurface2D used in driver
    bool        isCmCreated;               // [in] Is the 2D surface created by CM?
    int32_t     returnValue;               // [out] the return value from driver

    bool        isLibvaCreated;            // [in] if the surface created via libva
    void        *vaDisplay;                     // [in] VaDisplay used to free va sruface
}CM_CREATESURFACE2D_PARAM, *PCM_CREATESURFACE2D_PARAM;

#if defined(__cplusplus)
extern "C" {
#endif
int32_t CmThinExecute(VADriverContextP vaDriverCtx,
                      void *deviceHandle,
                      uint32_t inputFunctionId,
                      void *inputData,
                      uint32_t inputDataLen);
#if defined(__cplusplus)
};
#endif

int32_t CmFillMosResource(VASurfaceID vaSurfaceID,
                          VADriverContext *vaDriverCtx,
                          PMOS_RESOURCE osResource);

MOS_FORMAT              CmOSFmtToMosFmt(CM_OSAL_SURFACE_FORMAT format);
CM_OSAL_SURFACE_FORMAT  CmMosFmtToOSFmt(MOS_FORMAT format);

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMWRAPPEROS_H_
