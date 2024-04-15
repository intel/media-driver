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
//! \file     mhw_sfc_impl.h
//! \brief    MHW SFC interface common base
//! \details
//!

#ifndef __MHW_SFC_IMPL_H__
#define __MHW_SFC_IMPL_H__

#include "mhw_sfc_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace sfc
{
template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _SFC_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;

        MOS_ZeroMemory(&m_outputSurfCtrl, sizeof(m_outputSurfCtrl));
        MOS_ZeroMemory(&m_avsLineBufferCtrl, sizeof(m_avsLineBufferCtrl));
        MOS_ZeroMemory(&m_iefLineBufferCtrl, sizeof(m_iefLineBufferCtrl));
        MOS_ZeroMemory(&m_sfdLineBufferCtrl, sizeof(m_sfdLineBufferCtrl));

        m_scalingMode = MHW_SCALING_AVS;

        if (osItf == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid input pointers provided");
            return;
        }
        if (!osItf->bUsesGfxAddress && !osItf->bUsesPatchList)
        {
            MHW_ASSERTMESSAGE("No valid addressing mode indicated");
            return;
        }

        m_sfcScalabilitySupported = false;
        m_sfcScalabilityEnabled   = false;
        m_indexofSfc              = 0;
        m_numofSfc                = 1;

        // Get Memory control object directly from MOS.
        // If any override is needed, something like pfnOverrideMemoryObjectCtrl() / pfnComposeSurfaceCacheabilityControl()
        // will need to be implemented.
        m_outputSurfCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;

        m_avsLineBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;
        m_iefLineBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;

        m_sfdLineBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;
        m_avsLineTileBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;
        m_iefLineTileBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;
        m_sfdLineTileBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;
        m_histogramBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;
        m_sfcIndirectBufferCtrl.Value = osItf->pfnCachePolicyGetMemoryObject(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED,
            osItf->pfnGetGmmClientContext(osItf)).DwordValue;

        m_maxWidth  = MHW_SFC_MAX_WIDTH;
        m_maxHeight = MHW_SFC_MAX_HEIGHT;
    };

    static __inline uint32_t MosGetHWTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
    {
        uint32_t tileMode = 0;

        if (gmmTileEnabled)
        {
            return tileModeGMM;
        }

        switch (tileType)
        {
        case MOS_TILE_LINEAR:
            tileMode = 0;
            break;
        case MOS_TILE_YS:
            tileMode = 1;
            break;
        case MOS_TILE_X:
            tileMode = 2;
            break;
        default:
            tileMode = 3;
            break;
        }
        return tileMode;
    }

    void SetSfcAVSChromaTable(
        PSFC_AVS_CHROMA_FILTER_COEFF pUVCoeffTable,
        int32_t                      *piUVCoefsX,
        int32_t                      *piUVCoefsY)
    {
        int32_t i;

        MHW_CHK_NULL_NO_STATUS_RETURN(pUVCoeffTable);
        MHW_CHK_NULL_NO_STATUS_RETURN(piUVCoefsX);
        MHW_CHK_NULL_NO_STATUS_RETURN(piUVCoefsY);

        for (i = 0; i < NUM_HW_POLYPHASE_TABLES; i++, pUVCoeffTable++)
        {
            pUVCoeffTable->DW0.Table1XFilterCoefficient2 = *(piUVCoefsX++);
            pUVCoeffTable->DW0.Table1XFilterCoefficient3 = *(piUVCoefsX++);
            pUVCoeffTable->DW1.Table1XFilterCoefficient4 = *(piUVCoefsX++);
            pUVCoeffTable->DW1.Table1XFilterCoefficient5 = *(piUVCoefsX++);

            pUVCoeffTable->DW0.Table1YFilterCoefficient2 = *(piUVCoefsY++);
            pUVCoeffTable->DW0.Table1YFilterCoefficient3 = *(piUVCoefsY++);
            pUVCoeffTable->DW1.Table1YFilterCoefficient4 = *(piUVCoefsY++);
            pUVCoeffTable->DW1.Table1YFilterCoefficient5 = *(piUVCoefsY++);
        }
    }

    void SetSfcAVSLumaTable(
        MOS_FORMAT                      SrcFormat,
        PSFC_AVS_LUMA_FILTER_COEFF      pCoeffTable,
        int32_t * piYCoefsX,
        int32_t * piYCoefsY,
        bool                            bUse8x8Filter)
    {
        int32_t i;

        MHW_CHK_NULL_NO_STATUS_RETURN(pCoeffTable);
        MHW_CHK_NULL_NO_STATUS_RETURN(piYCoefsX);
        MHW_CHK_NULL_NO_STATUS_RETURN(piYCoefsY);

        for (i = 0; i < NUM_HW_POLYPHASE_TABLES; i++, pCoeffTable++)
        {
            // 4-tap filtering for G-channel, update only center 4 coeffs.
            if (IS_RGB32_FORMAT(SrcFormat) && (!bUse8x8Filter))
            {
                pCoeffTable->DW0.Table0XFilterCoefficient0 = 0;
                pCoeffTable->DW0.Table0XFilterCoefficient1 = 0;
                pCoeffTable->DW1.Table0XFilterCoefficient2 = *(piYCoefsX++);
                pCoeffTable->DW1.Table0XFilterCoefficient3 = *(piYCoefsX++);
                pCoeffTable->DW2.Table0XFilterCoefficient4 = *(piYCoefsX++);
                pCoeffTable->DW2.Table0XFilterCoefficient5 = *(piYCoefsX++);
                pCoeffTable->DW3.Table0XFilterCoefficient6 = 0;
                pCoeffTable->DW3.Table0XFilterCoefficient7 = 0;

                pCoeffTable->DW0.Table0YFilterCoefficient0 = 0;
                pCoeffTable->DW0.Table0YFilterCoefficient1 = 0;
                pCoeffTable->DW1.Table0YFilterCoefficient2 = *(piYCoefsY++);
                pCoeffTable->DW1.Table0YFilterCoefficient3 = *(piYCoefsY++);
                pCoeffTable->DW2.Table0YFilterCoefficient4 = *(piYCoefsY++);
                pCoeffTable->DW2.Table0YFilterCoefficient5 = *(piYCoefsY++);
                pCoeffTable->DW3.Table0YFilterCoefficient6 = 0;
                pCoeffTable->DW3.Table0YFilterCoefficient7 = 0;
            }
            else
            {
                pCoeffTable->DW0.Table0XFilterCoefficient0 = *(piYCoefsX++);
                pCoeffTable->DW0.Table0XFilterCoefficient1 = *(piYCoefsX++);
                pCoeffTable->DW1.Table0XFilterCoefficient2 = *(piYCoefsX++);
                pCoeffTable->DW1.Table0XFilterCoefficient3 = *(piYCoefsX++);
                pCoeffTable->DW2.Table0XFilterCoefficient4 = *(piYCoefsX++);
                pCoeffTable->DW2.Table0XFilterCoefficient5 = *(piYCoefsX++);
                pCoeffTable->DW3.Table0XFilterCoefficient6 = *(piYCoefsX++);
                pCoeffTable->DW3.Table0XFilterCoefficient7 = *(piYCoefsX++);

                pCoeffTable->DW0.Table0YFilterCoefficient0 = *(piYCoefsY++);
                pCoeffTable->DW0.Table0YFilterCoefficient1 = *(piYCoefsY++);
                pCoeffTable->DW1.Table0YFilterCoefficient2 = *(piYCoefsY++);
                pCoeffTable->DW1.Table0YFilterCoefficient3 = *(piYCoefsY++);
                pCoeffTable->DW2.Table0YFilterCoefficient4 = *(piYCoefsY++);
                pCoeffTable->DW2.Table0YFilterCoefficient5 = *(piYCoefsY++);
                pCoeffTable->DW3.Table0YFilterCoefficient6 = *(piYCoefsY++);
                pCoeffTable->DW3.Table0YFilterCoefficient7 = *(piYCoefsY++);
            }
        }
    }

    MOS_STATUS GetInputFrameWidthHeightAlignUnit(
        uint32_t &widthAlignUnit,
        uint32_t &heightAlignUnit,
        bool bVdbox,
        CODECHAL_STANDARD codecStandard,
        CodecDecodeJpegChromaType jpegChromaType) override
    {
        if (bVdbox)
        {
            if (CODECHAL_JPEG == codecStandard && (jpegYUV400 == jpegChromaType ||
                                                      jpegYUV444 == jpegChromaType || jpegYUV422H2Y == jpegChromaType) ||
                jpegBGR == jpegChromaType || jpegRGB == jpegChromaType)
            {
                widthAlignUnit  = 8;
                heightAlignUnit = 8;
                return MOS_STATUS_SUCCESS;
            }
            else if (CODECHAL_HEVC == codecStandard || CODECHAL_VP9 == codecStandard)
            {
                widthAlignUnit  = 8;
                heightAlignUnit = 8;
                return MOS_STATUS_SUCCESS;
            }
            else if (CODECHAL_AV1 == codecStandard)
            {
                widthAlignUnit  = 1;
                heightAlignUnit = 1;
                return MOS_STATUS_SUCCESS;
            }
            else
            {
                widthAlignUnit  = 16;
                heightAlignUnit = 16;
                return MOS_STATUS_SUCCESS;
            }
        }
        else
        {
            widthAlignUnit  = m_veWidthAlignment;
            heightAlignUnit = m_veHeightAlignment;
            return MOS_STATUS_SUCCESS;
        }
    }

    MOS_STATUS SetSfcAVSScalingMode(
        MHW_SCALING_MODE ScalingMode) override
    {
        m_scalingMode = ScalingMode;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetInputMinWidthHeightInfo(uint32_t &width, uint32_t &height) override
    {
        width  = m_inputMinWidth;
        height = m_inputMinHeight;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetOutputMinWidthHeightInfo(uint32_t &width, uint32_t &height) override
    {
        width  = m_outputMinWidth;
        height = m_outputMinHeight;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetMinWidthHeightInfo(uint32_t &width, uint32_t &height) override
    {
        width = m_minWidth;
        height = m_minHeight;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetMaxWidthHeightInfo(uint32_t &width, uint32_t &height) override
    {
        width  = m_maxWidth;
        height = m_maxHeight;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetScalingRatioLimit(float &minScalingRatio, float &maxScalingRatio) override
    {
        minScalingRatio = m_minScalingRatio;
        maxScalingRatio = m_maxScalingRatio;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetSfcSamplerTable(
        SFC_AVS_LUMA_Coeff_Table_PAR   *pLumaTable,
        SFC_AVS_CHROMA_Coeff_Table_PAR *pChromaTable,
        PMHW_AVS_PARAMS           pAvsParams,
        MOS_FORMAT                srcFormat,
        float                     fScaleX,
        float                     fScaleY,
        uint32_t                  dwChromaSiting,
        bool                      bUse8x8Filter,
        float                     fHPStrength,
        float                     fLanczosT) override
    {
        MHW_FUNCTION_ENTER;

        int32_t * piYCoefsX, *piYCoefsY;
        int32_t * piUVCoefsX, *piUVCoefsY;
        MHW_PLANE Plane;

        MHW_CHK_NULL_RETURN(pLumaTable);
        MHW_CHK_NULL_RETURN(pChromaTable);
        MHW_CHK_NULL_RETURN(pAvsParams);

        fHPStrength = 0.0F;
        piYCoefsX   = pAvsParams->piYCoefsX;
        piYCoefsY   = pAvsParams->piYCoefsY;
        piUVCoefsX  = pAvsParams->piUVCoefsX;
        piUVCoefsY  = pAvsParams->piUVCoefsY;

        // Skip calculation if no changes to AVS parameters
        if (srcFormat == pAvsParams->Format &&
            fScaleX == pAvsParams->fScaleX  &&
            fScaleY == pAvsParams->fScaleY  &&
            bUse8x8Filter == pAvsParams->bUse8x8Filter)
        {
            MHW_NORMALMESSAGE("Skip calculation since no changes to AVS parameters. srcFormat %d, fScaleX %f, fScaleY %f",
                srcFormat, fScaleX, fScaleY);

            SetSfcAVSLumaTable(
                srcFormat,
                pLumaTable->LumaTable,
                piYCoefsX,
                piYCoefsY,
                bUse8x8Filter);

            SetSfcAVSChromaTable(
                pChromaTable->ChromaTable,
                piUVCoefsX,
                piUVCoefsY);
            return MOS_STATUS_SUCCESS;
        }

        // AVS Coefficients don't change for Scaling Factors > 1.0x
        // Hence recalculation is avoided
        if (fScaleX > 1.0F && pAvsParams->fScaleX > 1.0F)
        {
            pAvsParams->fScaleX = fScaleX;
        }

        // AVS Coefficients don't change for Scaling Factors > 1.0x
        // Hence recalculation is avoided
        if (fScaleY > 1.0F && pAvsParams->fScaleY > 1.0F)
        {
            pAvsParams->fScaleY = fScaleY;
        }

        MHW_NORMALMESSAGE("srcFormat %d, pAvsParams->Format %d, fScaleX %f, fScaleY %f, pAvsParams->fScaleX %f, pAvsParams->fScaleY %f, pAvsParams->bUse8x8Filter %d, bUse8x8Filter %d, m_scalingMode %d, bForcePolyPhaseCoefs %d",
            srcFormat, pAvsParams->Format, fScaleX, fScaleY, pAvsParams->fScaleX, pAvsParams->fScaleY, pAvsParams->bUse8x8Filter, bUse8x8Filter, m_scalingMode, (pAvsParams->bForcePolyPhaseCoefs ? 1 : 0));

        // Recalculate Horizontal scaling table
        if (srcFormat != pAvsParams->Format || fScaleX != pAvsParams->fScaleX || pAvsParams->bUse8x8Filter != bUse8x8Filter)
        {
            MOS_ZeroMemory(
                piYCoefsX,
                8 * 32 * sizeof(int32_t));

            MOS_ZeroMemory(
                piUVCoefsX,
                4 * 32 * sizeof(int32_t));

            Plane = (IS_RGB32_FORMAT(srcFormat) && !bUse8x8Filter) ? MHW_U_PLANE : MHW_Y_PLANE;

            pAvsParams->fScaleX = fScaleX;

            if (m_scalingMode == MHW_SCALING_NEAREST)
            {
                MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piYCoefsX,
                    Plane,
                    true));
                MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piUVCoefsX,
                    MHW_U_PLANE,
                    true));
            }
            else
            {
                // For 1x scaling in horizontal direction and not force polyphase coefs, use special coefficients for filtering
                if ((fScaleX == 1.0F && !pAvsParams->bForcePolyPhaseCoefs))
                {
                    MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                        piYCoefsX,
                        Plane,
                        true));

                    // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
                    // So, coefficient for UV/RB channels caculation can be passed
                    if ((!(IS_RGB32_FORMAT(srcFormat) && bUse8x8Filter)))
                    {
                        MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                            piUVCoefsX,
                            MHW_U_PLANE,
                            true));
                    }
                }
                else
                {
                    // Clamp the Scaling Factor if > 1.0x
                    fScaleX = MOS_MIN(1.0F, fScaleX);

                    MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                        piYCoefsX,
                        fScaleX,
                        Plane,
                        srcFormat,
                        fHPStrength,
                        bUse8x8Filter,
                        NUM_HW_POLYPHASE_TABLES,
                        0));
                }

                // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
                // So, coefficient for UV/RB channels caculation can be passed
                if (!(IS_RGB32_FORMAT(srcFormat) && bUse8x8Filter))
                {
                    // If Chroma Siting info is present
                    if (dwChromaSiting & MHW_CHROMA_SITING_HORZ_LEFT)
                    {
                        // No Chroma Siting
                        MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsX,
                            2.0F,
                            fScaleX));
                    }
                    else
                    {
                        // Chroma siting offset will be add in the HW cmd
                        MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsX,
                            3.0F,
                            fScaleX));
                    }
                }
            }
        }

        // Recalculate Vertical scaling table
        if (srcFormat != pAvsParams->Format || fScaleY != pAvsParams->fScaleY || pAvsParams->bUse8x8Filter != bUse8x8Filter)
        {
            MOS_ZeroMemory(piYCoefsY, 8 * 32 * sizeof(int32_t));
            MOS_ZeroMemory(piUVCoefsY, 4 * 32 * sizeof(int32_t));

            Plane = (IS_RGB32_FORMAT(srcFormat) && !bUse8x8Filter) ? MHW_U_PLANE : MHW_Y_PLANE;

            pAvsParams->fScaleY = fScaleY;

            if (m_scalingMode == MHW_SCALING_NEAREST)
            {
                MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piYCoefsY,
                    Plane,
                    true));
                MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piUVCoefsY,
                    MHW_U_PLANE,
                    true));
            }
            else
            {
                // For 1x scaling in vertical direction and not force polyphase coefs, use special coefficients for filtering
                if ((fScaleY == 1.0F && !pAvsParams->bForcePolyPhaseCoefs))
                {
                    MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                        piYCoefsY,
                        Plane,
                        true));

                    // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
                    // So, coefficient for UV/RB channels caculation can be passed
                    if ((!(IS_RGB32_FORMAT(srcFormat) && bUse8x8Filter)))
                    {
                        MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                            piUVCoefsY,
                            MHW_U_PLANE,
                            true));
                    }
                }
                else
                {
                    // Clamp the Scaling Factor if > 1.0x
                    fScaleY = MOS_MIN(1.0F, fScaleY);

                    MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                        piYCoefsY,
                        fScaleY,
                        Plane,
                        srcFormat,
                        fHPStrength,
                        bUse8x8Filter,
                        NUM_HW_POLYPHASE_TABLES,
                        0));
                }

                // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
                // So, coefficient for UV/RB channels caculation can be passed
                if (!(IS_RGB32_FORMAT(srcFormat) && bUse8x8Filter))
                {
                    // If Chroma Siting info is present
                    if (dwChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
                    {
                        // No Chroma Siting
                        MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsY,
                            2.0F,
                            fScaleY));
                    }
                    else
                    {
                        // Chroma siting offset will be add in the HW cmd
                        MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsY,
                            3.0F,
                            fScaleY));
                    }
                }
            }
        }

        // Save format used to calculate AVS parameters
        pAvsParams->Format = srcFormat;
        // Need to recaculate if use8x8Filter changed
        pAvsParams->bUse8x8Filter = bUse8x8Filter;

        SetSfcAVSLumaTable(
            srcFormat,
            pLumaTable->LumaTable,
            piYCoefsX,
            piYCoefsY,
            bUse8x8Filter);

        SetSfcAVSChromaTable(
            pChromaTable->ChromaTable,
            piUVCoefsX,
            piUVCoefsY);

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set which Sfc can be used by HW
    //! \details  VPHAL set which Sfc can be use by HW
    //! \param    [in] dwSfcIndex;
    //!           set which Sfc can be used by HW
    //! \param    [in] dwSfcCount;
    //!           set Sfc Count
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetSfcIndex(
        uint32_t dwSfcIndex,
        uint32_t dwSfcCount) override
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_ASSERT(dwSfcIndex < dwSfcCount);

        m_indexofSfc            = dwSfcIndex;
        m_numofSfc              = dwSfcCount;
        m_sfcScalabilityEnabled = (dwSfcCount > 1) ? true : false;

        return eStatus;
    }

    void IsOutPutCenterEnable(bool inputEnable) override
    {
        m_outputCenteringEnable = inputEnable;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_LOCK)
    {
        _MHW_SETCMD_CALLBASE(SFC_LOCK);

        cmd.DW1.VeSfcPipeSelect                    = (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_VEBOX) ? 1 : 0;
        cmd.DW1.PreScaledOutputSurfaceOutputEnable = params.bOutputToMemory ? 1 : 0;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_FRAME_START)
    {
        _MHW_SETCMD_CALLBASE(SFC_FRAME_START);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_IEF_STATE)
    {
        _MHW_SETCMD_CALLBASE(SFC_IEF_STATE);

        // Init SFC_IEF_STATE_CMD
        cmd.DW1.GainFactor            = 0;
        cmd.DW1.R3XCoefficient        = 0;
        cmd.DW1.R3CCoefficient        = 0;
        cmd.DW2.GlobalNoiseEstimation = 0;
        cmd.DW2.R5XCoefficient        = 0;
        cmd.DW2.R5CxCoefficient       = 0;
        cmd.DW2.R5CCoefficient        = 0;

        cmd.DW3.StdSinAlpha  = 101;
        cmd.DW3.StdCosAlpha  = 79;
        cmd.DW5.DiamondAlpha = 100;
        cmd.DW7.InvMarginVyl = 3300;
        cmd.DW8.InvMarginVyu = 1600;
        cmd.DW10.S0L         = MOS_BITFIELD_VALUE((uint32_t)-5, 11);
        cmd.DW10.YSlope2     = 31;
        cmd.DW12.YSlope1     = 31;
        cmd.DW14.S0U         = 256;
        cmd.DW15.S1U         = 113;
        cmd.DW15.S2U         = MOS_BITFIELD_VALUE((uint32_t)-179, 11);

        // Set IEF Params
        if (params.bIEFEnable)
        {
            cmd.DW1.GainFactor          = params.dwGainFactor;
            cmd.DW1.StrongEdgeThreshold = params.StrongEdgeThreshold;
            cmd.DW1.R3XCoefficient      = params.dwR3xCoefficient;
            cmd.DW1.R3CCoefficient      = params.dwR3cCoefficient;
            cmd.DW2.StrongEdgeWeight    = params.StrongEdgeWeight;
            cmd.DW2.RegularWeight       = params.RegularWeight;
            cmd.DW2.R5XCoefficient      = params.dwR5xCoefficient;
            cmd.DW2.R5CxCoefficient     = params.dwR5cxCoefficient;
            cmd.DW2.R5CCoefficient      = params.dwR5cCoefficient;
            cmd.DW4.VyStdEnable         = params.bVYSTDEnable;
            cmd.DW5.SkinDetailFactor    = params.bSkinDetailFactor;
        }

        // Set CSC Params
        if (params.bCSCEnable)
        {
            MHW_CHK_NULL_RETURN(params.pfCscCoeff);
            MHW_CHK_NULL_RETURN(params.pfCscInOffset);
            MHW_CHK_NULL_RETURN(params.pfCscOutOffset);
            cmd.DW16.TransformEnable = true;

            cmd.DW16.C0 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[0] * 1024.0F);  // S2.10
            cmd.DW16.C1 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[1] * 1024.0F);  // S2.10
            cmd.DW17.C2 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[2] * 1024.0F);  // S2.10

            cmd.DW17.C3 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[3] * 1024.0F);  // S2.10
            cmd.DW18.C4 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[4] * 1024.0F);  // S2.10
            cmd.DW18.C5 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[5] * 1024.0F);  // S2.10

            cmd.DW19.C6 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[6] * 1024.0F);  // S2.10
            cmd.DW19.C7 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[7] * 1024.0F);  // S2.10
            cmd.DW20.C8 = (uint32_t)MOS_F_ROUND(params.pfCscCoeff[8] * 1024.0F);  // S2.10

            cmd.DW21.OffsetIn1 = (uint32_t)MOS_F_ROUND(params.pfCscInOffset[0] * 4.0F);  // S8.2
            cmd.DW22.OffsetIn2 = (uint32_t)MOS_F_ROUND(params.pfCscInOffset[1] * 4.0F);  // S8.2
            cmd.DW23.OffsetIn3 = (uint32_t)MOS_F_ROUND(params.pfCscInOffset[2] * 4.0F);  // S8.2

            cmd.DW21.OffsetOut1 = (uint32_t)MOS_F_ROUND(params.pfCscOutOffset[0] * 4.0F);  // S8.2
            cmd.DW22.OffsetOut2 = (uint32_t)MOS_F_ROUND(params.pfCscOutOffset[1] * 4.0F);  // S8.2
            cmd.DW23.OffsetOut3 = (uint32_t)MOS_F_ROUND(params.pfCscOutOffset[2] * 4.0F);  // S8.2
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_AVS_CHROMA_Coeff_Table)
    {
        _MHW_SETCMD_CALLBASE(SFC_AVS_CHROMA_Coeff_Table);

        //PSFC_AVS_CHROMA_FILTER_COEFF pChromaCoeff;
        //pChromaCoeff = params.ChromaTable;

        // Copy programmed State tables into the command
        MHW_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.DW1),
            sizeof(SFC_AVS_CHROMA_FILTER_COEFF) * 32,
            params.ChromaTable,
            sizeof(SFC_AVS_CHROMA_FILTER_COEFF) * 32));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_AVS_LUMA_Coeff_Table)
    {
        _MHW_SETCMD_CALLBASE(SFC_AVS_LUMA_Coeff_Table);

        //PSFC_AVS_LUMA_FILTER_COEFF pLumaCoeff;
        //pLumaCoeff = params.LumaTable;

        // Copy programmed State tables into the command
        MHW_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.DW1),
            sizeof(SFC_AVS_LUMA_FILTER_COEFF) * 32,
            params.LumaTable,
            sizeof(SFC_AVS_LUMA_FILTER_COEFF) * 32));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_AVS_STATE)
    {
        _MHW_SETCMD_CALLBASE(SFC_AVS_STATE);

       // Inilizatialied the SFC_AVS_STATE_CMD
        cmd.DW1.TransitionAreaWith8Pixels = 5;
        cmd.DW1.TransitionAreaWith4Pixels = 4;
        if (params.dwAVSFilterMode == MEDIASTATE_SFC_AVS_FILTER_BILINEAR)
        {
            cmd.DW1.SharpnessLevel = 0;
        }
        else
        {
            cmd.DW1.SharpnessLevel = 255;
        }

        cmd.DW2.MaxDerivativePoint8  = 20;
        cmd.DW2.MaxDerivative4Pixels = 7;

        return MOS_STATUS_SUCCESS;
    }
 
