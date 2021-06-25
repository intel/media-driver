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
//! \file     media_libva_interface_next.h
//! \brief    libva interface next head file
//!

#ifndef __MEDIA_LIBVA_INTERFACE_NEXT_H__
#define __MEDIA_LIBVA_INTERFACE_NEXT_H__

#include <va/va.h>
#include <va/va_backend.h>
#include "media_libva_common_next.h"

class MediaLibvaInterfaceNext
{
public:
    //!
    //! \brief  Initialize
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] devicefd
    //!         Devoce fd
    //! \param  [out] major_version
    //!         Major version
    //! \param  [out] minor_version
    //!         Minor version
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus Initialize (
        VADriverContextP ctx,
        int32_t          devicefd,
        int32_t         *major_version,     /* out */
        int32_t         *minor_version      /* out */
    );

    //!
    //! \brief  clean up all library internal resources
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus Terminate(VADriverContextP ctx);

private:
    //!
    //! \brief  Init component list
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus InitCompList(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Release component list
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //!
    static void ReleaseCompList(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Load DDI function pointer
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus LoadFunction(VADriverContextP ctx);

        //!
    //! \brief  Free for media context
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //!
    static void FreeForMediaContext(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Free for media context
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context mutex
    //!
    static void DestroyMediaContextMutex(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Create media context
    //!
    //!
    static PDDI_MEDIA_CONTEXT CreateMediaDriverContext();

    //!
    //! \brief  Initialize
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to DDI media driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus HeapInitialize(PDDI_MEDIA_CONTEXT mediaCtx);

#ifdef _MMC_SUPPORTED
    //!
    //! \brief  Decompress internal media memory 
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] osResource
    //!         Pointer mos resource
    //!
    static void MediaMemoryDecompressInternal(
        PMOS_CONTEXT  mosCtx,
        PMOS_RESOURCE osResource);
    
    //!
    //! \brief  copy internal media surface to another surface 
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] inputOsResource
    //!         Pointer input mos resource
    //! \param  [in] outputOsResource
    //!         Pointer output mos resource
    //! \param  [in] boutputcompressed
    //!         output can be compressed or not
    //!
    static void MediaMemoryCopyInternal(
        PMOS_CONTEXT  mosCtx, 
        PMOS_RESOURCE inputOsResource, 
        PMOS_RESOURCE outputOsResource, 
        bool          boutputcompressed);
    
    //!
    //! \brief  copy internal media surface/buffer to another surface/buffer 
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] inputOsResource
    //!         Pointer input mos resource
    //! \param  [in] outputOsResource
    //!         Pointer output mos resource
    //! \param  [in] boutputcompressed
    //!         output can be compressed or not
    //! \param  [in] copyWidth
    //!         The 2D surface Width
    //! \param  [in] copyHeight
    //!         The 2D surface height
    //! \param  [in] copyInputOffset
    //!         The offset of copied surface from
    //! \param  [in] copyOutputOffset
    //!         The offset of copied to
    //!
    static void MediaMemoryCopy2DInternal(
        PMOS_CONTEXT  mosCtx,
        PMOS_RESOURCE inputOsResource,
        PMOS_RESOURCE outputOsResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        uint32_t      bpp,
        bool          boutputcompressed);

    //!
    //! \brief  Tile/Linear format conversion for media surface/buffer
    //!
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] inputOsResource
    //!         Pointer input mos resource
    //! \param  [in] outputOsResource
    //!         Pointer output mos resource
    //! \param  [in] copyWidth
    //!         The 2D surface Width
    //! \param  [in] copyHeight
    //!         The 2D surface height
    //! \param  [in] copyInputOffset
    //!         The offset of copied surface from
    //! \param  [in] copyOutputOffset
    //!         The offset of copied to
    //! \param  [in] isTileToLinear
    //!         Convertion direction, true: tile->linear, false: linear->tile
    //! \param  [in] outputCompressed
    //!         output can be compressed or not
    //!
    static VAStatus MediaMemoryTileConvertInternal(
        PMOS_CONTEXT  mosCtx,
        PMOS_RESOURCE inputOsResource,
        PMOS_RESOURCE outputOsResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        bool          isTileToLinear,
        bool          outputCompressed);

#endif

#if defined(X11_FOUND)
#define X11_LIB_NAME "libX11.so.6"
    //!
    //! \brief  Close opened libX11.so lib, free related function table.
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //!
    static void DestroyX11Connection(
        PDDI_MEDIA_CONTEXT mediaCtx);
   
    //!
    //! \brief  dlopen libX11.so, setup the function table
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //!
    static VAStatus ConnectX11(
        PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Output driver initialization
    //!
    //! \param  [in] ctx
    //!     Pointer to VA driver context
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    static bool OutputDriInit(VADriverContextP ctx);

    //!
    //! \brief  Get dso symbols
    //!
    //! \param  [in] h
    //!     Pointer dso handle
    //! \param  [in] vtable
    //!     Pointer to VA driver table
    //! \param  [in] vtable_length
    //!     VA driver table length
    //! \param  [in] symbols
    //!     dso symbols
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    static bool DsoGetSymbols(
        struct dso_handle          *h,
        void                       *vtable,
        uint32_t                    vtable_length,
        const struct dso_symbol    *symbols);

    //!
    //! \brief  Open dso
    //!
    //! \param  [in] path
    //!     dso path
    //!
    //! \return dso_handle
    //!     dso handle struct
    //!
    static struct dso_handle* DsoOpen(const char *path);

    //!
    //! \brief  Get symbol
    //!
    //! \param  [in] h
    //!     Pointer dso handle
    //! \param  [in] func_vptr
    //!     Pointer to function
    //! \param  [in] name
    //!     Functions name
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    static bool GetSymbol(
        struct dso_handle *h,
        void              *func_vptr,
        const char        *name);
#endif

    //!
    //! \brief  Free allocated surfaceheap elements
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //!
    static void FreeSurfaceHeapElements(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Free allocated bufferheap elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeBufferHeapElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated Imageheap elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeImageHeapElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated contextheap elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeContextHeapElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated ContextCM elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeContextCMElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated heap elements
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus HeapDestroy(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Execute free allocated bufferheap elements for FreeContextHeapElements function
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //! \param  [in] contextHeap
    //!         context heap
    //! \param  [in] vaContextOffset
    //!         context heap
    //! \param  [in] ctxNums
    //!         context numbers
    //!
    static void FreeContextHeap(
        VADriverContextP ctx,
        PDDI_MEDIA_HEAP  contextHeap,
        int32_t          vaContextOffset,
        int32_t          ctxNums);

    //!
    //! \brief  DestroyCMContext
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //! \param  [in] ctxID
    //!         context ID
    //!
    static VAStatus DestroyContextCM(
        VADriverContextP ctx,
        VAContextID      ctxID);

public:
    // Global mutex
    static MEDIA_MUTEX_T m_GlobalMutex;
};

#endif //__MEDIA_LIBVA_INTERFACE_NEXT_H__