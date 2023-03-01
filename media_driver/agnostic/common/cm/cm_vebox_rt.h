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
//! \file      cm_vebox_rt.h
//! \brief     Contains CmVeboxRT declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMVEBOXRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMVEBOXRT_H_

#include "cm_vebox.h"
#include "cm_hal_vebox.h"

namespace CMRT_UMD
{
class CmDeviceRT;
class CmSurface2DRT;
class CmVeboxData;

//! \brief      CmVebox is the inheritance class of CmVebox_base. All the vebox
//!             operation go through this class API.

class CmVeboxRT: public CmVebox
{
public:
    static int32_t Create(CmDeviceRT *device,
                          uint32_t index,
                          CmVeboxRT* &cmVebox);

    static int32_t Destroy(CmVeboxRT* &cmVebox);

    //! \brief      Set vebox state
    //! \details    Pass user defined vebox state to vebox class member
    //!             m_veboxState
    //! \param      [in] veboxState
    //!             CM_VEBOX_STATE
    //! \returns    CM_SUCCESS
    CM_RT_API int32_t SetState(CM_VEBOX_STATE &veboxState);

    //! \brief      Set current frame input surface
    //! \details    Bind the CM surface to current frame input surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetCurFrameInputSurface(CmSurface2D *surface);

    //! \brief      Set current frame input surface control bits
    //! \details    Pass user defined current frame input surface control bits to vebox
    //!             member, later,the control bit will control memory object state and
    //!             compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t
    SetCurFrameInputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      Set previous frame input surface
    //! \details    Bind the CM surface to previous frame input surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetPrevFrameInputSurface(CmSurface2D *surface);

    //! \brief      Set previous frame input surface control bits
    //! \details    Pass user defined previous frame input surface control bits to vebox
    //!             member, durine enqueue, the control bit will pass to vebox command to
    //!             control memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t SetPrevFrameInputSurfaceControlBits(
        const uint16_t ctrlBits);

    //! \brief      Set STMM frame input surface
    //! \details    Bind the CM surface to STMM input surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetSTMMInputSurface(CmSurface2D *surface);

    //! \brief      Set STMM frame input surface control bits
    //! \details    Pass User defined STMM Input frame surface control bits to vebox member,
    //!             during enqueue, the control bit will pass to vebox command to control
    //!             memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t SetSTMMInputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      Set STMM frame output surface
    //! \details    Bind the CM surface to STMM output surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetSTMMOutputSurface(CmSurface2D *surface);

    //! \brief      Set STMM output frame surface control bits
    //! \details    Pass User defined STMM Output frame surface control bits to vebox member,
    //!             during enqueue, the control bit will pass to vebox command to control
    //!             memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t SetSTMMOutputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      Set Denoised current frame output surface
    //! \details    Bind the CM surface to denoised current frame output surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetDenoisedCurFrameOutputSurface(CmSurface2D *surface);

    //! \brief      Set denoised current output frame surface control bits
    //! \details    Pass user defined  denoised current output frame surface control bits
    //!             to vebox member, during euqueue, the control bit will pass to vebox
    //!             command to control memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t
    SetDenoisedCurOutputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      set current frame output surface
    //! \details    Bind the CM surface to current frame output surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetCurFrameOutputSurface(CmSurface2D *surface);

    //! \brief      Set current frame output surface control bits
    //! \details    Pass user defined current frame output surface control bits to vebox
    //!             member, during enqueue, the control bit will pass to vebox command to
    //!             control memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t
    SetCurFrameOutputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      Set previous frame output surface
    //! \details    Bind the CM surface to previous frame output surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetPrevFrameOutputSurface(CmSurface2D *surface);

    //! \brief      Set previous frame output surface control bits
    //! \details    Pass user defined previous frame output surface control bits to vebox
    //!             member, during enqueue, the control bit will pass to vebox command to
    //!             control memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t
    SetPrevFrameOutputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      Set statistics output surface
    //! \details    Bind the CM surface to statistics output surface in the vebox class
    //! \param      [in] surface
    //!             CmSurface2D
    //! \retval     CM_SUCCESS if the surface index is less than max index number
    //! \retval     CM_FAILURE if the surface index is larger or equal to max index
    //!             number
    CM_RT_API int32_t SetStatisticsOutputSurface(CmSurface2D *surface);

    //! \brief      Set statistics output surface control bits
    //! \details    Pass user defined statistics output surface control bits to vebox
    //!             member, during euqueue, the control bit will pass to vebox command
    //!             to control memory object state and compression etc
    //! \param      [in] ctrlBits
    //!             control bit value
    //! \retval     CM_SUCCESS if surface index is less than max index number
    //! \retval     CM_FAILURE if surface index is equal or larger than max index number
    CM_RT_API int32_t
    SetStatisticsOutputSurfaceControlBits(const uint16_t ctrlBits);

    //! \brief      Set vebox parameter.
    //! \details    Pass pointer of the parameter buffer to vebox class member
    //!             m_paramBuffer. Later on, during the enqueue stage, the
    //!             parameter in the buffer will be bonded to the VEBOX command for
    //!             processing.
    //! \param      [in] paramBuffer
    //!             CmBufferUP which contains all the vebox parameter.
    //! \returns    CM_SUCCESS.
    CM_RT_API int32_t SetParam(CmBufferUP *paramBuffer);

    int32_t GetSurface(uint32_t surfUsage, CmSurface2DRT *&surface);

    virtual uint32_t GetIndexInVeboxArray();

    CM_VEBOX_STATE GetState();

    CmBufferUP *GetParam();

    uint16_t GetSurfaceControlBits(uint32_t usage);

protected:
    CmVeboxRT(CmDeviceRT *device, uint32_t index);

    ~CmVeboxRT();

    int32_t Initialize();

    int32_t SetSurfaceInternal(VEBOX_SURF_USAGE surfUsage, CmSurface2D *surface);

    int32_t SetSurfaceControlBitsInternal(VEBOX_SURF_USAGE surfUsage,
                                          const uint16_t ctrlBits);

    CmDeviceRT *m_device;
    uint32_t m_maxSurfaceIndex;
    CM_VEBOX_STATE m_veboxState;
    CmBufferUP *m_paramBuffer;
    CmSurface2DRT *m_surface2D[VEBOX_MAX_SURFACE_COUNT];
    uint16_t m_surfaceCtrlBits[VEBOX_MAX_SURFACE_COUNT];
    uint32_t m_indexInVeboxArray;

private:
    CmVeboxRT(const CmVeboxRT& other);
    CmVeboxRT& operator=(const CmVeboxRT& other);
};
};  //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMVEBOXRT_H_
