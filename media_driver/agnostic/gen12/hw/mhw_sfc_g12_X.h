/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file     mhw_sfc_g12_X.h
//! \brief    Defines functions for constructing sfc commands on Gen12-based platforms
//!

#ifndef __MHW_SFC_G12_X_H__
#define __MHW_SFC_G12_X_H__

#include "mhw_sfc_generic.h"
#include "mhw_sfc_hwcmd_g12_X.h"
#include "mhw_utilities.h"
#include "mos_os.h"

static const int   MHW_SFC_MAX_WIDTH_G12  = 16 * 1024;
static const int   MHW_SFC_MAX_HEIGHT_G12 = 16 * 1024;

struct MHW_SFC_STATE_PARAMS_G12: public MHW_SFC_STATE_PARAMS
{
    // HCP-SFC pipe only for scalability and more input/output color format
    uint32_t                        engineMode;                                 //!< 0 - single, 1 - left most column, 2 - right most column, 3 - middle column
    uint32_t                        inputBitDepth;                              //!< 0 - 8bit, 1 - 10bit, 2 - 12bit
    uint32_t                        tileType;                                   //!< virtual tile = 1, another tile = 0
    uint32_t                        srcStartX;                                  //!< Source surface column horizontal start position in pixel
    uint32_t                        srcEndX;                                    //!< Source surface column horizontal end position in pixel
    uint32_t                        dstStartX;                                  //!< Destination surface column horizontal start position in pixel
    uint32_t                        dstEndX;                                    //!< Destination surface column horizontal end position in pixel

    // Histogram stream out
    PMOS_SURFACE                    histogramSurface;                    //!< Histogram stream out buffer
    // Row Store and Column Store Scratch buffer
    PMOS_RESOURCE                   resSfdLineBuffer;                        // SFD Row Store buffer used by SFC
    PMOS_RESOURCE                   resAvsLineTileBuffer;                    // AVS Column Store buffer used by SFC
    PMOS_RESOURCE                   resSfdLineTileBuffer;                    // SFD Column Store buffer used by SFC
};
using PMHW_SFC_STATE_PARAMS_G12 = MHW_SFC_STATE_PARAMS_G12*;

class MhwSfcInterfaceG12 : public MhwSfcInterfaceGeneric<mhw_sfc_g12_X>
{
public:
    MhwSfcInterfaceG12(PMOS_INTERFACE pOsInterface);

    virtual ~MhwSfcInterfaceG12()
    {

    }

    MOS_STATUS AddSfcLock(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_LOCK_PARAMS           pSfcLockParams);

    MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_STATE_PARAMS          pSfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface);

    MOS_STATUS AddSfcAvsState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_AVS_STATE             pSfcAvsState);

    MOS_STATUS AddSfcFrameStart(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        uint8_t                        sfcPipeMode);

    MOS_STATUS AddSfcIefState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_IEF_STATE_PARAMS      pSfcIefStateParams);

    MOS_STATUS AddSfcAvsChromaTable(
        PMOS_COMMAND_BUFFER             pCmdBuffer,
        PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable);

    MOS_STATUS AddSfcAvsLumaTable(
        PMOS_COMMAND_BUFFER             pCmdBuffer,
        PMHW_SFC_AVS_LUMA_TABLE         pLumaTable);

    MOS_STATUS SetSfcSamplerTable(
        PMHW_SFC_AVS_LUMA_TABLE         pLumaTable,
        PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable,
        PMHW_AVS_PARAMS                 pAvsParams,
        MOS_FORMAT                      SrcFormat,
        float                           fScaleX,
        float                           fScaleY,
        uint32_t                        dwChromaSiting,
        bool                            bUse8x8Filter);

    //!
    //! \brief      get Output centering wheter enable
    //! \param      [in] inputEnable
    //!             wheter enable the Output center.
    //! \return     void
    //!
    void IsOutPutCenterEnable(
        bool                            inputEnable);
public:
    enum SFC_PIPE_MODE_G12
    {
        SFC_PIPE_MODE_HCP = 2
    };

    enum VD_VE_ORDER_MODE
    {
        LCU_16_16_HEVC = 0,
        LCU_32_32_HEVC = 1,
        LCU_64_64_HEVC = 2,
        LCU_64_64_VP9 = 3,
        LCU_64_64_VP9_ENC = 4
    };

protected:
    bool m_outputCenteringEnable = true;
};
#endif // __MHW_SFC_G12_X_H__
