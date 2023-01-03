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
//! \file     ddi_codec_base_specific.h
//! \brief    Defines base class for softlet DDI codec encode/decoder
//!

#ifndef _DDI_CODEC_BASE_SPECIFIC_H_
#define _DDI_CODEC_BASE_SPECIFIC_H_

#include <stdint.h>
#include <va/va.h>
#include <va/va_backend.h>

#include "ddi_codec_def_specific.h"
#include "media_libva_common_next.h"

namespace codec
{
#define SURFACE_STATE_INACTIVE              0   //!< Surface state inactive flag, should be transfer from inactive->inuse->active->inactive
#define SURFACE_STATE_ACTIVE_IN_LASTFRAME   1   //!< Surface state active in last frame flag, means surface appears in DPB of last frame and certainly some of them will appear in DPB of current frame.
#define SURFACE_STATE_ACTIVE_IN_CURFRAME    64  //!< Surface state active in current frame flag, means surface will be used in current frame.

//!
//! \class  DdiCodecBase
//! \brief  Ddi codec base class
//!
class DdiCodecBase
{
public:
    //!
    //! \brief Constructor
    //!
    DdiCodecBase(){};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiCodecBase(){};

    //!
    //! \brief    Get ready to process for a target surface
    //! \details  It begins the process (encode/decode/vp) for a specified target surface
    //!
    //! \param    [in] ctx
    //!           Pointer to VA driver context
    //! \param    [in] context
    //!           Already created context for the process
    //! \param    [in] renderTarget
    //!           Specified target surface
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus BeginPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VASurfaceID      renderTarget) = 0;

    //!
    //! \brief    Send required buffers to for process
    //! \details  It sends needed buffers by the process (encode/decode/vp) to the driver
    //!
    //! \param    [in] ctx
    //!           Pointer to VA driver context
    //! \param    [in] context
    //!           Already created context for the process
    //! \param    [in] buffers
    //!           Pointer to the buffer array
    //! \param    [in] numBuffers
    //!           Number of buffers in above array
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) = 0;

    //!
    //! \brief    Make the end of rendering for a picture
    //! \details  The driver will start processing the corresponding decoding/encoding/vp for
    //!           given context. This call is non-blocking. The app can start another
    //!           Begin/Render/End sequence on a different render target
    //!
    //! \param    [in] ctx
    //!           Pointer to VA driver context
    //! \param    [in] context
    //!           Already created context for the process
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus EndPicture(
        VADriverContextP ctx,
        VAContextID      context) = 0;

    //!
    //! \brief    Register Render Target Surface
    //! \details  Register surface in render target table
    //!
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] surface
    //!           Pointer to DDI_MEDIA_SURFACE
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RegisterRTSurfaces(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface);

    //!
    //! \brief    Unregister Render Target Surface
    //! \details  Unregister surface in render target table
    //!
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] surface
    //!           Pointer to DDI_MEDIA_SURFACE
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus UnRegisterRTSurfaces(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface);

protected:
    //!
    //! \brief    Get Render Target Index
    //! \details  Get surface index in render target table
    //!
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] surface
    //!           Pointer to DDI_MEDIA_SURFACE
    //!
    //! \return   int32_t
    //!           Render target index
    //!
    int32_t GetRenderTargetID(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface);

    //!
    //! \brief    Clear Reference List
    //! \details  Clear surface state in render target table
    //!
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] withDpb
    //!           Dpb flag
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ClearRefList(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, bool withDpb);

    //!
    //! \brief    Update Registered Render Target Surface Flag
    //! \details  Check if surface need to be registered in render target table
    //!
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] surface
    //!           Pointer to DDI_MEDIA_SURFACE
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus UpdateRegisteredRTSurfaceFlag(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface);
MEDIA_CLASS_DEFINE_END(codec__DdiCodecBase)
};

}
#endif /*  _DDI_CODEC_BASE_SPECIFIC_H_ */