protected:
    using base_t = Itf;

    bool     m_outputCenteringEnable   = true;
    bool     m_sfcScalabilitySupported = false; 
    bool     m_sfcScalabilityEnabled   = false;
    uint32_t m_indexofSfc              = 0;
    uint32_t m_numofSfc                = 0;

    uint16_t                           m_veWidthAlignment      = MHW_SFC_VE_WIDTH_ALIGN;
    uint16_t                           m_veHeightAlignment     = MHW_SFC_VE_HEIGHT_ALIGN;
    uint32_t                           m_maxWidth              = MHW_SFC_MAX_WIDTH;
    uint32_t                           m_maxHeight             = MHW_SFC_MAX_HEIGHT;
    uint32_t                           m_minWidth              = MHW_SFC_MIN_WIDTH;
    uint32_t                           m_minHeight             = MHW_SFC_MIN_HEIGHT;
    uint32_t                           m_inputMinWidth         = MHW_SFC_MIN_WIDTH;             // SFC input min width size for ve+sfc
    uint32_t                           m_inputMinHeight        = MHW_SFC_MIN_HEIGHT;            // SFC input min height size for ve+sfc
    uint32_t                           m_outputMinWidth        = MHW_SFC_OUTPUT_MIN_WIDTH;      // SFC output min width size for ve+sfc
    uint32_t                           m_outputMinHeight       = MHW_SFC_OUTPUT_MIN_HEIGHT;     // SFC output min height size for ve+sfc

    float                              m_maxScalingRatio       = MHW_SFC_MAX_SCALINGFACTOR;
    float                              m_minScalingRatio       = MHW_SFC_MIN_SCALINGFACTOR;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_outputSurfCtrl        = {};   // Output Frame caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_avsLineBufferCtrl     = {};   // AVS Line Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_iefLineBufferCtrl     = {};   // IEF Line Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_sfdLineBufferCtrl     = {};   // SFD Line Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_avsLineTileBufferCtrl = {};   // AVS Line Tile Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_iefLineTileBufferCtrl = {};   // IEF Line Tile Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_sfdLineTileBufferCtrl = {};   // SFD Line Tile Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_histogramBufferCtrl   = {};   // Histogram Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS   m_sfcIndirectBufferCtrl = {};   // SfcIndirect Buffer caching control bits
    MHW_SCALING_MODE                   m_scalingMode           = MHW_SCALING_NEAREST;
MEDIA_CLASS_DEFINE_END(mhw__sfc__Impl)
};
}  // namespace sfc
}  // namespace mhw

#endif  // __MHW_SFC_IMPL_H__
