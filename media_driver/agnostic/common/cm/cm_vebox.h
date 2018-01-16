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
//!
//! \file      cm_vebox.h 
//! \brief     Contains CmVebox declarations. 
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMVEBOX_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMVEBOX_H_

#include "cm_def.h"

#define VEBOX_MAX_SURFACE_COUNT  (16)

namespace CMRT_UMD
{
class CmSurface2D;
class CmBufferUP;

//! \brief      CmVebox is an abstraction of the vebox hardware, 
//! \details    Details of vebox hardware be found at 
//!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
//!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf

class CmVebox
{
public:
    //! \brief      Set state for the vebox object
    //! \param      [in] VeBoxState
    //!             CM_VEBOX_STATE
    //! \returns    CM_SUCCESS
    CM_RT_API virtual int32_t SetState(CM_VEBOX_STATE &VeBoxState) = 0;

    //! \brief      Set the current frame input surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t SetCurFrameInputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits for the current frame input surface
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetCurFrameInputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the previous frame input surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t SetPrevFrameInputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits for the previous frame input surface
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetPrevFrameInputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the STMM input surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t SetSTMMInputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits the for STMM input surface
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetSTMMInputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the STMM output surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t SetSTMMOutputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits the for STMM output surface
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetSTMMOutputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the denoised current frame output surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetDenoisedCurFrameOutputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits for the denoised current frame output surface 
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetDenoisedCurOutputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the current frame output surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t SetCurFrameOutputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits for the current frame output surface 
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetCurFrameOutputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the previous frame output surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t SetPrevFrameOutputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits for the previous frame output surface 
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetPrevFrameOutputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set the statistics output surface for the vebox object
    //! \param      [in] pSurf
    //!             CmSurface2D
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetStatisticsOutputSurface(CmSurface2D *pSurf) = 0;

    //! \brief      Set the control bits for the statistics output surface 
    //! \details    Details of the control bits can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS
    CM_RT_API virtual int32_t
    SetStatisticsOutputSurfaceControlBits(const uint16_t ctrlBits) = 0;

    //! \brief      Set vebox parameters through a CmBufferUP.
    //! \details    The size of this  CmBufferUp is 4K. It contains VEBOX_DNDI_STATE, 
    //!             VEBOX_IECP_STATE, VEBOX_GAMUT_STATE, VEBOX_VERTEX_TABLE, and
    //!             VEBOX_CAPTURE_PIPE_STATE. Each state is 1K. The details of each state
    //!             can be found at 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol09-media_vebox.pdf, or 
    //!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol09-media_vebox.pdf
    //! \param      [in] pParamBuffer
    //!             CmBufferUP which contains all the vebox parameter.
    //! \returns    CM_SUCCESS.
    CM_RT_API virtual int32_t SetParam(CmBufferUP *pParamBuffer) = 0;
};
};//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMVEBOX_H_
