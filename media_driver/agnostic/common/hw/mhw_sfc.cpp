/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_sfc.cpp
//! \brief    MHW interface for constructing commands for the SFC
//! \details  Impelements the functionalities common across all platforms for MHW_SFC
//!

#include "mhw_sfc.h"
#include "mhw_utilities.h"

MhwSfcInterface::MhwSfcInterface(PMOS_INTERFACE pOsInterface)
{
    MHW_FUNCTION_ENTER;

    if (pOsInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid input pointers provided");
        return;
    }
    if (!pOsInterface->bUsesGfxAddress && !pOsInterface->bUsesPatchList)
    {
        MHW_ASSERTMESSAGE("No valid addressing mode indicated");
        return;
    }

    m_osInterface = pOsInterface;
    memset(&m_outputSurfCtrl, 0, sizeof(m_outputSurfCtrl));
    memset(&m_avsLineBufferCtrl, 0, sizeof(m_avsLineBufferCtrl));
    memset(&m_iefLineBufferCtrl, 0, sizeof(m_iefLineBufferCtrl));

    if (m_osInterface->bUsesGfxAddress)
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else  //PatchList
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
}

void MhwSfcInterface::SetSfcAVSChromaTable(
    PSFC_AVS_CHROMA_FILTER_COEFF        pUVCoeffTable,
    int32_t                             *piUVCoefsX,
    int32_t                             *piUVCoefsY)
{
    int32_t i;

    MHW_ASSERT(pUVCoeffTable);
    MHW_ASSERT(piUVCoefsX);
    MHW_ASSERT(piUVCoefsY);

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

void MhwSfcInterface::SetSfcAVSLumaTable(
    MOS_FORMAT                      SrcFormat,
    PSFC_AVS_LUMA_FILTER_COEFF      pCoeffTable,
    int32_t                         *piYCoefsX,
    int32_t                         *piYCoefsY,
    bool                            bUse8x8Filter)
{
    int32_t i;

    MHW_ASSERT(pCoeffTable);
    MHW_ASSERT(piYCoefsX);
    MHW_ASSERT(piYCoefsY);

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

MOS_STATUS MhwSfcInterface::SetSfcSamplerTable(
    PMHW_SFC_AVS_LUMA_TABLE         pLumaTable,
    PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting,
    bool                            bUse8x8Filter)
{
    int32_t                             iPhaseOffset;

    float       fHPStrength;
    int32_t     *piYCoefsX, *piYCoefsY;
    int32_t     *piUVCoefsX, *piUVCoefsY;
    MHW_PLANE   Plane;

    MHW_CHK_NULL_RETURN(pLumaTable);
    MHW_CHK_NULL_RETURN(pChromaTable);
    MHW_CHK_NULL_RETURN(pAvsParams);

    fHPStrength = 0.0F;
    piYCoefsX   = pAvsParams->piYCoefsX;
    piYCoefsY   = pAvsParams->piYCoefsY;
    piUVCoefsX  = pAvsParams->piUVCoefsX;
    piUVCoefsY  = pAvsParams->piUVCoefsY;

    //  Skip calculation if no changes to AVS parameters
    if (SrcFormat == pAvsParams->Format  &&
        fScaleX   == pAvsParams->fScaleX &&
        fScaleY   == pAvsParams->fScaleY)
    {
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

    // Recalculate Horizontal scaling table
    if (SrcFormat != pAvsParams->Format || fScaleX != pAvsParams->fScaleX)
    {
        MOS_ZeroMemory(
            piYCoefsX,
            8 * 32 * sizeof(int32_t));

        MOS_ZeroMemory(
            piUVCoefsX,
            4 * 32 * sizeof(int32_t));

        // 4-tap filtering for RGB format G-channel.
        Plane = IS_RGB32_FORMAT(SrcFormat) ? MHW_U_PLANE : MHW_Y_PLANE;

        pAvsParams->fScaleX = fScaleX;

        // For 1x scaling in horizontal direction and not force polyphase coefs, use special coefficients for filtering
        if (fScaleX == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
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
            // Clamp the Scaling Factor if > 1.0x
            fScaleX = MOS_MIN(1.0F, fScaleX);

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsX,
                fScaleX,
                Plane,
                SrcFormat,
                fHPStrength,
                bUse8x8Filter,
                NUM_HW_POLYPHASE_TABLES));
        }

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
            // Chroma siting offset needs to be added
            if (dwChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER)
            {
                iPhaseOffset = MOS_UF_ROUND(0.5F * 16.0F);   // U0.4
            }
            else //if (ChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT)
            {
                iPhaseOffset = MOS_UF_ROUND(1.0F * 16.0F);   // U0.4
            }

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUVOffset(
                piUVCoefsX,
                3.0F,
                fScaleX,
                iPhaseOffset));
        }
    }

    // Recalculate Vertical scaling table
    if (SrcFormat != pAvsParams->Format || fScaleY != pAvsParams->fScaleY)
    {
        MOS_ZeroMemory(piYCoefsY, 8 * 32 * sizeof(int32_t));

        MOS_ZeroMemory(piUVCoefsY, 4 * 32 * sizeof(int32_t));

        // 4-tap filtering for RGB format G-channel.
        Plane = IS_RGB32_FORMAT(SrcFormat) ? MHW_U_PLANE : MHW_Y_PLANE;

        pAvsParams->fScaleY = fScaleY;

        // For 1x scaling in vertical direction and not force polyphase coefs, use special coefficients for filtering
        if (fScaleY == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
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
            // Clamp the Scaling Factor if > 1.0x
            fScaleY = MOS_MIN(1.0F, fScaleY);

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsY,
                fScaleY,
                Plane,
                SrcFormat,
                fHPStrength,
                bUse8x8Filter,
                NUM_HW_POLYPHASE_TABLES));
        }

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
            // Chroma siting offset needs to be added
            if (dwChromaSiting & MHW_CHROMA_SITING_VERT_CENTER)
            {
                iPhaseOffset = MOS_UF_ROUND(0.5F * 16.0F);   // U0.4
            }
            else //if (ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
            {
                iPhaseOffset = MOS_UF_ROUND(1.0F * 16.0F);   // U0.4
            }

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUVOffset(
                piUVCoefsY,
                3.0F,
                fScaleY,
                iPhaseOffset));
        }
    }

    // Save format used to calculate AVS parameters
    pAvsParams->Format = SrcFormat;

    SetSfcAVSLumaTable(
        SrcFormat,
        pLumaTable->LumaTable,
        piYCoefsX,
        piYCoefsY,
        false);

    SetSfcAVSChromaTable(
        pChromaTable->ChromaTable,
        piUVCoefsX,
        piUVCoefsY);

    return MOS_STATUS_SUCCESS;
}
