/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_libva_util_next.h
//! \brief    libva(and its extension) utility head file
//!

#ifndef __MEDIA_LIBVA_UTIL_NEXT_H__
#define __MEDIA_LIBVA_UTIL_NEXT_H__

#include "media_libva_common_next.h"
#include "vp_common.h"

#ifdef ANDROID
#define DDI_FUNC_ENTER            UMD_ATRACE_BEGIN(__FUNCTION__)
#define DDI_FUNCTION_EXIT(status)       UMD_ATRACE_END
#else
#define DDI_FUNC_ENTER                                                      \
    MOS_FUNCTION_TRACE(MOS_COMPONENT_DDI, MOS_SUBCOMP_SELF)
#define DDI_FUNCTION_EXIT(status)                                               \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, status)
#endif

#define DDI_CODEC_FUNC_ENTER                                                \
    MOS_FUNCTION_TRACE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CODEC)

#define DDI_VP_FUNC_ENTER                                                   \
    MOS_FUNCTION_TRACE(MOS_COMPONENT_DDI, MOS_SUBCOMP_VP)

#define DDI_CP_FUNC_ENTER                                                   \
    MOS_FUNCTION_TRACE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CP)

#define DDI_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _expr)

#define DDI_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define DDI_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define DDI_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define DDI_CODEC_ASSERTMESSAGE(_message, ...)                              \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CODEC, _message, ##__VA_ARGS__)

#define DDI_CODEC_NORMALMESSAGE(_message, ...)                              \
    MOS_NORMALMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CODEC, _message, ##__VA_ARGS__)

#define DDI_CODEC_VERBOSEMESSAGE(_message, ...)                             \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CODEC, _message, ##__VA_ARGS__)

#define DDI_VP_ASSERTMESSAGE(_message, ...)                                 \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_VP, _message, ##__VA_ARGS__)

#define DDI_VP_NORMALMESSAGE(_message, ...)                                 \
    MOS_NORMALMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_VP, _message, ##__VA_ARGS__)

#define DDI_VP_VERBOSEMESSAGE(_message, ...)                                \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_VP, _message, ##__VA_ARGS__)

#define DDI_CP_ASSERTMESSAGE(_message, ...)                                 \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CP, _message, ##__VA_ARGS__)

#define DDI_CP_NORMALMESSAGE(_message, ...)                                 \
    MOS_NORMALMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CP, _message, ##__VA_ARGS__)

#define DDI_CP_VERBOSEMESSAGE(_message, ...)                                \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_DDI, MOS_SUBCOMP_CP, _message, ##__VA_ARGS__)

// Check the return value of function.
// If failed,  print the error message and return,
// do nothing otherwise.
#define DDI_CHK_RET(_ret, _str)                                           \
{                                                                             \
    VAStatus tmpRet = _ret;                                                   \
    if (VA_STATUS_SUCCESS != tmpRet) {                                        \
        DDI_ASSERTMESSAGE("%s [%d].", _str, tmpRet);                        \
        return tmpRet;                                                        \
    }                                                                         \
}

#define DDI_CODEC_CHK_RET(_ret, _str)                      \
{                                                                             \
    VAStatus tmpRet = _ret;                                                   \
    if (VA_STATUS_SUCCESS != tmpRet) {                                        \
        DDI_CODEC_ASSERTMESSAGE("%s [%d].", _str, tmpRet);                        \
        return tmpRet;                                                        \
    }                                                                         \
}

#define DDI_VP_CHK_RET(_ret, _str)                         \
{                                                                             \
    VAStatus tmpRet = _ret;                                                   \
    if (VA_STATUS_SUCCESS != tmpRet) {                                        \
        DDI_VP_ASSERTMESSAGE("%s [%d].", _str, tmpRet);                        \
        return tmpRet;                                                        \
    }                                                                         \
}

#define DDI_CP_CHK_RET(_ret, _str)                         \
{                                                                             \
    VAStatus tmpRet = _ret;                                                   \
    if (VA_STATUS_SUCCESS != tmpRet) {                                        \
        DDI_CP_ASSERTMESSAGE("%s [%d].", _str, tmpRet);                        \
        return tmpRet;                                                        \
    }                                                                         \
}

// Check the return status of parse function in renderPicture
// If failed, assign new status and break
// do nothing otherwise.
#define DDI_CHK_STATUS(_ret, _newret)                                       \
{                                                                             \
    if (VA_STATUS_SUCCESS != _ret) {                                          \
        vaStatus = _newret;                                                   \
        break;                                                                \
    }                                                                         \
}

