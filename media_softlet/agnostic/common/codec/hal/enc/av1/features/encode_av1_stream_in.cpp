/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_stream_in.cpp
//! \brief    Defines the common interface for encode av1 stream in utility
//!

#include "encode_av1_stream_in.h"
#include "encode_tile.h"
#include "encode_av1_basic_feature.h"
#include "mos_solo_generic.h"

namespace encode
{
    Av1StreamIn::~Av1StreamIn()
    {
        ENCODE_FUNC_CALL();
        if (m_LcuMap != nullptr)
        {
            MOS_FreeMemory(m_LcuMap);
        }

        MOS_SafeFreeMemory(m_streamInTemp);
    }

    static void SetCommonParams(uint8_t tu, CommonStreamInParams& params)
    {
        params.MaxCuSize = 3;
        params.MaxTuSize = 3;
        switch (tu)
        {
        case 2:
            params.NumImePredictors         = 12;
            params.NumMergeCandidateCu8x8   = 3;
            params.NumMergeCandidateCu16x16 = 3;
            params.NumMergeCandidateCu32x32 = 3;
            params.NumMergeCandidateCu64x64 = 3;
            break;
        case 4:
            params.NumImePredictors         = 8;
            params.NumMergeCandidateCu8x8   = 2;
            params.NumMergeCandidateCu16x16 = 2;
            params.NumMergeCandidateCu32x32 = 3;
            params.NumMergeCandidateCu64x64 = 3;
            break;
        case 7:
            params.NumImePredictors         = 4;
            params.NumMergeCandidateCu8x8   = 2;
            params.NumMergeCandidateCu16x16 = 1;
            params.NumMergeCandidateCu32x32 = 2;
            params.NumMergeCandidateCu64x64 = 2;
            break;
        default:
            MHW_ASSERTMESSAGE("Invalid TU provided!");
        }
    }

