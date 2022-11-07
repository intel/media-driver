/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     ddi_media_context.h
//! \brief    ddi media context defination head file
//! \details  ddi media context defination head file
//!

#ifndef __DDI_MEDIA_CONTEXT_H_
#define __DDI_MEDIA_CONTEXT_H_

#include "mos_cmdbufmgr.h"
#include "media_libva_caps.h"

//!
//! \struct DDI_MEDIA_CONTEXT
//! \brief  Media heap for shared internal structures
//!
struct DDI_MEDIA_CONTEXT
{
    MOS_BUFMGR         *pDrmBufMgr  = nullptr;
    // handle for /dev/dri/card0
    int32_t             fd          = 0;
    int32_t             iDeviceId   = 0;
    bool                bIsAtomSOC  = 0;

    MEDIA_FEATURE_TABLE SkuTable    = {};
    MEDIA_WA_TABLE      WaTable     = {};

    PDDI_MEDIA_HEAP     pSurfaceHeap  = nullptr;
    uint32_t            uiNumSurfaces = 0;

    PDDI_MEDIA_HEAP     pBufferHeap   = nullptr;
    uint32_t            uiNumBufs     = 0;

    PDDI_MEDIA_HEAP     pImageHeap    = nullptr;
    uint32_t            uiNumImages   = 0;

    PDDI_MEDIA_HEAP     pDecoderCtxHeap = nullptr;
    uint32_t            uiNumDecoders   = 0;

    PDDI_MEDIA_HEAP     pEncoderCtxHeap = nullptr;
    uint32_t            uiNumEncoders   = 0;

    PDDI_MEDIA_HEAP     pVpCtxHeap      = nullptr;
    uint32_t            uiNumVPs        = 0;

    PDDI_MEDIA_HEAP     pProtCtxHeap    = nullptr;
    uint32_t            uiNumProts      = 0;

    PDDI_MEDIA_HEAP     pCmCtxHeap      = nullptr;
    uint32_t            uiNumCMs        = 0;

    PDDI_MEDIA_HEAP     pMfeCtxHeap     = nullptr;
    uint32_t            uiNumMfes       = 0;

    // display info
    uint32_t            uiDisplayWidth  = 0;
    uint32_t            uiDisplayHeight = 0;

    // media context reference number
    uint32_t            uiRef           = 0;

    // modulized Gpu context and cmd buffer
    bool                modularizedGpuCtxEnabled    = false;
    OsContext           *m_osContext                = nullptr;
    GpuContextMgr       *m_gpuContextMgr            = nullptr;
    CmdBufMgr           *m_cmdBufMgr                = nullptr;

    // Apogeio MOS module
    MOS_DEVICE_HANDLE   m_osDeviceContext = MOS_INVALID_HANDLE;

    // mutexs to protect the shared resource among multiple context
    MEDIA_MUTEX_T       SurfaceMutex   = {};
    MEDIA_MUTEX_T       MemDecompMutex = {};
    MEDIA_MUTEX_T       BufferMutex    = {};
    MEDIA_MUTEX_T       ImageMutex     = {};
    MEDIA_MUTEX_T       DecoderMutex   = {};
    MEDIA_MUTEX_T       EncoderMutex   = {};
    MEDIA_MUTEX_T       VpMutex        = {};
    MEDIA_MUTEX_T       ProtMutex      = {};
    MEDIA_MUTEX_T       CmMutex        = {};
    MEDIA_MUTEX_T       MfeMutex       = {};

    // GT system Info
    MEDIA_SYSTEM_INFO  *pGtSystemInfo           = nullptr;

    // Media memory decompression data structure
    void               *pMediaMemDecompState    = nullptr;

    // Media copy data structure
    void               *pMediaCopyState         = nullptr;

    // Perf tag
    PERF_DATA          *perfData                = nullptr;

    // Media reset enable flag
    bool                bMediaResetEnable       = false;

    // Media memory decompression function
    void (* pfnMemoryDecompress)(
        PMOS_CONTEXT  pMosCtx,
        PMOS_RESOURCE pOsResource)  = nullptr;

    //!
    //! \brief  the function ptr for surface copy function
    //!
    void  (* pfnMediaMemoryCopy )(
        PMOS_CONTEXT       pMosCtx,
        PMOS_RESOURCE      pInputResource,
        PMOS_RESOURCE      pOutputResource,
        bool               bOutputCompressed)  = nullptr;

    //!
    //! \brief  the function ptr for Media Memory 2D copy function
    //!
    void (* pfnMediaMemoryCopy2D)(
        PMOS_CONTEXT       pMosCtx,
        PMOS_RESOURCE      pInputResource,
        PMOS_RESOURCE      pOutputResource,
        uint32_t           copyWidth,
        uint32_t           copyHeight,
        uint32_t           copyInputOffset,
        uint32_t           copyOutputOffset,
        uint32_t           bpp,
        bool               bOutputCompressed)  = nullptr;

    //!
    //! \brief  the function ptr for Media Tile Convert function
    //!
    VAStatus (* pfnMediaMemoryTileConvert)(
        PMOS_CONTEXT       pMosCtx,
        PMOS_RESOURCE      pInputResource,
        PMOS_RESOURCE      pOutputResource,
        uint32_t           copyWidth,
        uint32_t           copyHeight,
        uint32_t           copyInputOffset,
        uint32_t           copyOutputOffset,
        bool               isTileToLinear,
        bool               outputCompressed)  = nullptr;

    PLATFORM            platform            = {};

    MediaLibvaCaps     *m_caps              = nullptr;

    GMM_CLIENT_CONTEXT  *pGmmClientContext  = nullptr;

    // Aux Table Manager
    AuxTableMgr         *m_auxTableMgr      = nullptr;

    bool                m_useSwSwizzling    = false;
    bool                m_tileYFlag         = false;

#if !defined(ANDROID) && defined(X11_FOUND)
    // X11 Func table, for vpgPutSurface (Linux)
    PDDI_X11_FUNC_TABLE X11FuncTable        = nullptr;

    /* VA/DRI (X11) specific data */
    struct va_dri_output *dri_output        = nullptr;
    //vpgPutSurfaceLinuxHW acceleration hack
    MEDIA_MUTEX_T   PutSurfaceRenderMutex   = {};
    MEDIA_MUTEX_T   PutSurfaceSwapBufferMutex = {};
#endif
    bool                  m_apoMosEnabled     = false;
#ifdef _MANUAL_SOFTLET_
    DdiMediaFunctions     *m_compList[CompCount]    = {};
    MediaInterfacesHwInfo *m_hwInfo                 = nullptr;
    MediaLibvaCapsNext    *m_capsNext               = nullptr;
    bool                  m_apoDdiEnabled           = false;
#endif
    MediaUserSettingSharedPtr m_userSettingPtr      = nullptr;  // used to save user setting instance
};

#endif // __DDI_MEDIA_CONTEXT_H_
