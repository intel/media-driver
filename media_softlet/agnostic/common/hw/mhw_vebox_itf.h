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
//! \file     mhw_vebox_itf.h
//! \brief    MHW VEBOX interface common base
//! \details
//!

#ifndef __MHW_VEBOX_ITF_H__
#define __MHW_VEBOX_ITF_H__

#include "mhw_itf.h"
#include "mhw_vebox_cmdpar.h"
#include "mhw_mi.h"

#define _VEBOX_CMD_DEF(DEF)                             \
    DEF(VEBOX_SURFACE_STATE);                           \
    DEF(VEBOX_STATE);                                   \
    DEF(VEB_DI_IECP);                                   \
    DEF(VEBOX_TILING_CONVERT);                          \

namespace mhw
{
namespace vebox
{
class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;
        _VEBOX_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetVeboxSurfaceControlBits(PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams, uint32_t* pSurfCtrlBits) = 0;

    virtual MOS_STATUS VeboxAdjustBoundary(PMHW_VEBOX_SURFACE_PARAMS pCurrSurf, uint32_t* pdwSurfaceWidth, uint32_t* pdwSurfaceHeight, bool bDIEnable) = 0;

    virtual MOS_STATUS SetVeboxIndex(uint32_t dwVeboxIndex, uint32_t dwVeboxCount, uint32_t dwUsingSFC) = 0;

    virtual MOS_STATUS DestroyHeap() = 0;

    virtual MOS_STATUS CreateHeap() = 0;

    virtual MOS_STATUS UpdateVeboxSync() = 0;

    virtual MOS_STATUS AddVeboxTilingConvert(PMOS_COMMAND_BUFFER cmdBuffer, PMHW_VEBOX_SURFACE_PARAMS          inSurParams, PMHW_VEBOX_SURFACE_PARAMS outSurParams) = 0;

    virtual MOS_STATUS GetVeboxHeapInfo(const MHW_VEBOX_HEAP** ppVeboxHeap) = 0;

    virtual MOS_STATUS SetVeboxHeapStateIndex(uint32_t index) = 0;

    virtual MOS_STATUS AddVeboxSurfaces(PMOS_COMMAND_BUFFER pCmdBufferInUse, PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams) = 0;

    virtual MOS_STATUS SetVeboxDndiState(PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams) = 0;

    virtual MOS_STATUS SetVeboxIecpState(PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) = 0;

    virtual MOS_STATUS SetDisableHistogram(PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) = 0;

    virtual MOS_STATUS SetAlphaFromStateSelect(PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) = 0;

    virtual MOS_STATUS SetVeboxLaceColorParams(MHW_LACE_COLOR_CORRECTION *pLaceColorParams)  = 0;

    virtual MOS_STATUS SetVeboxIecpAceState(PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) = 0;

    virtual MOS_STATUS SetVeboxGamutState(PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams, PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams) = 0;

    virtual MOS_STATUS SetVeboxChromaParams(MHW_VEBOX_CHROMA_PARAMS* chromaParams) = 0;

    virtual MOS_STATUS AddVeboxHdrState(PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) = 0;

    virtual MOS_STATUS AssignVeboxState() = 0;

    virtual MOS_STATUS SetgnLumaWgts(uint32_t dwLumaStadTh, uint32_t dw4X4TGNEThCnt, bool bTGNEEnable) = 0;

    virtual MOS_STATUS SetgnChromaWgts(uint32_t dwChromaStadTh) = 0;

    virtual MOS_STATUS SetgnHVSParams(bool tGNEEnable, uint32_t lumaStadTh, uint32_t chromaStadTh, uint32_t tGNEThCnt, uint32_t historyInit, bool fallBack) = 0;

    virtual MOS_STATUS SetgnHVSMode(bool hVSAutoBdrate, bool hVSAutoSubjective, uint32_t bSDThreshold) = 0;

    virtual MOS_STATUS FindVeboxGpuNodeToUse(PMHW_VEBOX_GPUNODE_LIMIT pVeboxGpuNodeLimit) = 0;

    virtual MOS_STATUS CreateGpuContext(PMOS_INTERFACE  pOsInterface, MOS_GPU_CONTEXT VeboxGpuContext, MOS_GPU_NODE VeboxGpuNode) = 0;

    virtual uint32_t GetVeboxNumInstances() = 0;

    virtual bool IsVeboxScalabilitywith4K() = 0;

    virtual MOS_STATUS Add1DLutState(void *&surface, PMHW_1DLUT_PARAMS p1DLutParams) = 0;

    virtual MOS_STATUS AddFP16State(PMHW_FP16_PARAMS pFP16Params) = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    virtual bool IsVeboxIdReportEnabled() = 0;

    virtual MOS_STATUS ReportVeboxId() = 0;
#endif
    _VEBOX_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

MEDIA_CLASS_DEFINE_END(mhw__vebox__Itf)
};
}  // namespace vebox
}  // namespace mhw
#endif  // __MHW_VEBOX_ITF_H__