    MOS_STATUS Av1StreamIn::Init(Av1BasicFeature *basicFeature, EncodeAllocator *allocator, PMOS_INTERFACE osInterface)
    {
        ENCODE_FUNC_CALL();
        m_basicFeature = basicFeature;
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        m_allocator = allocator;
        ENCODE_CHK_NULL_RETURN(m_allocator);
        m_osInterface = osInterface;
        ENCODE_CHK_NULL_RETURN(m_osInterface);

        SetCommonParams(m_basicFeature->m_targetUsage, m_commonPar);

        if (!m_initialized || m_basicFeature->m_resolutionChanged)
        {
            MOS_ALLOC_GFXRES_PARAMS allocParams;
            MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParams.Type = MOS_GFXRES_BUFFER;
            allocParams.TileType = MOS_TILE_LINEAR;
            allocParams.Format = Format_Buffer;

            auto CurFrameWidth  = m_basicFeature->m_av1PicParams->frame_width_minus1 + 1;
            auto CurFrameHeight = m_basicFeature->m_av1PicParams->frame_height_minus1 + 1;

            allocParams.dwBytes = (MOS_ALIGN_CEIL(CurFrameWidth, 64) / m_streamInBlockSize) *
                                  (MOS_ALIGN_CEIL(CurFrameHeight, 64) / m_streamInBlockSize) * CODECHAL_CACHELINE_SIZE;

            m_streamInSize = allocParams.dwBytes;
            m_streamInTemp = (uint8_t *)MOS_AllocAndZeroMemory(m_streamInSize);
            ENCODE_CHK_NULL_RETURN(m_streamInTemp);

            allocParams.pBufName = "Av1 StreamIn Data Buffer";
            allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::StreamInBuffer, allocParams);

            m_widthInLCU  = MOS_ALIGN_CEIL(CurFrameWidth, 64) / 64;
            m_heightInLCU = MOS_ALIGN_CEIL(CurFrameHeight, 64) / 64;

            if (m_LcuMap != nullptr)
            {
                MOS_FreeMemory(m_LcuMap);
                m_LcuMap = nullptr;
            }

            if (m_LcuMap == nullptr)
            {
                m_LcuMap = static_cast<uint32_t *>(MOS_AllocAndZeroMemory(m_widthInLCU * m_heightInLCU * sizeof(uint32_t)));
            }
            ENCODE_CHK_STATUS_RETURN(SetupLCUMap());
            m_initialized = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1StreamIn::Update()
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            ENCODE_CHK_NULL_RETURN(m_streamInTemp);
            MOS_ZeroMemory(m_streamInTemp, m_streamInSize);

            ENCODE_CHK_STATUS_RETURN(StreamInInit(m_streamInTemp));

            m_enabled = true;
        }
        return MOS_STATUS_SUCCESS;
    }

    static MOS_STATUS CalculateTilesBoundary(
        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams,
        uint32_t* rowBd,
        uint32_t* colBd)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(av1PicParams);
        ENCODE_CHK_NULL_RETURN(rowBd);
        ENCODE_CHK_NULL_RETURN(colBd);

        for (uint32_t i = 0; i < av1PicParams->tile_cols; i++)
        {
            colBd[i + 1] = colBd[i] + av1PicParams->width_in_sbs_minus_1[i] + 1;
        }

        for (uint32_t i = 0; i < av1PicParams->tile_rows; i++)
        {
            rowBd[i + 1] = rowBd[i] + av1PicParams->height_in_sbs_minus_1[i] + 1;
        }

        return MOS_STATUS_SUCCESS;

    }

    void Av1StreamIn::Reset()
    {
        ENCODE_FUNC_CALL();
        m_enabled = false;
    }

    uint32_t Av1StreamIn::GetLCUAddr(uint32_t x, uint32_t y) const
    {
        ENCODE_FUNC_CALL();

        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams =
            m_basicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(av1PicParams);

        uint32_t colBd[m_maxTileBdNum] = { 0 };
        uint32_t rowBd[m_maxTileBdNum] = { 0 };
        ENCODE_CHK_STATUS_RETURN(CalculateTilesBoundary(av1PicParams, &rowBd[0], &colBd[0]));

        uint32_t tileX = 0, tileY = 0, CtbAddrRStoTS = 0;
        for (auto j = 0; j < av1PicParams->tile_cols; j++)
        {
            if (x >= colBd[j])
            {
                tileX = j;
            }
        }

        for (auto k = 0; k < av1PicParams->tile_rows; k++)
        {
            if (y >= rowBd[k])
            {
                tileY = k;
            }
        }

        uint32_t widthinLCU = MOS_ROUNDUP_DIVIDE((av1PicParams->frame_width_minus1 +1), 64);

        CtbAddrRStoTS = widthinLCU * rowBd[tileY] + (av1PicParams->height_in_sbs_minus_1[tileY] + 1) * colBd[tileX];
        CtbAddrRStoTS += (y - rowBd[tileY]) * (av1PicParams->width_in_sbs_minus_1[tileX] + 1 ) + x - colBd[tileX];

        return CtbAddrRStoTS;
    }

    MOS_STATUS Av1StreamIn::SetupLCUMap()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        for (uint32_t y = 0; y < m_heightInLCU; y++)
        {
            for (uint32_t x = 0; x < m_widthInLCU; x++)
            {
                uint32_t  Lcu_Offset = GetLCUAddr(x, y);
                uint32_t lcu_Index = y * m_widthInLCU + x;
                m_LcuMap[lcu_Index] = Lcu_Offset;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    uint32_t Av1StreamIn::GetCuOffset(uint32_t xIdx, uint32_t yIdx) const
    {
        uint32_t lcu_Index = yIdx / m_num32x32BlocksInLCUOnedimension * m_widthInLCU + xIdx / m_num32x32BlocksInLCUOnedimension;
        uint32_t Lcu_Offset = m_LcuMap[lcu_Index];
        return Lcu_Offset * m_num32x32BlocksInLCU + (yIdx % m_num32x32BlocksInLCUOnedimension) * m_num32x32BlocksInLCUOnedimension +
            (xIdx % m_num32x32BlocksInLCUOnedimension);
    }

    VdencStreamInState* Av1StreamIn::GetStreamInBuffer()
    {
        ENCODE_FUNC_CALL();

        return (VdencStreamInState *)m_streamInTemp;
    }

    MOS_STATUS Av1StreamIn::ReturnStreamInBuffer()
    {
        ENCODE_FUNC_CALL();

        m_streamInBuffer = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
        ENCODE_CHK_NULL_RETURN(m_streamInBuffer);

        uint8_t *streaminBuffer = (uint8_t *)m_allocator->LockResourceForWrite(m_streamInBuffer);
        ENCODE_CHK_NULL_RETURN(streaminBuffer);

        MOS_SecureMemcpy(streaminBuffer, m_streamInSize, m_streamInTemp, m_streamInSize);

        m_allocator->UnLock(m_streamInBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1StreamIn::StreamInInit(uint8_t *streamInBuffer)
    {
        ENCODE_CHK_NULL_RETURN(m_osInterface);
        uint16_t numLCUs = m_widthInLCU * m_heightInLCU;
        memset(streamInBuffer, 0, numLCUs * m_num32x32BlocksInLCU * sizeof(VdencStreamInState));
        Av1FrameType frame_type = static_cast<Av1FrameType>(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type);
        MEDIA_WA_TABLE *pWaTable   = m_osInterface->pfnGetWaTable(m_osInterface);
        ENCODE_CHK_NULL_RETURN(pWaTable);

        for (uint16_t LcuAddr = 0; LcuAddr < numLCUs; LcuAddr++)
        {
            for (uint16_t CuAddr = 0; CuAddr < m_num32x32BlocksInLCU; CuAddr++)
            {
                VdencStreamInState* pStreamIn32x32 = (VdencStreamInState *)(streamInBuffer) + LcuAddr * m_num32x32BlocksInLCU + CuAddr;

                if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && frame_type == keyFrame && !m_osInterface->bSimIsActive && !Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext))
                {
                    pStreamIn32x32->DW0.MaxCuSize                = 3;
                    pStreamIn32x32->DW0.MaxTuSize                = 3;
                    pStreamIn32x32->DW0.NumImePredictors         = 0;
                    pStreamIn32x32->DW6.NumMergeCandidateCu8x8   = 2;
                    pStreamIn32x32->DW6.NumMergeCandidateCu16x16 = 0;
                    pStreamIn32x32->DW6.NumMergeCandidateCu32x32 = 0;
                    pStreamIn32x32->DW6.NumMergeCandidateCu64x64 = 0;
                }
                else
                {
                    pStreamIn32x32->DW0.MaxCuSize                = m_commonPar.MaxCuSize;
                    pStreamIn32x32->DW0.MaxTuSize                = m_commonPar.MaxTuSize;
                    pStreamIn32x32->DW0.NumImePredictors         = m_commonPar.NumImePredictors;
                    pStreamIn32x32->DW6.NumMergeCandidateCu8x8   = m_commonPar.NumMergeCandidateCu8x8;
                    pStreamIn32x32->DW6.NumMergeCandidateCu16x16 = m_commonPar.NumMergeCandidateCu16x16;
                    pStreamIn32x32->DW6.NumMergeCandidateCu32x32 = m_commonPar.NumMergeCandidateCu32x32;
                    pStreamIn32x32->DW6.NumMergeCandidateCu64x64 = m_commonPar.NumMergeCandidateCu64x64;
                }
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    const CommonStreamInParams& Av1StreamIn::GetCommonParams() const
    {
        return m_commonPar;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1StreamIn)
    {
        if (m_enabled)
        {
            ENCODE_CHK_NULL_RETURN(m_basicFeature);
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

            params.streamInBuffer = m_streamInBuffer;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1StreamIn)
    {
        params.vdencStreamIn = m_enabled;

        return MOS_STATUS_SUCCESS;
    }
}  // namespace encode