// Check the condition, if true, print the error message
// and return the specified value, do nothing otherwise.
#define DDI_CHK_CONDITION(condition, _str, _ret)                            \
    if (condition) {                                                        \
        DDI_ASSERTMESSAGE(_str);                                            \
        return _ret;                                                        \
    }

#define DDI_CODEC_CHK_CONDITION(condition, _str, _ret)                      \
    if (condition) {                                                        \
        DDI_CODEC_ASSERTMESSAGE(_str);                                      \
        return _ret;                                                        \
    }

#define DDI_VP_CHK_CONDITION(condition, _str, _ret)                         \
    if (condition) {                                                        \
        DDI_VP_ASSERTMESSAGE(_str);                                         \
        return _ret;                                                        \
    }

#define DDI_CP_CHK_CONDITION(condition, _str, _ret)                         \
    if (condition) {                                                        \
        DDI_CP_ASSERTMESSAGE(_str);                                         \
        return _ret;                                                        \
    }

// If pointer is nullptr, print the error message and return the specified value.
#define DDI_CHK_NULL(_ptr, _str, _ret)                                      \
    DDI_CHK_CONDITION((nullptr == (_ptr)), _str, _ret)
#define DDI_CODEC_CHK_NULL(_ptr, _str, _ret)                                \
    DDI_CODEC_CHK_CONDITION((nullptr == (_ptr)), _str, _ret)
#define DDI_VP_CHK_NULL(_ptr, _str, _ret)                                   \
    DDI_VP_CHK_CONDITION((nullptr == (_ptr)), _str, _ret)
#define DDI_CP_CHK_NULL(_ptr, _str, _ret)                                   \
    DDI_CP_CHK_CONDITION((nullptr == (_ptr)), _str, _ret)

#define DDI_CHK_LARGER(p, bottom, str, ret)                                 \
    DDI_CHK_CONDITION((p <= bottom),str,ret)
#define DDI_CODEC_CHK_LARGER(p, bottom, str, ret)                           \
    DDI_CODEC_CHK_CONDITION((p <= bottom),str,ret)
#define DDI_VP_CHK_LARGER(p, bottom, str, ret)                              \
    DDI_VP_CHK_CONDITION((p <= bottom),str,ret)
#define DDI_CP_CHK_LARGER(p, bottom, str, ret)                              \
    DDI_CP_CHK_CONDITION((p <= bottom),str,ret)
    
#define DDI_CHK_LESS(p, upper, str, ret)                                    \
    DDI_CHK_CONDITION((p >= upper),str,ret)
#define DDI_CODEC_CHK_LESS(p, upper, str, ret)                              \
    DDI_CODEC_CHK_CONDITION((p >= upper),str,ret)
#define DDI_VP_CHK_LESS(p, upper, str, ret)                                 \
    DDI_VP_CHK_CONDITION((p >= upper),str,ret)
#define DDI_CP_CHK_LESS(p, upper, str, ret)                                 \
    DDI_CP_CHK_CONDITION((p >= upper),str,ret)
struct MEDIA_SURFACE_ALLOCATE_PARAM
{
    uint32_t          pitch;
    uint32_t          tileFormat;
    int32_t           width;
    int32_t           height;
    DDI_MEDIA_FORMAT  format;
    int32_t           alignedWidth;
    int32_t           alignedHeight;
    uint32_t          cpTag;
    int               memType;
    bool              bMemCompEnable;
    bool              bMemCompRC;
};

#define LENGTH_OF_FPS_FILE_NAME 128

#ifdef ANDROID
#define FPS_FILE_NAME   "/mnt/sdcard/fps.txt"
#else
#define FPS_FILE_NAME   "./fps.txt"
#endif

class MediaLibvaUtilNext
{
private:

    //!
    //! \brief  Generate gmm parameters for external surface for none comporession mode
    //!
    //! \param  [out] gmmCustomParams
    //!         gmm parameters
    //! \param  [in] params
    //!         surface allocate paramaters
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GenerateGmmParamsForNoneCompressionExternalSurface(
        GMM_RESCREATE_CUSTOM_PARAMS  &gmmCustomParams,
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        PDDI_MEDIA_SURFACE           mediaSurface);
    
