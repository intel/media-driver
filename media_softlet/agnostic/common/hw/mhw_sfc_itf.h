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
//! \file     mhw_sfc_itf.h
//! \brief    MHW SFC interface common base
//! \details
//!

#ifndef __MHW_SFC_ITF_H__
#define __MHW_SFC_ITF_H__

#include "mhw_itf.h"
#include "mhw_sfc_cmdpar.h"

#define _SFC_CMD_DEF(DEF)                    \
    DEF(SFC_LOCK);                           \
    DEF(SFC_STATE);                          \
    DEF(SFC_AVS_STATE);                      \
    DEF(SFC_FRAME_START);                    \
    DEF(SFC_IEF_STATE);                      \
    DEF(SFC_AVS_CHROMA_Coeff_Table);         \
    DEF(SFC_AVS_LUMA_Coeff_Table)

namespace mhw
{
namespace sfc
{
class Itf
{
public:

    enum CommandsNumberOfAddresses
    {
        SFC_STATE_CMD_NUMBER_OF_ADDRESSES                  = 8,
        SFC_AVS_LUMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES   = 0,
        SFC_AVS_CHROMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES = 0,
        SFC_AVS_STATE_CMD_NUMBER_OF_ADDRESSES              = 0,
        SFC_FRAME_START_CMD_NUMBER_OF_ADDRESSES            = 0,
        SFC_IEF_STATE_CMD_NUMBER_OF_ADDRESSES              = 0,
        SFC_LOCK_CMD_NUMBER_OF_ADDRESSES                   = 0,
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;
        _SFC_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetSfcSamplerTable(
        SFC_AVS_LUMA_Coeff_Table_PAR   *pLumaTable,
        SFC_AVS_CHROMA_Coeff_Table_PAR *pChromaTable,
        PMHW_AVS_PARAMS           pAvsParams,
        MOS_FORMAT                SrcFormat,
        float                     fScaleX,
        float                     fScaleY,
        uint32_t                  dwChromaSiting,
        bool                      bUse8x8Filter,
        float                     fHPStrength,
        float                     fLanczosT) = 0;

    //!
    //! \brief      get Output centering wheter enable
    //! \param      [in] inputEnable
    //!             wheter enable the Output center.
    //! \return     void
    //!
    virtual void IsOutPutCenterEnable(
        bool inputEnable) = 0;

    //!
    //! \brief    Set which Sfc can be used by HW
    //! \details  VPHAL set which Sfc can be use by HW
    //! \param    [in] dwSfcIndex;
    //!           set which Sfc can be used by HW
    //! \param    [in] dwSfcCount;
    //!           set Sfc Count
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSfcIndex(
        uint32_t dwSfcIndex,
        uint32_t dwSfcCount) = 0;

    virtual MOS_STATUS GetInputFrameWidthHeightAlignUnit(
        uint32_t &                widthAlignUnit,
        uint32_t &                heightAlignUnit,
        bool                      bVdbox,
        CODECHAL_STANDARD         codecStandard,
        CodecDecodeJpegChromaType jpegChromaType) = 0;

    virtual MOS_STATUS GetInputMinWidthHeightInfo(uint32_t &width, uint32_t &height) = 0;
    virtual MOS_STATUS GetOutputMinWidthHeightInfo(uint32_t &width, uint32_t &height) = 0;

    virtual MOS_STATUS GetMinWidthHeightInfo(uint32_t &width, uint32_t &height) = 0;

    virtual MOS_STATUS GetMaxWidthHeightInfo(uint32_t &width, uint32_t &height) = 0;

    virtual MOS_STATUS GetScalingRatioLimit(float &minScalingRatio, float &maxScalingRatio) = 0;

        //!
        //! \brief      Sets AVS Scaling Mode. Will configure the different coefficients of 8-Tap polyphase filter according to scaling mode.
        //! \param      [in] ScalingMode
        //!             AVS scaling mode e.g. Nearest, 8-Tap polyphase etc.
        //! \return     MOS_STATUS
        //!
        virtual MOS_STATUS SetSfcAVSScalingMode(
            MHW_SCALING_MODE ScalingMode) = 0;

    _SFC_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

MEDIA_CLASS_DEFINE_END(mhw__sfc__Itf)
};
}  // namespace vebox
}  // namespace mhw
#endif  // __MHW_VEBOX_ITF_H__
