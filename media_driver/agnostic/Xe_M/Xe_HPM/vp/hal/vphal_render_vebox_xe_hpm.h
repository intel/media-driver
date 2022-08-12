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
//! \file     vphal_render_vebox_xe_hpm.h
//! \brief    Interface and structure specific for Xe_HPM Vebox
//! \details  Interface and structure specific for Xe_HPM Vebox
//!
#ifndef __VPHAL_RENDER_VEBOX_XE_HPM_H__
#define __VPHAL_RENDER_VEBOX_XE_HPM_H__

#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_xe_xpm.h"

typedef class VPHAL_VEBOX_STATE_XE_HPM *PVPHAL_VEBOX_STATE_XE_HPM;
class VPHAL_VEBOX_STATE_XE_HPM:virtual public VPHAL_VEBOX_STATE_XE_XPM
{
public:
    VPHAL_VEBOX_STATE_XE_HPM(
        PMOS_INTERFACE                 pOsInterface,
        PMHW_VEBOX_INTERFACE           pVeboxInterface,
        PMHW_SFC_INTERFACE             pSfcInterface,
        PRENDERHAL_INTERFACE           pRenderHal,
        PVPHAL_VEBOX_EXEC_STATE        pVeboxExecState,
        PVPHAL_RNDR_PERF_DATA          pPerfData,
        const VPHAL_DNDI_CACHE_CNTL    &dndiCacheCntl,
        MOS_STATUS                     *peStatus);

    virtual ~VPHAL_VEBOX_STATE_XE_HPM();

    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief    Check TGNE Valid
    //! \details  Check TGNE is Valid or not
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CheckTGNEValid(
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr,
        uint32_t *pQuery);

    //!
    //! \brief    Consistent Check the value of GNE Luma
    //! \details  Consistent Check the value of GNE Luma
    //! \param    [in, out] dwGNELuma
    //!           Spatial GNE in Y channel
    //! \param    uint32_t* pStatSlice0GNEPtr
    //!           [in] Pointer to Vebox slice0 GNE data
    //! \param    uint32_t* pStatSlice1GNEPtr
    //!           [in] Pointer to Vebox slice1 GNE data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GNELumaConsistentCheck(
        uint32_t &dwGNELuma,
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr);

    //!
    //! \brief    Vebox HW state heap update for MHW states
    //! \details  update Vebox HW states (DN),
    //! \param    PVPHAL_SURFACE pSrcSurface
    //!           [in] Pointer to input surface of Vebox
    //! \param    PVPHAL_VEBOX_STATE_PARAMS pVeboxStateParams
    //!           [in] Pointer to VEBOX State Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxUpdateVeboxHwStates(
        PVPHAL_SURFACE                pSrcSurface,
        PVPHAL_VEBOX_STATE_PARAMS     pVeboxStateParams);

protected:
    virtual MOS_STATUS VeboxUpdateVeboxStates(
        PVPHAL_SURFACE pSrcSurface) override;

    virtual MOS_STATUS VeboxUpdateDnStatesForHVS(
        PVPHAL_DENOISE_PARAMS pDNParams,
        uint32_t *            pStatSlice0GNEPtr,
        uint32_t *            pStatSlice1GNEPtr);
};

#endif // __VPHAL_RENDER_VEBOX_XE_HPM_H__