    //!
    //! \brief  Generate gmm parameters for external surface for comporession mode
    //!
    //! \param  [out] gmmParams
    //!         gmm parameters
    //! \param  [in] params
    //!         surface allocate paramaters
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GenerateGmmParamsForCompressionExternalSurface(
        GMM_RESCREATE_CUSTOM_PARAMS_2 &gmmParams,
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        PDDI_MEDIA_SURFACE           mediaSurface,
        PDDI_MEDIA_CONTEXT           mediaDrvCtx);

    //!
    //! \brief  Init surface allocate parameters
    //!
    //! \param  [out] params
    //!         surface allocate paramaters
    //! \param  [in] width
    //!         surface width
    //! \param  [in] height
    //!         surface height
    //! \param  [in] format
    //!         media format
    //! \param  [in] memType
    //!         memory type
    //! \param  [in] surfaceUsageHint
    //!         surface usage hint
    //!
    //! \return void
    //!
    static void InitSurfaceAllocateParams(
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        int32_t                      width,
        int32_t                      height,
        DDI_MEDIA_FORMAT             format,
        int                          memType,
        uint32_t                     surfaceUsageHint);

    //!
    //! \brief  Set default tileformat from media foramt
    //!
    //! \param  [in] format
    //!         DDI media surface format
    //! \param  [in] surfaceUsageHint
    //!         surface usage hint
    //! \param  [in] skuTable
    //!         Sku table
    //! \param  [out] params
    //!         surface allocate paramaters
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SetDefaultTileFormat(
        DDI_MEDIA_FORMAT             format,
        uint32_t                     surfaceUsageHint,
        MEDIA_FEATURE_TABLE          *skuTable,
        MEDIA_SURFACE_ALLOCATE_PARAM &params);

    //!
    //! \brief  Set surface parameter from modifier
    //!
    //! \param  [out] params
    //!         surface allocate paramaters
    //! \param  [in] modifier
    //!         modifier from surface desc
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SetSurfaceParameterFromModifier(
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        uint64_t                     modifier);

    //!
    //! \brief  Create external surface
    //!
    //! \param  [in] params
    //!         surface allocate paramaters
    //! \param  [out] mediaSurface
    //!         Pointer to ddi media surface
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateExternalSurface(
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        PDDI_MEDIA_SURFACE           mediaSurface,
        PDDI_MEDIA_CONTEXT           mediaDrvCtx);
    
    //!
    //! \brief  Generate gmm parameters for internal surface
    //!
    //! \param  [out] gmmParams
    //!         gmm parameters
    //! \param  [in] params
    //!         surface allocate paramaters
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GenerateGmmParamsForInternalSurface(
        GMM_RESCREATE_PARAMS         &gmmParams,
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        PDDI_MEDIA_CONTEXT           mediaDrvCtx);
    
    //!
    //! \brief  Create internal surface
    //!
    //! \param  [in] params
    //!         surface allocate paramaters
    //! \param  [out] mediaSurface
    //!         Pointer to ddi media surface
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateInternalSurface(
        MEDIA_SURFACE_ALLOCATE_PARAM &params,
        PDDI_MEDIA_SURFACE           mediaSurface,
        PDDI_MEDIA_CONTEXT           mediaDrvCtx);

    //!
    //! \brief  Allocate surface
    //!
    //! \param  [in] format
    //!         Ddi media format
    //! \param  [in] width
    //!         Width of the region
    //! \param  [in] height
    //!         Height of the region
    //! \param  [out] mediaSurface
    //!         Pointer to ddi media surface
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus AllocateSurface(
        DDI_MEDIA_FORMAT            format,
        int32_t                     width,
        int32_t                     height,
        PDDI_MEDIA_SURFACE          mediaSurface,
        PDDI_MEDIA_CONTEXT          mediaDrvCtx);
    
    //!
    //! \brief  Swizzle surface by Hardware, current only support VEBOX
    //!
    //! \param  [in] surface
    //!         Pointer of surface
    //! \param  [in] isDeSwizzle
    //!         Whether it's de-swizzling or not
    //!         Swizzling    - copying from video memory to temporary buffer
    //!         De-swizzling - copying from temporary buffer to video memory
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SwizzleSurfaceByHW(DDI_MEDIA_SURFACE *surface, bool isDeSwizzle = false);

    //!
    //! \brief  Allocate 2D buffer
    //!
    //! \param  [in] height
    //!         Height of the region
    //! \param  [in] width
    //!         Width of the region
    //! \param  [out] mediaBuffer
    //!         Pointer to ddi media buffer
    //! \param  [in] bufmgr
    //!         Mos buffer manager
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus Allocate2DBuffer(
        uint32_t            height,
        uint32_t            width,
        PDDI_MEDIA_BUFFER   mediaBuffer,
        MOS_BUFMGR          *bufmgr);

    //!
    //! \brief  Allocate buffer
    //!
    //! \param  [in] format
    //!         Ddi media format
    //! \param  [in] size
    //!         Size of the region
    //! \param  [out] mediaBuffer
    //!         Pointer to ddi media buffer
    //! \param  [in] bufmgr
    //!         Mos buffer manager
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus AllocateBuffer(
        DDI_MEDIA_FORMAT      format,
        int32_t               size,
        PDDI_MEDIA_BUFFER     mediaBuffer,
        MOS_BUFMGR            *bufmgr,
        bool                  isShadowBuffer = false);

public:
    //!
    //! \brief  Allocate pmedia surface from heap
    //!
    //! \param  [in] surfaceHeap
    //!         Pointer to ddi media heap
    //!
    //! \return PDDI_MEDIA_SURFACE_HEAP_ELEMENT
    //!     Pointer to ddi media surface heap element
    //!
    static PDDI_MEDIA_SURFACE_HEAP_ELEMENT AllocPMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap);

    //!
    //! \brief  Release pmedia surface from heap
    //!
    //! \param  [in] surfaceHeap
    //!         Pointer to ddi media heap
    //! \param  [in] vaSurfaceID
    //!         VA surface ID
    //!
    static void ReleasePMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap, uint32_t vaSurfaceID);

    //!
    //! \brief  Create surface
    //! 
    //! \param  [in] surface
    //!         Ddi media surface
    //! \param  [in] mediaDrvCtx
    //!         Pointer to ddi media context
    //!         
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateSurface(DDI_MEDIA_SURFACE  *surface, PDDI_MEDIA_CONTEXT mediaDrvCtx);

    //!
    //!
    //!  Descripion: if the bo of media surface was allocated from App,
    //!              should return true, otherwise, false. In current implemeation
    //!              external buffer passed with pSurfDesc.
    //!
    static bool IsExternalSurface(PDDI_MEDIA_SURFACE surface);

    //!
    //! \brief convert Media Format to Gmm Format for GmmResCreate parameter.
    //!
    //! \param    [in] format
    //!         Pointer to DDI_MEDIA_FORMAT
    //!
    //! \return GMM_RESOURCE_FORMAT
    //!         Pointer to gmm format type
    //!
    static GMM_RESOURCE_FORMAT ConvertMediaFmtToGmmFmt(DDI_MEDIA_FORMAT format);

    //!
    //! \brief  Wait semaphore
    //!
    //! \param  [in] sem
    //!         Pointer to media semaphore thread
    //!
    static void WaitSemaphore(PMEDIA_SEM_T sem);

    //!
    //! \brief  Try wait semaphore
    //! 
    //! \param  [in] sem
    //!         Pointer to media semaphore thread
    //!
    //! \return int32_t 
    //!     Try wait for semaphore. Return 0 if success, else -1 if fail        
    //!
    static int32_t TryWaitSemaphore(PMEDIA_SEM_T sem);

    //!
    //! \brief  Post semaphore
    //!
    //! \param  [in] sem
    //!         Pointer to media semaphore thread
    //!
    static void PostSemaphore(PMEDIA_SEM_T sem);

    //!
    //! \brief  Destroy semaphore
    //!
    //! \param  [in] sem
    //!         Pointer to media semaphore thread
    //!
    static void DestroySemaphore(PMEDIA_SEM_T sem);

    //!
    //! \brief  Unregister RT surfaces
    //!
    //! \param  [in] ctx
    //!     Pointer to VA driver context
    //! \param  [in] surface
    //!     Pointer to ddi media surface
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus UnRegisterRTSurfaces(VADriverContextP ctx, PDDI_MEDIA_SURFACE surface);

    //!
    //! \brief  Free surface
    //!
    //! \param  [in] surface
    //!         Ddi media surface
    //!
    static void FreeSurface(DDI_MEDIA_SURFACE *surface);

    //!
    //! \brief  Free buffer
    //!
    //! \param  [in] buf
    //!         Ddi media buffer
    //!
    static void FreeBuffer(DDI_MEDIA_BUFFER *buf);

    //!
    //! \brief  Lock surface
    //!
    //! \param  [in] surface
    //!         Ddi media surface
    //! \param  [in] flag
    //!         Flag
    //!
    //! \return void*
    //!     Pointer to lock surface data
    //!
    static void* LockSurface(DDI_MEDIA_SURFACE  *surface, uint32_t flag);

    //!
    //! \brief  Lock surface
    //!
    //! \param  [in] surface
    //!         Ddi media surface
    //! \param  [in] flag
    //!         Flag
    //!
    //! \return void*
    //!     Pointer to lock surface data
    //!
    static void* LockSurfaceInternal(DDI_MEDIA_SURFACE *surface, uint32_t flag);

    //!
    //! \brief  Create Shadow Resource of Ddi media surface
    //!
    //! \param  [in] surface
    //!         Ddi media surface
    //!
    //! \return VAStatus
    //!
    static VAStatus CreateShadowResource(DDI_MEDIA_SURFACE *surface);

    //!
    //! \brief  Unlock surface
    //!
    //! \param  [in] surface
    //!         Ddi media surface
    //!
    static void UnlockSurface(DDI_MEDIA_SURFACE *surface);

    //!
    //! \brief  Lock buffer
    //!
    //! \param  [in] buf
    //!         Ddi media buffer
    //! \param  [in] flag
    //!         Flag
    //!
    //! \return void*
    //!     Pointer to lock buffer data
    //!
    static void* LockBuffer(DDI_MEDIA_BUFFER *buf, uint32_t flag);

    //!
    //! \brief  Unlock buffer
    //!
    //! \param  [in] buf
    //!         Ddi media buffer
    //!
    static void UnlockBuffer(DDI_MEDIA_BUFFER *buf);

    //!
    //! \brief  Swizzle Surface
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to VA driver context
    //! \param  [in] pGmmResInfo
    //!         Gmm resource info
    //! \param  [in] pLockedAddr
    //!         Pointer to locked address
    //! \param  [in] TileType
    //!         Tile type
    //! \param  [in] pResourceBase
    //!         Pointer to resource base
    //! \param  [in] bUpload
    //!         Blt upload
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SwizzleSurface(
        PDDI_MEDIA_CONTEXT         mediaCtx, 
        PGMM_RESOURCE_INFO         pGmmResInfo,
        void                       *pLockedAddr, 
        uint32_t                   TileType, 
        uint8_t                    *pResourceBase, 
        bool                       bUpload);

    //!
    //! \brief  Create buffer
    //! 
    //! \param  [out] buffer
    //!         Ddi media buffer
    //! \param  [in] bufmgr
    //!         Mos buffer manager
    //!         
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateBuffer(
        DDI_MEDIA_BUFFER *buffer,
        MOS_BUFMGR       *bufmgr);

    //!
    //! \brief  Allocate pmedia buffer from heap
    //! 
    //! \param  [in] bufferHeap
    //!         Pointer to ddi media heap
    //!         
    //! \return PDDI_MEDIA_BUFFER_HEAP_ELEMENT
    //!     Pointer to ddi media buffer heap element
    //!
    static PDDI_MEDIA_BUFFER_HEAP_ELEMENT AllocPMediaBufferFromHeap(PDDI_MEDIA_HEAP bufferHeap);

    //!
    //! \brief  Allocate PVA image from heap
    //! 
    //! \param  [in] imageHeap
    //!         Pointer to ddi media heap
    //!         
    //! \return PDDI_MEDIA_IMAGE_HEAP_ELEMENT
    //!     Pointer to ddi media image heap element
    //!
    static PDDI_MEDIA_IMAGE_HEAP_ELEMENT AllocPVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap);

    //!
    //! \brief convert FOURCC to Gmm Format.
    //!
    //! \param    [in] fourcc
    //!
    //! \return GMM_RESOURCE_FORMAT
    //!         Pointer to gmm format type
    //!
    static GMM_RESOURCE_FORMAT ConvertFourccToGmmFmt(uint32_t fourcc);

    //! \brief Get surface drm modifier
    //!
    //! \param  [in] mediaCtx
    //!         Media context
    //! \param  [in] mediaSurface
    //!         Pointer to the media surface
    //! \param  [out] modifier
    //!         Reference of the modifier
    //!
    //! \return   VAStatus
    //!     VA_STATUS_SUCCESS if success
    //!
    static VAStatus GetSurfaceModifier(
        DDI_MEDIA_CONTEXT  *mediaCtx,
        DDI_MEDIA_SURFACE  *mediaSurface,
        uint64_t           &modifier);
    
    //!
    //! \brief  Release pmedia buffer from heap
    //! 
    //! \param  [in] bufferHeap
    //!         Pointer to ddi media heap
    //! \param  [in] vaBufferID
    //!         VA buffer ID
    //!
    static void ReleasePMediaBufferFromHeap(
        PDDI_MEDIA_HEAP  bufferHeap,
        uint32_t         vaBufferID);

    //!
    //! \brief  Init a mutex
    //!
    //! \param  [in] mutex
    //!         input mutex
    //! \return     void
    //!
    static void InitMutex(PMEDIA_MUTEX_T mutex);

    //!
    //! \brief  Dstroy a mutex
    //!
    //! \param  [in] mutex
    //!         input mutex
    //! \return     void
    //!
    static void DestroyMutex(PMEDIA_MUTEX_T mutex);

    //!
    //! \brief  Set media reset enable flag
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to VA driver context
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SetMediaResetEnableFlag(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Allocate PVA context from heap
    //!
    //! \param  [in] vaContextHeap
    //!         Pointer to ddi media heap
    //!
    //! \return PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT
    //!     Pointer to ddi media vacontext heap element
    //!
    static PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT DdiAllocPVAContextFromHeap(PDDI_MEDIA_HEAP vaContextHeap);

    //!
    //! \brief  Release PVA context from heap
    //!
    //! \param  [in] vaContextHeap
    //!         Pointer to ddi media heap
    //! \param  [in] vaContextID
    //!         VA context ID
    //!
    static void DdiReleasePVAContextFromHeap(PDDI_MEDIA_HEAP vaContextHeap, uint32_t vaContextID);

    //!
    //! \brief map from media format to vphal format
    //!
    //! \params [in] mediaFormat
    //!         DDI_MEDIA_FORMAT
    //!
    //! \returns MOS_FORMAT
    //!
    static MOS_FORMAT GetFormatFromMediaFormat(DDI_MEDIA_FORMAT mediaFormat);

    //! \brief Get the VP Tile Type from Media Tile Type.
    //!
    //! \params [in] mediaTileType
    //!         input mediaTileType
    //!
    //! \returns Vp tile Type if call succeeds
    //!
    static MOS_TILE_TYPE GetTileTypeFromMediaTileType(uint32_t mediaTileType);

    //!
    //! \brief Get ColorSpace from the media format
    //!
    //! \params [in]  format
    //!        media format
    //!
    //! \returns appropriate VPHAL_CSPACE if call succeeds, CSpace_None otherwise
    //!
    static VPHAL_CSPACE GetColorSpaceFromMediaFormat(DDI_MEDIA_FORMAT format);

    //!
    //! \brief  Unreference buf object media buffer
    //!
    //! \param  [in] buf
    //!         Pointer to ddi media buffer
    //!
    static void UnRefBufObjInMediaBuffer(PDDI_MEDIA_BUFFER buf);

    //!
    //! \brief  ReleaseP VAImage From Heap
    //!
    //! \param  [in] imageHeap
    //!         Image heap
    //! \param  [in] vaImageID
    //!         VA image ID
    //!
    static void ReleasePVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap, uint32_t vaImageID);

    //!
    //! \brief  Media print frame per second
    //!
    static void MediaPrintFps();
private:
    static int32_t         m_frameCountFps;
    static struct timeval  m_tv1;
    static pthread_mutex_t m_fpsMutex;
    static int32_t         m_vaFpsSampleSize;
    static bool m_isMediaFpsPrintFpsEnabled;
MEDIA_CLASS_DEFINE_END(MediaLibvaUtilNext)    
};

//!
//! \brief  Helper inline class intended to simplify mutex lock/unlock
//!         operations primarily used as a stack-allocated object.
//!         In that case, the compiler guarantees to call the destructor
//!         leaving the scope. The class becomes handy in functions
//!         where there are several return statements with different
//!         exit code value.
//!
class MediaLibvaUtilNext_LockGuard {
private:
    PMEDIA_MUTEX_T m_pMutex;
public:
    MediaLibvaUtilNext_LockGuard(PMEDIA_MUTEX_T pMutex):m_pMutex(pMutex)
    {
        MosUtilities::MosLockMutex(m_pMutex);
    }
    ~MediaLibvaUtilNext_LockGuard()
    {
        MosUtilities::MosUnlockMutex(m_pMutex);
    }
MEDIA_CLASS_DEFINE_END(MediaLibvaUtilNext_LockGuard)   
};

#endif  //__MEDIA_LIBVA_UTIL_NEXT_H__
