/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     mhw_vdbox_hcp_impl.h
//! \brief    MHW VDBOX HCP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_HCP_IMPL_H__
#define __MHW_VDBOX_HCP_IMPL_H__

#include "mhw_vdbox_hcp_itf.h"
#include "mhw_impl.h"
#include "mhw_vdbox_hcp_def.h"

#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO (26 * MHW_CACHELINE_SIZE)       // 18+4+4
#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO_REXT (35 * MHW_CACHELINE_SIZE)  // 27+4+4

#define MHW_HCP_WORST_CASE_CU_TU_INFO (4 * MHW_CACHELINE_SIZE)       // 2+1+1
#define MHW_HCP_WORST_CASE_CU_TU_INFO_REXT (6 * MHW_CACHELINE_SIZE)  // 4+1+1

namespace mhw
{
namespace vdbox
{
namespace hcp
{
static constexpr uint32_t WATCHDOG_COUNT_CTRL_OFFSET_INIT                                  = 0x1C0178;
static constexpr uint32_t WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT                            = 0x1C017C;
static constexpr uint32_t HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT                     = 0x1C2828;
static constexpr uint32_t HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT                        = 0x1C28B8;
static constexpr uint32_t HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT                        = 0x1C28BC;
static constexpr uint32_t HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT              = 0x1C28A0;
static constexpr uint32_t HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT            = 0x1C28A8;
static constexpr uint32_t HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT    = 0x1C28A4;
static constexpr uint32_t HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT                          = 0x1C28C0;
static constexpr uint32_t HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT                              = 0x1C28C8;
static constexpr uint32_t HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT                         = 0x1C28DC;
static constexpr uint32_t HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT           = 0x1C28E0;
static constexpr uint32_t HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT = 0x1C28E4;
static constexpr uint32_t HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT                    = 0x1C28F0;
static constexpr uint32_t HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT                    = 0x1C28F4;
static constexpr uint32_t CS_ENGINE_ID_OFFSET_INIT                                         = 0x1C008C;
static constexpr uint32_t HCP_DEC_STATUS_REG_OFFSET_INIT                                   = 0x1C2800;
static constexpr uint32_t HCP_CABAC_STATUS_REG_OFFSET_INIT                                 = 0x1C2804;
static constexpr uint32_t HCP_FRAME_CRC_REG_OFFSET_INIT                                    = 0x1C2920;
static constexpr uint32_t MEMORY_ADDRESS_ATTRIBUTES_MOCS_CLEAN_MASK                        = 0xFFFFFF81;

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _HCP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);

        size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

    MOS_STATUS GetHcpBufSize(const HcpBufferSizePar &par, uint32_t &size) override
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        uint8_t  bitDepthMultFactor = 0;
        uint32_t mvtSize            = 0;
        uint32_t mvtbSize           = 0;
        uint32_t bufferSize         = 0;
        uint32_t rowStoreSzLCU      = 0;
        uint32_t colStoreSzLCU      = 0;
        double   dbFormatMultFactor = 0;

        uint8_t  maxBitDepth  = par.ucMaxBitDepth;
        uint32_t lcusize      = 1 << par.dwCtbLog2SizeY;
        uint32_t maxFrameSize = par.dwMaxFrameSize;
        // HEVC decoder has WA here, change to lcusize when the WA is removed
        uint32_t              widthInCtb        = MOS_ROUNDUP_DIVIDE(par.dwPicWidth, 16);
        uint32_t              heightInCtb       = MOS_ROUNDUP_DIVIDE(par.dwPicHeight, 16);
        uint32_t              numBaseUnitsInLCU = 1 << (par.dwCtbLog2SizeY - 2);  //size in number of 4x4 in the LCU per column
        HCP_CHROMA_FORMAT_IDC chromaFormat      = (HCP_CHROMA_FORMAT_IDC)par.ucChromaFormat;

        switch (par.bufferType)
        {
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE:
            dbFormatMultFactor = (chromaFormat == HCP_CHROMA_FORMAT_YUV444) ? 1.5 : 1;
            bitDepthMultFactor = (maxBitDepth > 8) ? 2 : 1;
            rowStoreSzLCU      = (uint32_t)(((2 * numBaseUnitsInLCU * dbFormatMultFactor * 128 * bitDepthMultFactor) + 511) / 512);
            bufferSize         = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE:
            dbFormatMultFactor = (chromaFormat == HCP_CHROMA_FORMAT_YUV444) ? 1.5 : 1;
            bitDepthMultFactor = (maxBitDepth > 8) ? 2 : 1;
            rowStoreSzLCU      = (uint32_t)(((2 * numBaseUnitsInLCU * dbFormatMultFactor * 128 * bitDepthMultFactor) + 511) / 512);
            bufferSize         = 2 * rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL:
            dbFormatMultFactor = (chromaFormat == HCP_CHROMA_FORMAT_YUV420) ? 1 : 1.5;
            bitDepthMultFactor = (maxBitDepth > 8) ? 2 : 1;
            colStoreSzLCU      = (uint32_t)(((2 * numBaseUnitsInLCU * dbFormatMultFactor * 128 * bitDepthMultFactor + 3 * 128 * bitDepthMultFactor) + 511) / 512);
            bufferSize         = 2 * colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::MV_UP_RT_COL:
            colStoreSzLCU = 1;
            bufferSize    = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_LINE:
            rowStoreSzLCU = (par.dwCtbLog2SizeY == 6) ? 2 : 1;
            bufferSize    = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE:
            rowStoreSzLCU = (par.dwCtbLog2SizeY == 6) ? 4 : 2;
            bufferSize    = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL:
            colStoreSzLCU = (par.dwCtbLog2SizeY == 6) ? 4 : 2;
            bufferSize    = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_UP_RIGHT_COL:
        {
            uint32_t colStoreSizeLCU[2][3];  //[bitdepth 8/10 or 12][LCU 16/32/64]
            if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
            {
                colStoreSizeLCU[0][0] = 1;
                colStoreSizeLCU[0][1] = 1;
                colStoreSizeLCU[0][2] = 1;
                colStoreSizeLCU[1][0] = 1;
                colStoreSizeLCU[1][1] = 2;
                colStoreSizeLCU[1][2] = 2;
            }
            else if (chromaFormat == HCP_CHROMA_FORMAT_YUV422)
            {
                colStoreSizeLCU[0][0] = 1;
                colStoreSizeLCU[0][1] = 1;
                colStoreSizeLCU[0][2] = 1;
                colStoreSizeLCU[1][0] = 1;
                colStoreSizeLCU[1][1] = 2;
                colStoreSizeLCU[1][2] = 2;
            }
            else
            {
                colStoreSizeLCU[0][0] = 1;
                colStoreSizeLCU[0][1] = 2;
                colStoreSizeLCU[0][2] = 2;
                colStoreSizeLCU[1][0] = 2;
                colStoreSizeLCU[1][1] = 3;
                colStoreSizeLCU[1][2] = 3;
            }
            colStoreSzLCU = colStoreSizeLCU[(maxBitDepth == 8) ? 0 : 1][par.dwCtbLog2SizeY > 3 ? par.dwCtbLog2SizeY - 4 : 0];
            bufferSize    = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
            break;
        }
        case HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_LFT_RECON_COL:
        {
            uint32_t colStoreSizeLCU[2][3];  //[bitdepth 8/10 or 12][LCU 16/32/64]
            if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
            {
                colStoreSizeLCU[0][0] = 1;
                colStoreSizeLCU[0][1] = 2;
                colStoreSizeLCU[0][2] = 2;
                colStoreSizeLCU[1][0] = 1;
                colStoreSizeLCU[1][1] = 2;
                colStoreSizeLCU[1][2] = 4;
            }
            else if (chromaFormat == HCP_CHROMA_FORMAT_YUV422)
            {
                colStoreSizeLCU[0][0] = 1;
                colStoreSizeLCU[0][1] = 2;
                colStoreSizeLCU[0][2] = 3;
                colStoreSizeLCU[1][0] = 2;
                colStoreSizeLCU[1][1] = 3;
                colStoreSizeLCU[1][2] = 6;
            }
            else
            {
                colStoreSizeLCU[0][0] = 1;
                colStoreSizeLCU[0][1] = 2;
                colStoreSizeLCU[0][2] = 3;
                colStoreSizeLCU[1][0] = 2;
                colStoreSizeLCU[1][1] = 3;
                colStoreSizeLCU[1][2] = 6;
            }
            colStoreSzLCU = colStoreSizeLCU[(maxBitDepth == 8) ? 0 : 1][par.dwCtbLog2SizeY > 3 ? par.dwCtbLog2SizeY - 4 : 0];
            bufferSize    = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
            break;
        }
        case HCP_INTERNAL_BUFFER_TYPE::SAO_LINE:
        {
            uint32_t uiRowStoreSizeLCU[2][3];  //[bitdepth 8 or10/12][LCU 16/32/64]
            if (chromaFormat == HCP_CHROMA_FORMAT_YUV420 || chromaFormat == HCP_CHROMA_FORMAT_YUV422)
            {
                uiRowStoreSizeLCU[0][0] = 2;
                uiRowStoreSizeLCU[0][1] = 3;
                uiRowStoreSizeLCU[0][2] = 5;
                uiRowStoreSizeLCU[1][0] = 2;
                uiRowStoreSizeLCU[1][1] = 4;
                uiRowStoreSizeLCU[1][2] = 6;
            }
            else
            {
                uiRowStoreSizeLCU[0][0] = 3;
                uiRowStoreSizeLCU[0][1] = 4;
                uiRowStoreSizeLCU[0][2] = 7;
                uiRowStoreSizeLCU[1][0] = 3;
                uiRowStoreSizeLCU[1][1] = 5;
                uiRowStoreSizeLCU[1][2] = 8;
            }
            rowStoreSzLCU = uiRowStoreSizeLCU[(maxBitDepth < 12) ? 0 : 1][par.dwCtbLog2SizeY > 3 ? par.dwCtbLog2SizeY - 4 : 0];
            bufferSize    = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
            break;
        }
        case HCP_INTERNAL_BUFFER_TYPE::SAO_TILE_LINE:
        {
            uint32_t uiRowStoreSizeLCU[2][3];  //[bitdepth 8 or 10/12][LCU 16/32/64]
            if (chromaFormat == HCP_CHROMA_FORMAT_YUV420 || chromaFormat == HCP_CHROMA_FORMAT_YUV422)
            {
                uiRowStoreSizeLCU[0][0] = 4;
                uiRowStoreSizeLCU[0][1] = 6;
                uiRowStoreSizeLCU[0][2] = 10;
                uiRowStoreSizeLCU[1][0] = 4;
                uiRowStoreSizeLCU[1][1] = 8;
                uiRowStoreSizeLCU[1][2] = 12;
            }
            else
            {
                uiRowStoreSizeLCU[0][0] = 6;
                uiRowStoreSizeLCU[0][1] = 8;
                uiRowStoreSizeLCU[0][2] = 14;
                uiRowStoreSizeLCU[1][0] = 6;
                uiRowStoreSizeLCU[1][1] = 10;
                uiRowStoreSizeLCU[1][2] = 16;
            }
            rowStoreSzLCU = uiRowStoreSizeLCU[(maxBitDepth < 12) ? 0 : 1][par.dwCtbLog2SizeY > 3 ? par.dwCtbLog2SizeY - 4 : 0];
            bufferSize    = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
            break;
        }
        case HCP_INTERNAL_BUFFER_TYPE::SAO_TILE_COL:
        {
            // [chroma_format_idc][lcu_size] = [420/422/444][lcu16/lcu32/lcu64]
            uint32_t formatMultFactorTab[3][3] = {{8, 10, 18}, {10, 14, 24}, {10, 14, 24}};
            uint32_t formatMultFactor;

            if (chromaFormat == HCP_CHROMA_FORMAT_MONOCHROME)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                MHW_ASSERTMESSAGE("invalid input chroma format.\n");
                return eStatus;
            }

            formatMultFactor = formatMultFactorTab[chromaFormat - 1][par.dwCtbLog2SizeY > 3 ? par.dwCtbLog2SizeY - 4 : 0];
            colStoreSzLCU    = formatMultFactor;
            bufferSize       = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
            break;
        }
        case HCP_INTERNAL_BUFFER_TYPE::HSAO_RS:
        {
            uint32_t maxTileColumn = MOS_ROUNDUP_DIVIDE(par.dwPicWidth, 128);
            bufferSize             = MOS_ALIGN_CEIL(widthInCtb + 3 * maxTileColumn, 4) * 16;
            break;
        }
        //Add HSSE here
        case HCP_INTERNAL_BUFFER_TYPE::CURR_MV_TEMPORAL:
            mvtSize    = ((((par.dwPicWidth + 63) >> 6) * (((par.dwPicHeight + 15) >> 4)) + 1) & (-2));
            mvtbSize   = ((((par.dwPicWidth + 31) >> 5) * (((par.dwPicHeight + 31) >> 5)) + 1) & (-2));
            bufferSize = MOS_MAX(mvtSize, mvtbSize) * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::CABAC_STREAMOUT:
            //From sas, cabac stream out buffer size =
            //(#LCU) in picture * (Worst case LCU_CU_TU_info) + 1 byte aligned per LCU + Bitstream Size * 3
            if ((chromaFormat == HCP_CHROMA_FORMAT_YUV420) && (maxBitDepth == 8))
            {
                bufferSize = widthInCtb * heightInCtb * MHW_HCP_WORST_CASE_CU_TU_INFO + widthInCtb * heightInCtb + maxFrameSize * 3;
            }
            else
            {
                bufferSize = widthInCtb * heightInCtb * MHW_HCP_WORST_CASE_CU_TU_INFO_REXT + widthInCtb * heightInCtb + maxFrameSize * 3;
            }
            bufferSize = MOS_ALIGN_CEIL(bufferSize, MHW_CACHELINE_SIZE);
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        size = bufferSize;

        return eStatus;
    }

    MOS_STATUS GetVP9BufSize(const HcpBufferSizePar &par, uint32_t &size) override
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        uint32_t bufferSize            = 0;
        uint32_t dblkRsbSizeMultiplier = 0;
        uint32_t dblkCsbSizeMultiplier = 0;
        uint32_t intraPredMultiplier   = 0;

        uint8_t               maxBitDepth   = par.ucMaxBitDepth;
        uint32_t              widthInSb     = par.dwPicWidth;
        uint32_t              heightInSb    = par.dwPicHeight;
        uint32_t              widthInMinCb  = widthInSb * 64 / 8;  //using smallest cb to get max width
        uint32_t              heightInMinCb = heightInSb * 64 / 8;
        HCP_CHROMA_FORMAT_IDC chromaFormat  = (HCP_CHROMA_FORMAT_IDC)par.ucChromaFormat;
        uint32_t              maxFrameSize  = par.dwMaxFrameSize;

        if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
        {
            dblkRsbSizeMultiplier = (maxBitDepth > 8) ? 36 : 18;
            dblkCsbSizeMultiplier = (maxBitDepth > 8) ? 34 : 17;
            intraPredMultiplier   = (maxBitDepth > 8) ? 4 : 2;
        }
        else if (chromaFormat == HCP_CHROMA_FORMAT_YUV444)
        {
            dblkRsbSizeMultiplier = (maxBitDepth > 8) ? 54 : 27;
            dblkCsbSizeMultiplier = (maxBitDepth > 8) ? 50 : 25;
            intraPredMultiplier   = (maxBitDepth > 8) ? 6 : 3;
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("Format not supported.");
            return eStatus;
        }

        switch (par.bufferType)
        {
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE:
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE:
            bufferSize = widthInSb * dblkRsbSizeMultiplier * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL:
            bufferSize = heightInSb * dblkCsbSizeMultiplier * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_LINE:
        case HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE:
            bufferSize = widthInSb * 5 * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL:
            bufferSize = heightInSb * 5 * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::CURR_MV_TEMPORAL:
        case HCP_INTERNAL_BUFFER_TYPE::COLL_MV_TEMPORAL:
            bufferSize = widthInSb * heightInSb * 9 * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::SEGMENT_ID:
            bufferSize = widthInSb * heightInSb * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::HVD_LINE:
        case HCP_INTERNAL_BUFFER_TYPE::HVD_TILE:
            bufferSize = widthInSb * MHW_CACHELINE_SIZE;
            break;
            // scalable mode specific buffers
        case HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_UP_RIGHT_COL:
        case HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_LFT_RECON_COL:
            bufferSize = intraPredMultiplier * heightInSb * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::CABAC_STREAMOUT:
            // From sas, cabac stream out buffer size =
            // (#LCU) in picture * (Worst case LCU_CU_TU_info) + 1 byte aligned per LCU + Bitstream Size * 3
            if ((chromaFormat == HCP_CHROMA_FORMAT_YUV420) && (maxBitDepth == 8))
            {
                bufferSize = widthInMinCb * heightInMinCb * MHW_HCP_WORST_CASE_CU_TU_INFO + widthInMinCb * heightInMinCb + maxFrameSize * 3;
            }
            else
            {
                bufferSize = widthInMinCb * heightInMinCb * MHW_HCP_WORST_CASE_CU_TU_INFO_REXT + widthInMinCb * heightInMinCb + maxFrameSize * 3;
            }
            bufferSize = MOS_ALIGN_CEIL(bufferSize, MHW_CACHELINE_SIZE);
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        size = bufferSize;

        return eStatus;
    }

    const HcpMmioRegisters *GetMmioRegisters(const MHW_VDBOX_NODE_IND index) const override
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }

    uint32_t GetEncCuRecordSize() override
    {
        return 8 * sizeof(uint32_t);
    }

    uint32_t GetHcpPakObjSize() override
    {
        return 8;
    }

    MOS_STATUS SetRowstoreCachingOffsets(const HcpVdboxRowStorePar &rowstoreParams) override
    {
        MHW_FUNCTION_ENTER;

        bool is8bit      = rowstoreParams.ucBitDepthMinus8 == 0;
        bool is10bit     = rowstoreParams.ucBitDepthMinus8 == 1 || rowstoreParams.ucBitDepthMinus8 == 2;
        bool is12bit     = rowstoreParams.ucBitDepthMinus8 > 2;
        bool isLcu32or64 = rowstoreParams.ucLCUSize == 32 || rowstoreParams.ucLCUSize == 64;
        bool isGt2k      = rowstoreParams.dwPicWidth > 2048;
        bool isGt4k      = rowstoreParams.dwPicWidth > 4096;
        bool isGt8k      = rowstoreParams.dwPicWidth > 8192;

        uint32_t index = 0;

        //HCP pipe for both HEVC decoder and HEVC encoder
        if (rowstoreParams.Mode == CODECHAL_DECODE_MODE_HEVCVLD || rowstoreParams.Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            constexpr bool RowStoreCacheEnableHEVC[16][5] =
            {
                { 1, 1, 1, 0, 1 }, { 1, 1, 1, 1, 1 }, { 1, 1, 0, 0, 0 }, { 1, 1, 0, 1, 0 },
                { 1, 1, 1, 1, 1 }, { 1, 1, 0, 0, 1 }, { 1, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1 },
                { 1, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1 }, { 1, 1, 1, 1, 1 }, { 1, 1, 0, 1, 1 },
                { 1, 1, 1, 1, 1 }, { 1, 0, 1, 1, 1 }, { 1, 1, 1, 1, 1 }, { 1, 0, 1, 1, 1 }
            };

            constexpr uint32_t RowStoreCacheAddrHEVC[16][5] =
                {
                { 0, 256, 1280,    0, 2048 }, { 0, 256, 1280, 1824, 1792 }, { 0, 512,    0,    0,    0 }, { 0, 256,   0, 2304,    0 },
                { 0, 256, 1024,    0, 1792 }, { 0, 512,    0,    0, 2048 }, { 0, 256, 1792,    0,    0 }, { 0,   0, 512,    0, 2048 },
                { 0, 256, 1792,    0,    0 }, { 0,   0,  256,    0, 1792 }, { 0, 256, 1024, 1568, 1536 }, { 0, 512,   0, 2112, 2048 },
                { 0, 256, 1792, 2336, 2304 }, { 0,   0,  512, 1600, 1536 }, { 0, 128, 1664, 2336, 2304 }, { 0,   0, 256, 1600, 1536 }
            };
            m_hevcDatRowStoreCache.enabled    = false;
            m_hevcDatRowStoreCache.dwAddress  = 0;
            m_hevcDfRowStoreCache.enabled     = false;
            m_hevcDfRowStoreCache.dwAddress   = 0;
            m_hevcSaoRowStoreCache.enabled    = false;
            m_hevcSaoRowStoreCache.dwAddress  = 0;
            m_hevcHSaoRowStoreCache.enabled   = false;
            m_hevcHSaoRowStoreCache.dwAddress = 0;

            if (isGt8k)
            {
                return MOS_STATUS_SUCCESS;
            }

            if ((rowstoreParams.ucChromaFormat == HCP_CHROMA_FORMAT_YUV420) ||
                (rowstoreParams.ucChromaFormat == HCP_CHROMA_FORMAT_YUV422))
            {
                index = 2 * isGt4k + isLcu32or64;
            }
            else if (rowstoreParams.ucChromaFormat == HCP_CHROMA_FORMAT_YUV444)
            {
                uint32_t subidx = is12bit ? 2 : (is10bit ? 1 : 0);
                index           = 4 + 6 * isLcu32or64 + 2 * subidx + isGt4k;
            }
            else
            {
                return MOS_STATUS_SUCCESS;
            }

            if (m_hevcDatRowStoreCache.supported)
            {
                m_hevcDatRowStoreCache.enabled   = RowStoreCacheEnableHEVC[index][0];
                m_hevcDatRowStoreCache.dwAddress = RowStoreCacheAddrHEVC[index][0];
            }

            if (m_hevcDfRowStoreCache.supported)
            {
                m_hevcDfRowStoreCache.enabled   = RowStoreCacheEnableHEVC[index][1];
                m_hevcDfRowStoreCache.dwAddress = RowStoreCacheAddrHEVC[index][1];
            }

            if (m_hevcSaoRowStoreCache.supported)
            {
                m_hevcSaoRowStoreCache.enabled   = RowStoreCacheEnableHEVC[index][2];
                m_hevcSaoRowStoreCache.dwAddress = RowStoreCacheAddrHEVC[index][2];
            }

            if (m_hevcHSaoRowStoreCache.supported)
            {
                m_hevcHSaoRowStoreCache.enabled   = RowStoreCacheEnableHEVC[index][4];
                m_hevcHSaoRowStoreCache.dwAddress = RowStoreCacheAddrHEVC[index][4];
            }
        }

        if (rowstoreParams.Mode == CODECHAL_DECODE_MODE_VP9VLD || rowstoreParams.Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            constexpr bool RowStoreCacheEnableVP9[13][4] =
            {
                { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 1, 1, 0, 1 },
                { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 0 }, { 1, 1, 0, 1 },
                { 1, 1, 1, 1 }, { 1, 1, 0, 1 }, { 1, 1, 1, 1 }, { 1, 1, 0, 1 },
                { 1, 1, 0, 1 }
            };

            constexpr uint32_t RowStoreCacheAddrVP9[13][4] =
                {
                { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,  64, 2368, }, { 0, 128,   0,  768, },
                { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,   0,    0, }, { 0, 128,   0,  768, },
                { 0,  64, 384, 2112, }, { 0, 128,   0,  768, }, { 0,  32, 192, 1920, }, { 0, 128,   0,  768, },
                { 0, 128,   0,  768, }
            };

            m_vp9HvdRowStoreCache.enabled   = false;
            m_vp9HvdRowStoreCache.dwAddress = 0;
            m_vp9DatRowStoreCache.enabled   = false;
            m_vp9DatRowStoreCache.dwAddress = 0;
            m_vp9DfRowStoreCache.enabled    = false;
            m_vp9DfRowStoreCache.dwAddress  = 0;

            if (isGt8k)
            {
                return MOS_STATUS_SUCCESS;
            }

            if ((rowstoreParams.ucChromaFormat >= HCP_CHROMA_FORMAT_YUV420) &&
                (rowstoreParams.ucChromaFormat <= HCP_CHROMA_FORMAT_YUV444))
            {
                index = 4 * (rowstoreParams.ucChromaFormat - HCP_CHROMA_FORMAT_YUV420) + 2 * (!is8bit) + isGt4k;
            }
            else
            {
                return MOS_STATUS_SUCCESS;
            }

            if (rowstoreParams.ucChromaFormat == HCP_CHROMA_FORMAT_YUV444 && !is8bit)
            {
                index += isGt2k;
            }

            if (m_vp9HvdRowStoreCache.supported)
            {
                m_vp9HvdRowStoreCache.enabled   = RowStoreCacheEnableVP9[index][0];
                m_vp9HvdRowStoreCache.dwAddress = RowStoreCacheAddrVP9[index][0];
            }

            if (m_vp9DatRowStoreCache.supported)
            {
                m_vp9DatRowStoreCache.enabled   = RowStoreCacheEnableVP9[index][1];
                m_vp9DatRowStoreCache.dwAddress = RowStoreCacheAddrVP9[index][1];
            }

            if (m_vp9DfRowStoreCache.supported)
            {
                m_vp9DfRowStoreCache.enabled   = RowStoreCacheEnableVP9[index][2];
                m_vp9DfRowStoreCache.dwAddress = RowStoreCacheAddrVP9[index][2];
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    bool IsRowStoreCachingSupported() override
    {
        return m_rowstoreCachingSupported;
    }

    uint32_t GetPakHWTileSizeRecordSize() override
    {
        return m_pakHWTileSizeRecordSize;
    }

    uint32_t GetHcpVp9PicStateCommandSize() override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    uint32_t GetHcpVp9SegmentStateCommandSize() override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

private:
    bool     m_rowstoreCachingSupported = false;
    uint32_t m_pakHWTileSizeRecordSize  = sizeof(HCPPakHWTileSizeRecord);

    virtual MOS_STATUS InitRowstoreUserFeatureSettings()
    {
        MHW_FUNCTION_ENTER;

        bool rowstoreCachingDisableDefaultValue = false;
        if (m_osItf->bSimIsActive)
        {
            // Disable RowStore Cache on simulation by default
            rowstoreCachingDisableDefaultValue = true;
        }
        else
        {
            rowstoreCachingDisableDefaultValue = false;
        }
        m_rowstoreCachingSupported = !rowstoreCachingDisableDefaultValue;
#if (_DEBUG || _RELEASE_INTERNAL)
        auto userSettingPtr = m_osItf->pfnGetUserSettingInstance(m_osItf);
        {
            MediaUserSetting::Value outValue;
            ReadUserSettingForDebug(userSettingPtr,
                outValue,
                "Disable RowStore Cache",
                MediaUserSetting::Group::Device,
                rowstoreCachingDisableDefaultValue,
                true);
            m_rowstoreCachingSupported = !(outValue.Get<bool>());
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

        if (m_rowstoreCachingSupported)
        {
            m_hevcDatRowStoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableHevcDatRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_hevcDatRowStoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_hevcDfRowStoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableHevcDfRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_hevcDfRowStoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_hevcSaoRowStoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableHevcSaoRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_hevcSaoRowStoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL
            m_hevcHSaoRowStoreCache.supported = m_hevcSaoRowStoreCache.supported;

            m_vp9HvdRowStoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVp9HvdRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_vp9HvdRowStoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_vp9DatRowStoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVp9DatRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_vp9DatRowStoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_vp9DfRowStoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVp9DfRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_vp9DfRowStoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL
        }

        return MOS_STATUS_SUCCESS;
    }

    void InitMmioRegisters()
    {
        HcpMmioRegisters *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

        mmioRegisters->watchdogCountCtrlOffset                           = WATCHDOG_COUNT_CTRL_OFFSET_INIT;
        mmioRegisters->watchdogCountThresholdOffset                      = WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT;
        mmioRegisters->hcpDebugFEStreamOutSizeRegOffset                  = HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT;
        mmioRegisters->hcpEncImageStatusMaskRegOffset                    = HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT;
        mmioRegisters->hcpEncImageStatusCtrlRegOffset                    = HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT;
        mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset            = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT;
        mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset           = HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT;
        mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset    = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT;
        mmioRegisters->hcpEncQpStatusCountRegOffset                      = HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT;
        mmioRegisters->hcpEncSliceCountRegOffset                         = HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT;
        mmioRegisters->hcpEncVdencModeTimerRegOffset                     = HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT;
        mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset         = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT;
        mmioRegisters->hcpVp9EncBitstreamBytecountFrameNoHeaderRegOffset = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT;
        mmioRegisters->hcpVp9EncImageStatusMaskRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT;
        mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT;
        mmioRegisters->csEngineIdOffset                                  = CS_ENGINE_ID_OFFSET_INIT;
        mmioRegisters->hcpDecStatusRegOffset                             = HCP_DEC_STATUS_REG_OFFSET_INIT;
        mmioRegisters->hcpCabacStatusRegOffset                           = HCP_CABAC_STATUS_REG_OFFSET_INIT;
        mmioRegisters->hcpFrameCrcRegOffset                              = HCP_FRAME_CRC_REG_OFFSET_INIT;

        m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
    }

protected:
    using base_t = Itf;

    HcpMmioRegisters m_mmioRegisters[MHW_VDBOX_NODE_MAX] = {}; //!< hcp mmio registers

    vdbox::RowStoreCache m_hevcDatRowStoreCache  = {};
    vdbox::RowStoreCache m_hevcDfRowStoreCache   = {};
    vdbox::RowStoreCache m_hevcSaoRowStoreCache  = {};
    vdbox::RowStoreCache m_hevcHSaoRowStoreCache = {};
    vdbox::RowStoreCache m_vp9HvdRowStoreCache   = {};
    vdbox::RowStoreCache m_vp9DfRowStoreCache    = {};
    vdbox::RowStoreCache m_vp9DatRowStoreCache   = {};

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};

    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;

        InitRowstoreUserFeatureSettings();
        InitMmioRegisters();
    }

    virtual ~Impl()
    {
        MHW_FUNCTION_ENTER;

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_hevcDatRowStoreCache.enabled ||
            m_hevcDfRowStoreCache.enabled ||
            m_hevcSaoRowStoreCache.enabled ||
            m_hevcHSaoRowStoreCache.enabled ||
            m_vp9HvdRowStoreCache.enabled ||
            m_vp9DatRowStoreCache.enabled ||
            m_vp9DfRowStoreCache.enabled)
        {
            // Report rowstore cache usage status to regkey
            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED,
                1,
                MediaUserSetting::Group::Device);
        }
#endif

    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_SURFACE_STATE);

#define DO_FIELDS()                                                                      \
    DO_FIELD(DW1, SurfaceId, params.surfaceStateId);                                     \
    DO_FIELD(DW1, SurfacePitchMinus1, params.surfacePitchMinus1);                        \
    DO_FIELD(DW2, SurfaceFormat, static_cast<uint32_t>(params.surfaceFormat));           \
    DO_FIELD(DW2, YOffsetForUCbInPixel, params.yOffsetForUCbInPixel);                    \
    DO_FIELD(DW3, YOffsetForVCr, params.yOffsetForVCr);                                  \
    DO_FIELD(DW3, DefaultAlphaValue, params.defaultAlphaValue);                          \
    DO_FIELD(DW4, MemoryCompressionEnable, params.refsMmcEnable & (~params.mmcSkipMask)); \
    DO_FIELD(DW4, CompressionType, params.refsMmcType);                                  \
    DO_FIELD(DW4, CompressionFormat, params.dwCompressionFormat);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_PIC_STATE);

#define DO_FIELDS()                                                                                                                                                    \
        DO_FIELD(DW1, Framewidthinmincbminus1, params.framewidthinmincbminus1);                                                                                        \
        DO_FIELD(DW1, PakTransformSkipEnable, (params.bDecodeInUse ? false : params.transformSkipEnabled));                                                            \
        DO_FIELD(DW1, Frameheightinmincbminus1, params.frameheightinmincbminus1);                                                                                      \
        DO_FIELD(DW2, Mincusize, params.mincusize);                                                                                                                    \
        DO_FIELD(DW2, CtbsizeLcusize, params.ctbsizeLcusize);                                                                                                          \
        DO_FIELD(DW2, Maxtusize, params.maxtusize);                                                                                                                    \
        DO_FIELD(DW2, Mintusize, params.mintusize);                                                                                                                    \
        DO_FIELD(DW2, ChromaSubsampling, params.chromaSubsampling);                                                                                                    \
        DO_FIELD(DW2, Minpcmsize, params.minpcmsize);                                                                                                                  \
        DO_FIELD(DW2, Maxpcmsize, params.maxpcmsize);                                                                                                                  \
        DO_FIELD(DW3, Colpicisi, 0);                                                                                                                                   \
        DO_FIELD(DW3, Curpicisi, 0);                                                                                                                                   \
        DO_FIELD(DW3, Log2Maxtransformskipsize, params.log2Maxtransformskipsize);                                                                                      \
        DO_FIELD(DW4, SampleAdaptiveOffsetEnabledFlag, params.sampleAdaptiveOffsetEnabled);                                                                            \
        DO_FIELD(DW4, PcmEnabledFlag, params.pcmEnabledFlag);                                                                                                          \
        DO_FIELD(DW4, CuQpDeltaEnabledFlag, params.cuQpDeltaEnabledFlag);                                                                                              \
        DO_FIELD(DW4, DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth, params.diffCuQpDeltaDepth);                                                                              \
        DO_FIELD(DW4, PcmLoopFilterDisableFlag, params.pcmLoopFilterDisableFlag);                                                                                      \
        DO_FIELD(DW4, ConstrainedIntraPredFlag, params.constrainedIntraPredFlag);                                                                                      \
        DO_FIELD(DW4, Log2ParallelMergeLevelMinus2, params.log2ParallelMergeLevelMinus2);                                                                              \
        DO_FIELD(DW4, SignDataHidingFlag, params.signDataHidingFlag);                                                                                                  \
        DO_FIELD(DW4, EntropyCodingSyncEnabledFlag, params.entropyCodingSyncEnabled);                                                                                  \
        DO_FIELD(DW4, WeightedPredFlag, params.weightedPredFlag);                                                                                                      \
        DO_FIELD(DW4, WeightedBipredFlag, params.weightedBipredFlag);                                                                                                  \
        DO_FIELD(DW4, Fieldpic, params.fieldpic);                                                                                                                      \
        DO_FIELD(DW4, Bottomfield, params.bottomfield);                                                                                                                \
        DO_FIELD(DW4, AmpEnabledFlag, params.ampEnabledFlag);                                                                                                          \
        DO_FIELD(DW4, TransquantBypassEnableFlag, params.transquantBypassEnableFlag);                                                                                  \
        DO_FIELD(DW4, StrongIntraSmoothingEnableFlag, params.strongIntraSmoothingEnableFlag);                                                                          \
        DO_FIELD(DW4, CuPacketStructure, 0);                                                                                                                           \
        DO_FIELD(DW4, TransformSkipEnabledFlag, params.transformSkipEnabled);                                                                                          \
        DO_FIELD(DW4, TilesEnabledFlag, params.tilesEnabledFlag);                                                                                                      \
        DO_FIELD(DW4, LoopFilterAcrossTilesEnabledFlag, params.loopFilterAcrossTilesEnabled);                                                                          \
        DO_FIELD(DW5, PicCbQpOffset, params.picCbQpOffset);                                                                                                            \
        DO_FIELD(DW5, PicCrQpOffset, params.picCrQpOffset);                                                                                                            \
        DO_FIELD(DW5, MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra, params.maxTransformHierarchyDepthIntra);                                                \
        DO_FIELD(DW5, MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter, params.maxTransformHierarchyDepthInter);                                                \
        DO_FIELD(DW5, PcmSampleBitDepthChromaMinus1, params.pcmSampleBitDepthChromaMinus1);                                                                            \
        DO_FIELD(DW5, PcmSampleBitDepthLumaMinus1, params.pcmSampleBitDepthLumaMinus1);                                                                                \
        DO_FIELD(DW5, BitDepthChromaMinus8, params.bitDepthChromaMinus8);                                                                                              \
        DO_FIELD(DW5, BitDepthLumaMinus8, params.bitDepthLumaMinus8);                                                                                                  \
        DO_FIELD(DW6, LcuMaxBitsizeAllowed, params.lcuMaxBitsizeAllowed);                                                                                              \
        DO_FIELD(DW6, Nonfirstpassflag, params.bNotFirstPass);                                                                                                                            \
        DO_FIELD(DW6, LcuMaxBitSizeAllowedMsb2its, params.lcuMaxBitSizeAllowedMsb2its);                                                                                \
        DO_FIELD(DW6, LcumaxbitstatusenLcumaxsizereportmask, 0);                                                                                                       \
        DO_FIELD(DW6, FrameszoverstatusenFramebitratemaxreportmask, 0);                                                                                                \
        DO_FIELD(DW6, FrameszunderstatusenFramebitrateminreportmask, 0);                                                                                               \
        DO_FIELD(DW6, LoadSlicePointerFlag, 0);                                                                                                                        \
        DO_FIELD(DW18, Minframesize, params.minframesize);                                                                                                             \
        DO_FIELD(DW18, Minframesizeunits, params.minframesizeunits);                                                                                                   \
        DO_FIELD(DW19, RdoqEnable, params.rdoqEnable);                                                                                                                 \
        DO_FIELD(DW19, SseEnable, params.sseEnable);                                                                                                                   \
        DO_FIELD(DW19, RhodomainRateControlEnable, params.rhodomainRateControlEnable);                                                                                 \
        DO_FIELD(DW19, Rhodomainframelevelqp, params.rhodomainframelevelqp);                                                                                           \
        DO_FIELD(DW19, FractionalQpAdjustmentEnable, params.fractionalQpAdjustmentEnable);                                                                             \
        DO_FIELD(DW19, FirstSliceSegmentInPicFlag, params.bDecodeInUse ? 0 : 1);                                                                                       \
        DO_FIELD(DW19, PakDynamicSliceModeEnable, params.pakDynamicSliceModeEnable);                                                                                   \
        DO_FIELD(DW19, SlicePicParameterSetId, params.slicePicParameterSetId);                                                                                         \
        DO_FIELD(DW19, Nalunittypeflag, params.nalunittypeflag);                                                                                                       \
        DO_FIELD(DW19, NoOutputOfPriorPicsFlag, params.noOutputOfPriorPicsFlag);                                                                                       \
        DO_FIELD(DW19, PartialFrameUpdateMode, params.partialFrameUpdateMode);                                                                                         \
        DO_FIELD(DW19, TemporalMvPredDisable, params.temporalMvPredDisable);                                                                                           \
        DO_FIELD(DW20, Intratucountbasedrdoqdisable, params.intratucountbasedrdoqdisable);                                                                             \
        DO_FIELD(DW21, SliceSizeThresholdInBytes, params.sliceSizeThresholdInBytes);                                                                                   \
        DO_FIELD(DW22, TargetSliceSizeInBytes, params.targetSliceSizeInBytes);                                                                                         \
        DO_FIELD(DW34, IntraBoundaryFilteringDisabledFlag, params.intraBoundaryFilteringDisabledFlag);                                                                 \
        DO_FIELD(DW34, MotionVectorResolutionControlIdc, params.motionVectorResolutionControlIdc);                                                                     \
        DO_FIELD(DW34, PpsCurrPicRefEnabledFlag, params.ppsCurrPicRefEnabledFlag);                                                                                     \
        DO_FIELD(DW34, IbcMotionCompensationBufferReferenceIdc, params.ibcMotionCompensationBufferReferenceIdc);                                                       \
        DO_FIELD(DW35, IbcConfiguration, params.ibcConfiguration);                                                                                                     \
        DO_FIELD(DW35, PaletteModeEnabledFlag, params.paletteModeEnabledFlag);                                                                                         \
        DO_FIELD(DW35, MonochromePaletteFlag, 0);                                                                                                                      \
        DO_FIELD(DW35, PaletteMaxSize, params.paletteMaxSize);                                                                                                         \
        DO_FIELD(DW35, DeltaPaletteMaxPredictorSize, params.deltaPaletteMaxPredictorSize);                                                                             \
        DO_FIELD(DW35, LumaBitDepthEntryMinus8, params.lumaBitDepthEntryMinus8);                                                                                       \
        DO_FIELD(DW35, ChromaBitDepthEntryMinus8, params.chromaBitDepthEntryMinus8);                                                                                   \
        DO_FIELD(DW37, Rdoqintratuthreshold, params.rdoqintratuthreshold);                                                                                             \
                                                                                                                                                                       \
        if (params.bDecodeInUse && params.pHevcExtPicParams)                                                                                                           \
        {                                                                                                                                                              \
            auto hevcExtPicParams = params.pHevcExtPicParams;                                                                                                          \
            DO_FIELD(DW2, ChromaQpOffsetListEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag);                          \
            DO_FIELD(DW2, DiffCuChromaQpOffsetDepth, hevcExtPicParams->diff_cu_chroma_qp_offset_depth);                                                                \
            DO_FIELD(DW2, ChromaQpOffsetListLenMinus1, hevcExtPicParams->chroma_qp_offset_list_len_minus1);                                                            \
            DO_FIELD(DW2, Log2SaoOffsetScaleLuma, hevcExtPicParams->log2_sao_offset_scale_luma);                                                                       \
            DO_FIELD(DW2, Log2SaoOffsetScaleChroma, hevcExtPicParams->log2_sao_offset_scale_chroma);                                                                   \
            DO_FIELD(DW3, CrossComponentPredictionEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag);               \
            DO_FIELD(DW3, CabacBypassAlignmentEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag);                       \
            DO_FIELD(DW3, PersistentRiceAdaptationEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.persistent_rice_adaptation_enabled_flag);               \
            DO_FIELD(DW3, IntraSmoothingDisabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.intra_smoothing_disabled_flag);                                  \
            DO_FIELD(DW3, ExplicitRdpcmEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.explicit_rdpcm_enabled_flag);                                      \
            DO_FIELD(DW3, ImplicitRdpcmEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.implicit_rdpcm_enabled_flag);                                      \
            DO_FIELD(DW3, TransformSkipContextEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.transform_skip_context_enabled_flag);                       \
            DO_FIELD(DW3, TransformSkipRotationEnabledFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.transform_skip_rotation_enabled_flag);                     \
            DO_FIELD(DW3, HighPrecisionOffsetsEnableFlag, hevcExtPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag);                        \
            DO_FIELD(DW32, CbQpOffsetList0, hevcExtPicParams->cb_qp_offset_list[0]);                                                                                   \
            DO_FIELD(DW32, CbQpOffsetList1, hevcExtPicParams->cb_qp_offset_list[1]);                                                                                   \
            DO_FIELD(DW32, CbQpOffsetList2, hevcExtPicParams->cb_qp_offset_list[2]);                                                                                   \
            DO_FIELD(DW32, CbQpOffsetList3, hevcExtPicParams->cb_qp_offset_list[3]);                                                                                   \
            DO_FIELD(DW32, CbQpOffsetList4, hevcExtPicParams->cb_qp_offset_list[4]);                                                                                   \
            DO_FIELD(DW32, CbQpOffsetList5, hevcExtPicParams->cb_qp_offset_list[5]);                                                                                   \
            DO_FIELD(DW33, CrQpOffsetList0, hevcExtPicParams->cr_qp_offset_list[0]);                                                                                   \
            DO_FIELD(DW33, CrQpOffsetList1, hevcExtPicParams->cr_qp_offset_list[1]);                                                                                   \
            DO_FIELD(DW33, CrQpOffsetList2, hevcExtPicParams->cr_qp_offset_list[2]);                                                                                   \
            DO_FIELD(DW33, CrQpOffsetList3, hevcExtPicParams->cr_qp_offset_list[3]);                                                                                   \
            DO_FIELD(DW33, CrQpOffsetList4, hevcExtPicParams->cr_qp_offset_list[4]);                                                                                   \
            DO_FIELD(DW33, CrQpOffsetList5, hevcExtPicParams->cr_qp_offset_list[5]);                                                                                   \
        }                                                                                                                                                              \
                                                                                                                                                                       \
        if (params.bDecodeInUse && params.pHevcSccPicParams)                                                                                                           \
        {                                                                                                                                                              \
            auto hevcSccPicParams = params.pHevcSccPicParams;                                                                                                          \
            DO_FIELD(DW34, PpsActCrQpOffsetPlus3, hevcSccPicParams->pps_act_cr_qp_offset_plus3);                                                                       \
            DO_FIELD(DW34, PpsActCbQpOffsetPlus5, hevcSccPicParams->pps_act_cb_qp_offset_plus5);                                                                       \
            DO_FIELD(DW34, PpsActYOffsetPlus5, hevcSccPicParams->pps_act_y_qp_offset_plus5);                                                                           \
            DO_FIELD(DW34, PpsSliceActQpOffsetsPresentFlag, hevcSccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag);                      \
            DO_FIELD(DW34, ResidualAdaptiveColourTransformEnabledFlag, hevcSccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag); \
            DO_FIELD(DW34, DeblockingFilterOverrideEnabledFlag, params.deblockingFilterOverrideEnabled);                                                               \
            DO_FIELD(DW34, PpsDeblockingFilterDisabledFlag, params.ppsDeblockingFilterDisabled);                                                                       \
            DO_FIELD(DW35, IbcMotionVectorErrorHandlingDisable, 0);                                                                                                    \
        }                                                                                                                                                              \
                                                                                                                                                                       \
        if (params.bDecodeInUse && params.requestCRC)                                                                                                                  \
        {                                                                                                                                                              \
            DO_FIELD(DW36, FrameCrcEnable, 1);                                                                                                                         \
            DO_FIELD(DW36, FrameCrcType, 0);                                                                                                                           \
        }

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_SLICE_STATE);

#define DO_FIELDS()                                                                                                     \
    DO_FIELD(DW1, SlicestartctbxOrSliceStartLcuXEncoder, params.slicestartctbxOrSliceStartLcuXEncoder);                 \
    DO_FIELD(DW1, SlicestartctbyOrSliceStartLcuYEncoder, params.slicestartctbyOrSliceStartLcuYEncoder);                 \
    DO_FIELD(DW2, NextslicestartctbxOrNextSliceStartLcuXEncoder, params.nextslicestartctbxOrNextSliceStartLcuXEncoder); \
    DO_FIELD(DW2, NextslicestartctbyOrNextSliceStartLcuYEncoder, params.nextslicestartctbyOrNextSliceStartLcuYEncoder); \
    DO_FIELD(DW3, SliceType, params.sliceType);                                                                         \
    DO_FIELD(DW3, Lastsliceofpic, params.lastsliceofpic);                                                               \
    DO_FIELD(DW3, SliceqpSignFlag, params.sliceqpSignFlag);                                                             \
    DO_FIELD(DW3, DependentSliceFlag, params.dependentSliceFlag);                                                       \
    DO_FIELD(DW3, SliceTemporalMvpEnableFlag, params.sliceTemporalMvpEnableFlag);                                       \
    DO_FIELD(DW3, Sliceqp, params.sliceqp);                                                                             \
    DO_FIELD(DW3, SliceCbQpOffset, params.sliceCbQpOffset);                                                             \
    DO_FIELD(DW3, SliceCrQpOffset, params.sliceCrQpOffset);                                                             \
    DO_FIELD(DW3, Intrareffetchdisable, params.intrareffetchdisable);                                                   \
    DO_FIELD(DW3, Lastsliceoftile, params.lastSliceInTile);                                                             \
    DO_FIELD(DW3, Lastsliceoftilecolumn, params.lastSliceInTileColumn);                                                 \
    DO_FIELD(DW3, CuChromaQpOffsetEnabledFlag, params.cuChromaQpOffsetEnable);                                          \
    DO_FIELD(DW4, SliceHeaderDisableDeblockingFilterFlag, params.deblockingFilterDisable);                              \
    DO_FIELD(DW4, SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder, params.tcOffsetDiv2);                                    \
    DO_FIELD(DW4, SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder, params.betaOffsetDiv2);                              \
    DO_FIELD(DW4, SliceLoopFilterAcrossSlicesEnabledFlag, params.loopFilterAcrossSlicesEnabled);                        \
    DO_FIELD(DW4, SliceSaoChromaFlag, params.saoChromaFlag);                                                            \
    DO_FIELD(DW4, SliceSaoLumaFlag, params.saoLumaFlag);                                                                \
    DO_FIELD(DW4, MvdL1ZeroFlag, params.mvdL1ZeroFlag);                                                                 \
    DO_FIELD(DW4, Islowdelay, params.isLowDelay);                                                                       \
    DO_FIELD(DW4, CollocatedFromL0Flag, params.collocatedFromL0Flag);                                                   \
    DO_FIELD(DW4, Chromalog2Weightdenom, params.chromalog2Weightdenom);                                                 \
    DO_FIELD(DW4, LumaLog2WeightDenom, params.lumaLog2WeightDenom);                                                     \
    DO_FIELD(DW4, CabacInitFlag, params.cabacInitFlag);                                                                 \
    DO_FIELD(DW4, Maxmergeidx, params.maxmergeidx);                                                                     \
    DO_FIELD(DW4, Collocatedrefidx, params.collocatedrefidx);                                                           \
    DO_FIELD(DW5, Sliceheaderlength, params.sliceheaderlength);                                                         \
    DO_FIELD(DW6, Roundinter, params.roundinter);                                                                       \
    DO_FIELD(DW6, Roundintra, params.roundintra);                                                                       \
    DO_FIELD(DW7, Cabaczerowordinsertionenable, params.cabaczerowordinsertionenable);                                   \
    DO_FIELD(DW7, Emulationbytesliceinsertenable, params.emulationbytesliceinsertenable);                               \
    DO_FIELD(DW7, TailInsertionEnable, params.tailInsertionEnable);                                                     \
    DO_FIELD(DW7, SlicedataEnable, params.slicedataEnable);                                                             \
    DO_FIELD(DW7, HeaderInsertionEnable, params.headerInsertionEnable);                                                 \
    DO_FIELD(DW7, DependentSliceDueToTileSplit, params.bIsNotFirstTile);                                                \
    DO_FIELD(DW8, IndirectPakBseDataStartOffsetWrite, params.indirectPakBseDataStartOffsetWrite);                       \
    DO_FIELD(DW9, TransformskipLambda, params.transformskiplambda);                                                     \
    DO_FIELD(DW10, TransformskipNumzerocoeffsFactor0, params.transformskipNumzerocoeffsFactor0);                        \
    DO_FIELD(DW10, TransformskipNumnonzerocoeffsFactor0, params.transformskipNumnonzerocoeffsFactor0);                  \
    DO_FIELD(DW10, TransformskipNumzerocoeffsFactor1, params.transformskipNumzerocoeffsFactor1);                        \
    DO_FIELD(DW10, TransformskipNumnonzerocoeffsFactor1, params.transformskipNumnonzerocoeffsFactor1);                  \
    DO_FIELD(DW11, Originalslicestartctbx, params.originalSliceStartCtbX);                                              \
    DO_FIELD(DW11, Originalslicestartctby, params.originalSliceStartCtbY);                                              \
    DO_FIELD(DW12, SliceActYQpOffset, params.sliceActYQpOffset);                                                        \
    DO_FIELD(DW12, SliceActCbQpOffset, params.sliceActCbQpOffset);                                                      \
    DO_FIELD(DW12, SliceActCrQpOffset, params.sliceActCrQpOffset);                                                      \
    DO_FIELD(DW12, UseIntegerMvFlag, params.useIntegerMvFlag);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_IND_OBJ_BASE_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_IND_OBJ_BASE_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        if (params.bDecodeInUse)
        {
            MHW_MI_CHK_NULL(params.presDataBuffer);

            InitMocsParams(resourceParams, &cmd.HcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value, 1, 6);

            resourceParams.presResource    = params.presDataBuffer;
            resourceParams.dwOffset        = params.dwDataOffset;
            resourceParams.pdwCmd          = (cmd.HcpIndirectBitstreamObjectBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.dwSize          = params.dwDataSize;
            resourceParams.bIsWritable     = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
        }

        if (!params.bDecodeInUse)
        {
            if (params.presMvObjectBuffer)
            {
                resourceParams.presResource                      = params.presMvObjectBuffer;
                resourceParams.dwOffset                          = params.dwMvObjectOffset;
                resourceParams.pdwCmd                            = (cmd.DW6_7.Value);
                resourceParams.dwLocationInCmd                   = _MHW_CMD_DW_LOCATION(DW6_7);
                resourceParams.dwSize                            = MOS_ALIGN_CEIL(params.dwMvObjectSize, 0x1000);
                resourceParams.bIsWritable                       = false;
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

                InitMocsParams(resourceParams, &cmd.HcpIndirectCuObjectObjectMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (params.presPakBaseObjectBuffer)
            {
                resourceParams.presResource                      = params.presPakBaseObjectBuffer;
                resourceParams.dwOffset                          = params.presPakBaseObjectBuffer->dwResourceOffset;
                resourceParams.pdwCmd                            = (cmd.DW9_10.Value);
                resourceParams.dwLocationInCmd                   = _MHW_CMD_DW_LOCATION(DW9_10);
                resourceParams.dwSize                            = MOS_ALIGN_FLOOR(params.dwPakBaseObjectSize, 0x1000);
                resourceParams.bIsWritable                       = true;
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

                InitMocsParams(resourceParams, &cmd.HcpPakBseObjectAddressMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));

                resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
            }

            if (params.presCompressedHeaderBuffer)
            {
                resourceParams.presResource    = params.presCompressedHeaderBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (cmd.DW14_15.Value);
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW14_15);
                resourceParams.dwSize          = params.dwCompressedHeaderSize;
                resourceParams.bIsWritable     = false;

                InitMocsParams(resourceParams, &cmd.HcpVp9PakCompressedHeaderSyntaxStreaminMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (params.presProbabilityCounterBuffer)
            {
                resourceParams.presResource    = params.presProbabilityCounterBuffer;
                resourceParams.dwOffset        = params.dwProbabilityCounterOffset;
                resourceParams.pdwCmd          = (cmd.DW17_18.Value);
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW17_18);
                resourceParams.dwSize          = params.dwProbabilityCounterSize;
                resourceParams.bIsWritable     = true;

                InitMocsParams(resourceParams, &cmd.HcpVp9PakProbabilityCounterStreamoutMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (params.presProbabilityDeltaBuffer)
            {
                resourceParams.presResource    = params.presProbabilityDeltaBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (cmd.DW20_21.Value);
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW20_21);
                resourceParams.dwSize          = params.dwProbabilityDeltaSize;
                resourceParams.bIsWritable     = false;

                InitMocsParams(resourceParams, &cmd.HcpVp9PakProbabilityDeltasStreaminMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (params.presTileRecordBuffer)
            {
                resourceParams.presResource    = params.presTileRecordBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (cmd.DW23_24.Value);
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW23_24);
                resourceParams.dwSize          = params.dwTileRecordSize;
                resourceParams.bIsWritable     = true;

                InitMocsParams(resourceParams, &cmd.HcpVp9PakTileRecordStreamoutMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
            else if (params.presPakTileSizeStasBuffer)
            {
                resourceParams.presResource    = params.presPakTileSizeStasBuffer;
                resourceParams.dwOffset        = params.dwPakTileSizeRecordOffset;
                resourceParams.pdwCmd          = (cmd.DW23_24.Value);
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW23_24);
                resourceParams.dwSize          = params.dwPakTileSizeStasBufferSize;
                resourceParams.bIsWritable     = WRITE_WA;

                InitMocsParams(resourceParams, &cmd.HcpVp9PakTileRecordStreamoutMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_QM_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_QM_STATE);

        for (uint8_t i = 0; i < 16; i++)
        {
            cmd.Quantizermatrix[i] = params.quantizermatrix[i];
        }

        cmd.DW1.PredictionType = params.predictionType;
        cmd.DW1.Sizeid         = params.sizeid;
        cmd.DW1.ColorComponent = params.colorComponent;
        cmd.DW1.DcCoefficient  = params.dcCoefficient;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(HCP_BSD_OBJECT);

        #define DO_FIELDS()                                             \
            DO_FIELD(DW1, IndirectBsdDataLength, params.bsdDataLength); \
            DO_FIELD(DW2, IndirectDataStartAddress, params.bsdDataStartOffset);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_TILE_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_TILE_STATE);

        cmd.DW1.Numtilecolumnsminus1 = params.numTileColumnsMinus1;
        cmd.DW1.Numtilerowsminus1    = params.numTileRowsMinus1;

        uint32_t column       = params.numTileColumnsMinus1 + 1;
        uint32_t lastDwEleNum = column % 4;
        uint32_t count        = column / 4;

        for (uint8_t i = 0; i < 5; i++)
        {
            cmd.CtbColumnPositionOfTileColumn[i].DW0.Value = 0;
        }

        for (uint8_t i = 0; i < 6; i++)
        {
            cmd.CtbRowPositionOfTileRow[i].DW0.Value = 0;
        }

        cmd.CtbColumnPositionMsb.DW0.Value = 0;
        cmd.CtbColumnPositionMsb.DW1.Value = 0;
        cmd.CtbRowPositionMsb.DW0.Value    = 0;
        cmd.CtbRowPositionMsb.DW1.Value    = 0;

        uint32_t colCumulativeValue = 0;
        uint32_t rowCumulativeValue = 0;

        // Column Position
        for (uint32_t i = 0; i < count; i++)
        {
            uint32_t &CtbColumnMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbColumnPositionMsb.DW0.Value : cmd.CtbColumnPositionMsb.DW1.Value;

            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos0I = colCumulativeValue & 0xFF; // lower 8bits
            CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 0)); // MSB 2bits
            colCumulativeValue                               += params.pTileColWidth[4 * i];

            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos1I = colCumulativeValue & 0xFF; // lower 8bits
            CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 2)); // MSB 2bits
            colCumulativeValue                               += params.pTileColWidth[4 * i + 1];

            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos2I = colCumulativeValue & 0xFF; // lower 8bits
            CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 4)); // MSB 2bits
            colCumulativeValue                               += params.pTileColWidth[4 * i + 2];

            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos3I = colCumulativeValue & 0xFF; // lower 8bits
            CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 6)); // MSB 2bits
            colCumulativeValue                               += params.pTileColWidth[4 * i + 3];
        }

        if (lastDwEleNum)
        {
            uint32_t  i                 = count;
            uint32_t &CtbColumnMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbColumnPositionMsb.DW0.Value : cmd.CtbColumnPositionMsb.DW1.Value;

            if (i < 5)
            {
                cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos0I = colCumulativeValue & 0xFF; // lower 8bits
                CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 0)); // MSB 2bits

                if (lastDwEleNum > 1)
                {
                    colCumulativeValue                               += params.pTileColWidth[4 * i];
                    cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos1I = colCumulativeValue & 0xFF;  //lower 8bits
                    CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 2)); // MSB 2bits

                    if (lastDwEleNum > 2)
                    {
                        colCumulativeValue                               += params.pTileColWidth[4 * i + 1];
                        cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos2I = colCumulativeValue & 0xFF;  //lower 8bits
                        CtbColumnMsbValue                                 = CtbColumnMsbValue | (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 4)); // MSB 2bits
                    }
                }
            }
        }

        // Row Postion
        uint32_t row = params.numTileRowsMinus1 + 1;
        lastDwEleNum = row % 4;
        count        = row / 4;

        for (uint32_t i = 0; i < count; i++)
        {
            uint32_t &CtbRowMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbRowPositionMsb.DW0.Value : cmd.CtbRowPositionMsb.DW1.Value;

            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos0I = rowCumulativeValue & 0xFF; // lower 8bits
            CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 0)); // MSB 2bits
            rowCumulativeValue                         += params.pTileRowHeight[4 * i];

            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos1I = rowCumulativeValue & 0xFF; // lower 8bits
            CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 2)); // MSB 2bits
            rowCumulativeValue                         += params.pTileRowHeight[4 * i + 1];

            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos2I = rowCumulativeValue & 0xFF; // lower 8bits
            CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 4)); // MSB 2bits
            rowCumulativeValue                         += params.pTileRowHeight[4 * i + 2];

            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos3I = rowCumulativeValue & 0xFF; // lower 8bits
            CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 6)); // MSB 2bits
            rowCumulativeValue                         += params.pTileRowHeight[4 * i + 3];
        }

        if (lastDwEleNum)
        {
            uint32_t i = count;
            uint32_t &CtbRowMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbRowPositionMsb.DW0.Value : cmd.CtbRowPositionMsb.DW1.Value;

            if (i < 6)
            {
                cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos0I = rowCumulativeValue & 0xFF; // lower 8bits
                CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 0)); // MSB 2bits

                if (lastDwEleNum > 1)
                {
                    rowCumulativeValue                         += params.pTileRowHeight[4 * i];
                    cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos1I = rowCumulativeValue & 0xFF; // lower 8bits
                    CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 2)); // MSB 2bits

                    if (lastDwEleNum > 2)
                    {
                        rowCumulativeValue                         += params.pTileRowHeight[4 * i + 1];
                        cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos2I = rowCumulativeValue & 0xFF; // lower 8bits
                        CtbRowMsbValue                              = CtbRowMsbValue | (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 4)); // MSB 2bits
                    }
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_REF_IDX_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_REF_IDX_STATE);

        cmd.DW1.Refpiclistnum                       = params.ucList;
        cmd.DW1.NumRefIdxLRefpiclistnumActiveMinus1 = params.numRefIdxLRefpiclistnumActiveMinus1;

        for (uint8_t i = 0; i < sizeof(params.fieldPicFlag) / sizeof(params.fieldPicFlag[0]); i++)
        {
            cmd.Entries[i].DW0.ListEntryLxReferencePictureFrameIdRefaddr07 = params.listEntryLxReferencePictureFrameIdRefaddr07[i];
            cmd.Entries[i].DW0.ReferencePictureTbValue                     = params.referencePictureTbValue[i];
            cmd.Entries[i].DW0.Longtermreference                           = params.longtermreference[i];
            cmd.Entries[i].DW0.FieldPicFlag                                = params.fieldPicFlag[i];
            cmd.Entries[i].DW0.BottomFieldFlag                             = params.bottomFieldFlag[i];
        }

        if (params.bDecodeInUse && (!params.bDummyReference))
        {
            for (uint8_t i = (uint8_t)params.ucNumRefForList; i < 16; i++)
            {
                cmd.Entries[i].DW0.Value = 0x00;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_WEIGHTOFFSET_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_WEIGHTOFFSET_STATE);

        uint8_t i      = 0;
        uint8_t refIdx = 0;

        cmd.DW1.Refpiclistnum = i = params.ucList;

        // Luma
        for (refIdx = 0; refIdx < MAX_REF_FRAME_NUM; refIdx++)
        {
            cmd.Lumaoffsets[refIdx].DW0.DeltaLumaWeightLxI  = params.LumaWeights[i][refIdx];
            cmd.Lumaoffsets[refIdx].DW0.LumaOffsetLxI       = (char)(params.LumaOffsets[i][refIdx] & 0xFF); // lower 8bits
            cmd.Lumaoffsets[refIdx].DW0.LumaOffsetLxIMsbyte = (char)((params.LumaOffsets[i][refIdx] >> 8) & 0xFF); // MSB 8bits
        }

        // Chroma
        for (refIdx = 0; refIdx < MAX_REF_FRAME_NUM; refIdx++)
        {
            // Cb
            cmd.Chromaoffsets[refIdx].DW0.DeltaChromaWeightLxI0 = params.ChromaWeights[i][refIdx][0];
            cmd.Chromaoffsets[refIdx].DW0.ChromaoffsetlxI0      = params.ChromaOffsets[i][refIdx][0] & 0xFF; // lower 8bits

            // Cr
            cmd.Chromaoffsets[refIdx].DW0.DeltaChromaWeightLxI1 = params.ChromaWeights[i][refIdx][1];
            cmd.Chromaoffsets[refIdx].DW0.ChromaoffsetlxI1      = params.ChromaOffsets[i][refIdx][1] & 0xFF; // lower 8bits
        }

        for (refIdx = 0; refIdx < MAX_REF_FRAME_NUM - 1; refIdx += 2) // MSB 8bits
        {
            cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI0Msbyte  = (params.ChromaOffsets[i][refIdx][0] >> 8) & 0xFF;
            cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI10Msbyte = (params.ChromaOffsets[i][refIdx + 1][0] >> 8) & 0xFF;
            cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI1Msbyte  = (params.ChromaOffsets[i][refIdx][1] >> 8) & 0xFF;
            cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI11Msbyte = (params.ChromaOffsets[i][refIdx + 1][1] >> 8) & 0xFF;
        }

        // last one
        cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI0Msbyte = (params.ChromaOffsets[i][refIdx][0] >> 8) & 0xFF;
        cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI1Msbyte = (params.ChromaOffsets[i][refIdx][1] >> 8) & 0xFF;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(HCP_PIPE_MODE_SELECT);

        if (params.setProtectionSettings)
        {
            MHW_CHK_STATUS_RETURN(params.setProtectionSettings(reinterpret_cast<uint32_t *>(&cmd)));
        }

#define DO_FIELDS()                                                                                                             \
    DO_FIELD(DW1, AdvancedRateControlEnable, params.bAdvancedRateControlEnable);                                                \
    DO_FIELD(DW1, CodecStandardSelect, params.codecStandardSelect);                                                             \
    DO_FIELD(DW1, PakPipelineStreamoutEnable, params.bStreamOutEnabled || params.bBRCEnabled || params.pakPiplnStrmoutEnabled); \
    DO_FIELD(DW1, DeblockerStreamoutEnable, params.bDeblockerStreamOutEnable);                                                  \
    DO_FIELD(DW1, VdencMode, params.bVdencEnabled);                                                                             \
    DO_FIELD(DW1, RdoqEnabledFlag, params.bRdoqEnable);                                                                         \
    DO_FIELD(DW1, PakFrameLevelStreamoutEnable, params.bStreamOutEnabled || params.pakFrmLvlStrmoutEnable);                     \
    DO_FIELD(DW1, PipeWorkingMode, params.pipeWorkMode);                                                                        \
    DO_FIELD(DW1, MultiEngineMode, params.multiEngineMode);                                                                     \
    DO_FIELD(DW1, TileBasedEngine, params.bTileBasedReplayMode);                                                                \
    DO_FIELD(DW1, Vp9DynamicScalingEnable, params.bDynamicScalingEnabled);                                                      \
    DO_FIELD(DW1, CodecSelect, params.codecSelect);                                                                             \
    DO_FIELD(DW1, PrefetchDisable, params.prefetchDisable);                                                                     \
    DO_FIELD(DW2, MediaSoftResetCounterPer1000Clocks, params.mediaSoftResetCounterPer1000Clocks);                               \
    DO_FIELD(DW6, PhaseIndicator, params.ucPhaseIndicator);                                                                     \
    DO_FIELD(DW6, HevcSeparateTileProgramming, params.bHEVCSeparateTileProgramming);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_SURFACE         details;

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));

        // 1. MHW_VDBOX_HCP_GENERAL_STATE_SHIFT(6) may not work with DecodedPicture
        // since it needs to be 4k aligned
        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        // Decoded Picture
        cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.PreDeblockSurfMmcState) ? 1 : 0;
        cmd.DecodedPictureMemoryAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.PreDeblockSurfMmcState) ? 1 : 0;
        //cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode       = Mhw_ConvertToTRMode(params->psPreDeblockSurface->TileType);

        cmd.DecodedPictureMemoryAddressAttributes.DW0.TileMode = GetHwTileType(params.psPreDeblockSurface->TileType, params.psPreDeblockSurface->TileModeGMM, params.psPreDeblockSurface->bGMMTileEnabled);

        resourceParams.presResource    = &(params.psPreDeblockSurface->OsResource);
        resourceParams.dwOffset        = params.psPreDeblockSurface->dwOffset;
        resourceParams.pdwCmd          = (cmd.DecodedPicture.DW0_1.Value);
        resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DecodedPicture);
        resourceParams.bIsWritable     = true;

        InitMocsParams(resourceParams, &cmd.DecodedPictureMemoryAddressAttributes.DW0.Value, 1, 6);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        // Deblocking Filter Line Buffer
        if (m_hevcDfRowStoreCache.enabled)
        {
            cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockingFilterLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.DeblockingFilterLineBuffer.DW0_1.BaseAddress                                                      = m_hevcDfRowStoreCache.dwAddress;
        }
        else if (m_vp9DfRowStoreCache.enabled)
        {
            cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockingFilterLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.DeblockingFilterLineBuffer.DW0_1.BaseAddress                                                      = m_vp9DfRowStoreCache.dwAddress;
        }
        else if (params.presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
        {
            resourceParams.presResource    = params.presMfdDeblockingFilterRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockingFilterLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockingFilterLineBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocking Filter Tile Line Buffer
        if (params.presDeblockingFilterTileRowStoreScratchBuffer != nullptr)
        {
            resourceParams.presResource    = params.presDeblockingFilterTileRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockingFilterTileLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockingFilterTileLineBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocking Filter Tile Column Buffer
        if (params.presDeblockingFilterColumnRowStoreScratchBuffer != nullptr)
        {
            resourceParams.presResource    = params.presDeblockingFilterColumnRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockingFilterTileColumnBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockingFilterTileColumnBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Metadata Line Buffer
        if (m_hevcDatRowStoreCache.enabled)
        {
            cmd.MetadataLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.MetadataLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.MetadataLineBuffer.DW0_1.BaseAddress                                                      = m_hevcDatRowStoreCache.dwAddress;
        }
        else if (m_vp9DatRowStoreCache.enabled)
        {
            cmd.MetadataLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.MetadataLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.MetadataLineBuffer.DW0_1.BaseAddress                                                      = m_vp9DatRowStoreCache.dwAddress;
        }
        else if (params.presMetadataLineBuffer != nullptr)
        {
            resourceParams.presResource    = params.presMetadataLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.MetadataLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(MetadataLineBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.MetadataLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Metadata Tile Line Buffer
        if (params.presMetadataTileLineBuffer != nullptr)
        {
            resourceParams.presResource    = params.presMetadataTileLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.MetadataTileLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(MetadataTileLineBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.MetadataTileLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Metadata Tile Column Buffer
        if (params.presMetadataTileColumnBuffer != nullptr)
        {
            resourceParams.presResource    = params.presMetadataTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.MetadataTileColumnBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(MetadataTileColumnBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SAO Line Buffer
        if (m_hevcSaoRowStoreCache.enabled)
        {
            cmd.SaoLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SaoLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.SaoLineBuffer.DW0_1.BaseAddress                                                      = m_hevcSaoRowStoreCache.dwAddress;
        }
        else if (params.presSaoLineBuffer != nullptr)
        {
            resourceParams.presResource    = params.presSaoLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SaoLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SaoLineBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SaoLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SAO Tile Line Buffer
        if (params.presSaoTileLineBuffer != nullptr)
        {
            resourceParams.presResource    = params.presSaoTileLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SaoTileLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SaoTileLineBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SaoTileLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SAO Tile Column Buffer
        if (params.presSaoTileColumnBuffer != nullptr)
        {
            resourceParams.presResource    = params.presSaoTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SaoTileColumnBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SaoTileColumnBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SaoTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Current Motion Vector Temporal Buffer
        if (params.presCurMvTempBuffer != nullptr)
        {
            resourceParams.presResource    = params.presCurMvTempBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CurrentMotionVectorTemporalBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CurrentMotionVectorTemporalBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        bool              firstRefPic = true;
        MOS_MEMCOMP_STATE mmcMode     = MOS_MEMCOMP_DISABLED;

        // NOTE: for both HEVC and VP9, set all the 8 ref pic addresses in HCP_PIPE_BUF_ADDR_STATE command to valid addresses for error concealment purpose
        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            // Reference Picture Buffer
            if (params.presReferences[i] != nullptr)
            {
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.presReferences[i], &details));

                if (firstRefPic)
                {
                    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.TileMode = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
                    firstRefPic                                                         = false;
                }

                resourceParams.presResource    = params.presReferences[i];
                resourceParams.pdwCmd          = (cmd.ReferencePictureBaseAddressRefaddr07[i].DW0_1.Value);
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ReferencePictureBaseAddressRefaddr07[i]);
                if (params.IBCRefIdxMask & (1 << i))
                {
                    // Motion Compensation will use this picture to write and read IBC data
                    resourceParams.bIsWritable = true;
                }
                else
                {
                    resourceParams.bIsWritable = false;
                }

                resourceParams.dwSharedMocsOffset = 53 - resourceParams.dwLocationInCmd;  // Common Prodected Data bit is in DW53

                InitMocsParams(resourceParams, &cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        // Same MMC status for deblock and ref surfaces
        cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable;
        cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.CompressionType                    = cmd.DecodedPictureMemoryAddressAttributes.DW0.CompressionType;

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // Original Uncompressed Picture Source, Encoder only
        if (params.psRawSurface != nullptr)
        {
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.RawSurfMmcState) ? 1 : 0;
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.RawSurfMmcState) ? 1 : 0;

            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.TileMode = GetHwTileType(params.psRawSurface->TileType, params.psRawSurface->TileModeGMM, params.psRawSurface->bGMMTileEnabled);

            resourceParams.presResource    = &params.psRawSurface->OsResource;
            resourceParams.dwOffset        = params.psRawSurface->dwOffset;
            resourceParams.pdwCmd          = (cmd.OriginalUncompressedPictureSource.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(OriginalUncompressedPictureSource);
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // StreamOut Data Destination, Decoder only
        if (params.presStreamOutBuffer != nullptr)
        {
            resourceParams.presResource    = params.presStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.StreamoutDataDestination.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(StreamoutDataDestination);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Pak Cu Level Streamout Data
        if (params.presPakCuLevelStreamoutBuffer != nullptr)
        {
            resourceParams.presResource    = params.presPakCuLevelStreamoutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.StreamoutDataDestination.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(StreamoutDataDestination);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Decoded Picture Status / Error Buffer Base Address
        if (params.presLcuBaseAddressBuffer != nullptr)
        {
            resourceParams.presResource    = params.presLcuBaseAddressBuffer;
            resourceParams.dwOffset        = params.dwLcuStreamOutOffset;
            resourceParams.pdwCmd          = (cmd.DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // LCU ILDB StreamOut Buffer
        if (params.presLcuILDBStreamOutBuffer != nullptr)
        {
            resourceParams.presResource    = params.presLcuILDBStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LcuIldbStreamoutBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LcuIldbStreamoutBuffer);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LcuIldbStreamoutBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            // Collocated Motion vector Temporal Buffer
            if (params.presColMvTempBuffer[i] != nullptr)
            {
                resourceParams.presResource    = params.presColMvTempBuffer[i];
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (cmd.CollocatedMotionVectorTemporalBuffer07[i].DW0_1.Value);
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CollocatedMotionVectorTemporalBuffer07[i]);
                resourceParams.bIsWritable     = false;

                resourceParams.dwSharedMocsOffset = 82 - resourceParams.dwLocationInCmd;  // Common Prodected Data bit is in DW82

                InitMocsParams(resourceParams, &cmd.CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // VP9 Probability Buffer
        if (params.presVp9ProbBuffer != nullptr)
        {
            resourceParams.presResource    = params.presVp9ProbBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.Vp9ProbabilityBufferReadWrite.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Vp9ProbabilityBufferReadWrite);
            resourceParams.bIsWritable     = true;

            resourceParams.dwSharedMocsOffset = 85 - resourceParams.dwLocationInCmd;  // Common Prodected Data bit is in DW88

            InitMocsParams(resourceParams, &cmd.Vp9ProbabilityBufferReadWriteMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // VP9 Segment Id Buffer
        if (params.presVp9SegmentIdBuffer != nullptr)
        {
            resourceParams.presResource    = params.presVp9SegmentIdBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DW86_87.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW86_87);
            resourceParams.bIsWritable     = true;

            resourceParams.dwSharedMocsOffset = 88 - resourceParams.dwLocationInCmd;  // Common Prodected Data bit is in DW88

            InitMocsParams(resourceParams, &cmd.Vp9SegmentIdBufferReadWriteMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // HVD Line Row Store Buffer
        if (m_vp9HvdRowStoreCache.enabled)
        {
            cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.BaseAddress                                                      = m_vp9HvdRowStoreCache.dwAddress;
        }
        else if (params.presHvdLineRowStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presHvdLineRowStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Vp9HvdLineRowstoreBufferReadWrite);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // HVC Tile Row Store Buffer
        if (params.presHvdTileRowStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presHvdTileRowStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.Vp9HvdTileRowstoreBufferReadWrite.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Vp9HvdTileRowstoreBufferReadWrite);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // HEVC SAO row store buffer, HSAO
        if (m_hevcHSaoRowStoreCache.enabled)
        {
            cmd.SaoRowstoreBufferReadWriteMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SaoRowstoreBufferReadWriteMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.DW95_96.SaoRowstoreBufferBaseAddress                                                              = (uint64_t)(m_hevcHSaoRowStoreCache.dwAddress) << 6;
        }
        else if (params.presSaoRowStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presSaoRowStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DW95_96.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW95_96);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SaoRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Frame Statistics Streamout Data Destination Buffer
        if (params.presFrameStatStreamOutBuffer != nullptr)
        {
            resourceParams.presResource = params.presFrameStatStreamOutBuffer;
            resourceParams.dwOffset     = params.dwFrameStatStreamOutOffset;
            resourceParams.pdwCmd          = (cmd.FrameStatisticsStreamoutDataDestinationBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FrameStatisticsStreamoutDataDestinationBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.FrameStatisticsStreamoutDataDestinationBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SSE Source Pixel Row Store Buffer
        if (params.presSseSrcPixelRowStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presSseSrcPixelRowStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SseSourcePixelRowstoreBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SseSourcePixelRowstoreBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SseSourcePixelRowstoreBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Slice state stream out buffer
        if (params.presSliceStateStreamOutBuffer != nullptr)
        {
            resourceParams.presResource    = params.presSliceStateStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.HcpScalabilitySliceStateBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(HcpScalabilitySliceStateBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.HcpScalabilitySliceStateBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // CABAC Syntax stream out buffer
        if (params.presCABACSyntaxStreamOutBuffer != nullptr)
        {
            resourceParams.presResource    = params.presCABACSyntaxStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.HcpScalabilityCabacDecodedSyntaxElementsBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(HcpScalabilityCabacDecodedSyntaxElementsBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.HcpScalabilityCabacDecodedSyntaxElementsBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // MV Upper Right Col Store
        if (params.presMvUpRightColStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presMvUpRightColStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.MotionVectorUpperRightColumnStoreBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(MotionVectorUpperRightColumnStoreBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.MotionVectorUpperRightColumnStoreBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // IntraPred Upper Right Col Store
        if (params.presIntraPredUpRightColStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presIntraPredUpRightColStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.IntraPredictionUpperRightColumnStoreBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(IntraPredictionUpperRightColumnStoreBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.IntraPredictionUpperRightColumnStoreBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // IntraPred Left Recon Col Store
        if (params.presIntraPredLeftReconColStoreBuffer != nullptr)
        {
            resourceParams.presResource    = params.presIntraPredLeftReconColStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.IntraPredictionLeftReconColumnStoreBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(IntraPredictionLeftReconColumnStoreBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.IntraPredictionLeftReconColumnStoreBufferAttributesReadWrite.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // CABAC Syntax Stream Out Buffer Max Address
        if (params.presCABACSyntaxStreamOutMaxAddr != nullptr)
        {
            resourceParams.presResource    = params.presCABACSyntaxStreamOutMaxAddr;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.HcpScalabilityCabacDecodedSyntaxElementsBufferMaxAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(HcpScalabilityCabacDecodedSyntaxElementsBufferMaxAddress);
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        MOS_ZeroMemory(&details, sizeof(details));
        details.Format            = Format_Invalid;
        MEDIA_WA_TABLE *m_waTable = this->m_osItf->pfnGetWaTable(this->m_osItf);
        MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, &params.psPreDeblockSurface->OsResource, &details));
        cmd.DecodedPictureMemoryAddressAttributes.DW0.TileMode = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            // All Reference Picture Buffer share same memory address attribute
            // As Reference Surface attribute should be aligned with Recon surface
            // Reference surface tilemode info won't get from GMM but just eqaul to DecodedPicture
            if (params.presReferences[i] != nullptr)
            {
                cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.TileMode = cmd.DecodedPictureMemoryAddressAttributes.DW0.TileMode;
                break;
            }
        }

        if (params.psRawSurface != nullptr)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, &params.psRawSurface->OsResource, &details));
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.TileMode = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_FQM_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_FQM_STATE);

        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.Quantizermatrix[i] = params.quantizermatrix[i];
        }

        cmd.DW1.IntraInter     = params.intraInter;
        cmd.DW1.Sizeid         = params.sizeid;
        cmd.DW1.ColorComponent = params.colorComponent;
        cmd.DW1.FqmDcValue1Dc  = params.fqmDcValue1Dc;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_PAK_INSERT_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(HCP_PAK_INSERT_OBJECT);

        uint32_t dwordsUsed = cmd.dwSize;

        dwordsUsed += params.dwPadding;
#define DO_FIELDS()                                                                             \
    DO_FIELD(DW0, DwordLength, OP_LENGTH(dwordsUsed));                                          \
    DO_FIELD(DW1, Headerlengthexcludefrmsize, params.bHeaderLengthExcludeFrmSize);              \
    DO_FIELD(DW1, EndofsliceflagLastdstdatainsertcommandflag, params.bEndOfSlice);              \
    DO_FIELD(DW1, LastheaderflagLastsrcheaderdatainsertcommandflag, params.bLastHeader);        \
    DO_FIELD(DW1, EmulationflagEmulationbytebitsinsertenable, params.bEmulationByteBitsInsert); \
    DO_FIELD(DW1, SkipemulbytecntSkipEmulationByteCount, params.uiSkipEmulationCheckCount);     \
    DO_FIELD(DW1, SliceHeaderIndicator, params.bResetBitstreamStartingPos);                     \
    DO_FIELD(DW1, DatabitsinlastdwSrcdataendingbitinclusion50, params.dataBitsInLastDw);        \
    DO_FIELD(DW1, DatabyteoffsetSrcdatastartingbyteoffset10, params.databyteoffset);            \
    DO_FIELD(DW1, IndirectPayloadEnable, params.bIndirectPayloadEnable);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_VP9_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_VP9_PIC_STATE);

#define DO_FIELDS()                                                                              \
    if (params.bDecodeInUse)                                                                     \
    {                                                                                            \
        DO_FIELD(DW0, DwordLength, params.dWordLength);                                          \
    }                                                                                            \
    DO_FIELD(DW1, FrameWidthInPixelsMinus1, params.frameWidthInPixelsMinus1);                    \
    DO_FIELD(DW1, FrameHeightInPixelsMinus1, params.frameHeightInPixelsMinus1);                  \
    DO_FIELD(DW2, FrameType, params.frameType);                                                  \
    DO_FIELD(DW2, AdaptProbabilitiesFlag, params.adaptProbabilitiesFlag);                        \
    DO_FIELD(DW2, IntraonlyFlag, params.intraOnlyFlag);                                          \
    DO_FIELD(DW2, AllowHiPrecisionMv, params.allowHiPrecisionMv);                                \
    DO_FIELD(DW2, McompFilterType, params.mcompFilterType);                                      \
    DO_FIELD(DW2, RefFrameSignBias02, params.refFrameSignBias02);                                \
    DO_FIELD(DW2, HybridPredictionMode, params.hybridPredictionMode);                            \
    DO_FIELD(DW2, SelectableTxMode, params.selectableTxMode);                                    \
    DO_FIELD(DW2, UsePrevInFindMvReferences, params.usePrevInFindMvReferences);                  \
    DO_FIELD(DW2, LastFrameType, params.lastFrameType);                                          \
    DO_FIELD(DW2, RefreshFrameContext, params.refreshFrameContext);                              \
    DO_FIELD(DW2, ErrorResilientMode, params.errorResilientMode);                                \
    DO_FIELD(DW2, FrameParallelDecodingMode, params.frameParallelDecodingMode);                  \
    DO_FIELD(DW2, FilterLevel, params.filterLevel);                                              \
    DO_FIELD(DW2, SharpnessLevel, params.sharpnessLevel);                                        \
    DO_FIELD(DW2, SegmentationEnabled, params.segmentationEnabled);                              \
    DO_FIELD(DW2, SegmentationUpdateMap, params.segmentationUpdateMap);                          \
    DO_FIELD(DW2, SegmentationTemporalUpdate, params.segmentationTemporalUpdate);                \
    DO_FIELD(DW2, LosslessMode, params.losslessMode);                                            \
    DO_FIELD(DW2, SegmentIdStreamoutEnable, params.segmentIdStreamOutEnable);                    \
    DO_FIELD(DW2, SegmentIdStreaminEnable, params.segmentIdStreamInEnable);                      \
    DO_FIELD(DW3, Log2TileColumn, params.log2TileColumn);                                        \
    DO_FIELD(DW3, Log2TileRow, params.log2TileRow);                                              \
    DO_FIELD(DW3, SseEnable, params.sseEnable);                                                  \
    DO_FIELD(DW3, ChromaSamplingFormat, params.chromaSamplingFormat);                            \
    DO_FIELD(DW3, Bitdepthminus8, params.bitdepthMinus8);                                        \
    DO_FIELD(DW3, ProfileLevel, params.profileLevel);                                            \
    DO_FIELD(DW4, VerticalScaleFactorForLast, params.verticalScaleFactorForLast);                \
    DO_FIELD(DW4, HorizontalScaleFactorForLast, params.horizontalScaleFactorForLast);            \
    DO_FIELD(DW5, VerticalScaleFactorForGolden, params.verticalScaleFactorForGolden);            \
    DO_FIELD(DW5, HorizontalScaleFactorForGolden, params.horizontalScaleFactorForGolden);        \
    DO_FIELD(DW6, VerticalScaleFactorForAltref, params.verticalScaleFactorForAltref);            \
    DO_FIELD(DW6, HorizontalScaleFactorForAltref, params.horizontalScaleFactorForAltref);        \
    DO_FIELD(DW7, LastFrameWidthInPixelsMinus1, params.lastFrameWidthInPixelsMinus1);            \
    DO_FIELD(DW7, LastFrameHieghtInPixelsMinus1, params.lastFrameHeightInPixelsMinus1);          \
    DO_FIELD(DW8, GoldenFrameWidthInPixelsMinus1, params.goldenFrameWidthInPixelsMinus1);        \
    DO_FIELD(DW8, GoldenFrameHieghtInPixelsMinus1, params.goldenFrameHeightInPixelsMinus1);      \
    DO_FIELD(DW9, AltrefFrameWidthInPixelsMinus1, params.altrefFrameWidthInPixelsMinus1);        \
    DO_FIELD(DW9, AltrefFrameHieghtInPixelsMinus1, params.altrefFrameHeightInPixelsMinus1);      \
    DO_FIELD(DW10, UncompressedHeaderLengthInBytes70, params.uncompressedHeaderLengthInBytes70); \
    DO_FIELD(DW10, FirstPartitionSizeInBytes150, params.firstPartitionSizeInBytes150);           \
    DO_FIELD(DW13, BaseQIndexSameAsLumaAc, params.baseQIndexSameAsLumaAc);                       \
    DO_FIELD(DW13, HeaderInsertionEnable, params.headerInsertionEnable);                         \
    DO_FIELD(DW14, ChromaacQindexdelta, params.chromaAcQIndexDelta);                             \
    DO_FIELD(DW14, ChromadcQindexdelta, params.chromaDcQIndexDelta);                             \
    DO_FIELD(DW14, LumaDcQIndexDelta, params.lumaDcQIndexDelta);                                 \
    DO_FIELD(DW15, LfRefDelta0, params.lfRefDelta0);                                             \
    DO_FIELD(DW15, LfRefDelta1, params.lfRefDelta1);                                             \
    DO_FIELD(DW15, LfRefDelta2, params.lfRefDelta2);                                             \
    DO_FIELD(DW15, LfRefDelta3, params.lfRefDelta3);                                             \
    DO_FIELD(DW16, LfModeDelta0, params.lfModeDelta0);                                           \
    DO_FIELD(DW16, LfModeDelta1, params.lfModeDelta1);                                           \
    DO_FIELD(DW17, Bitoffsetforlfrefdelta, params.bitOffsetForLfRefDelta);                       \
    DO_FIELD(DW17, Bitoffsetforlfmodedelta, params.bitOffsetForLfModeDelta);                     \
    DO_FIELD(DW18, Bitoffsetforqindex, params.bitOffsetForQIndex);                               \
    DO_FIELD(DW18, Bitoffsetforlflevel, params.bitOffsetForLfLevel);                             \
    DO_FIELD(DW19, VdencPakOnlyPass, params.vdencPakOnlyPass);                                   \
    DO_FIELD(DW32, Bitoffsetforfirstpartitionsize, params.bitOffsetForFirstPartitionSize);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_VP9_SEGMENT_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_VP9_SEGMENT_STATE);

#define DO_FIELDS()                                                                           \
    DO_FIELD(DW1, SegmentId, params.segmentId);                                               \
    DO_FIELD(DW2, SegmentSkipped, params.segmentSkipped);                                     \
    DO_FIELD(DW2, SegmentReference, params.segmentReference);                                 \
    DO_FIELD(DW2, SegmentReferenceEnabled, params.segmentReferenceEnabled);                   \
    DO_FIELD(DW3, Filterlevelref0Mode0, params.filterLevelRef0Mode0);                         \
    DO_FIELD(DW3, Filterlevelref0Mode1, params.filterLevelRef0Mode1);                         \
    DO_FIELD(DW3, Filterlevelref1Mode0, params.filterLevelRef1Mode0);                         \
    DO_FIELD(DW3, Filterlevelref1Mode1, params.filterLevelRef1Mode1);                         \
    DO_FIELD(DW4, Filterlevelref2Mode0, params.filterLevelRef2Mode0);                         \
    DO_FIELD(DW4, Filterlevelref2Mode1, params.filterLevelRef2Mode1);                         \
    DO_FIELD(DW4, Filterlevelref3Mode0, params.filterLevelRef3Mode0);                         \
    DO_FIELD(DW4, Filterlevelref3Mode1, params.filterLevelRef3Mode1);                         \
    DO_FIELD(DW5, LumaDcQuantScaleDecodeModeOnly, params.lumaDcQuantScaleDecodeModeOnly);     \
    DO_FIELD(DW5, LumaAcQuantScaleDecodeModeOnly, params.lumaAcQuantScaleDecodeModeOnly);     \
    DO_FIELD(DW6, ChromaDcQuantScaleDecodeModeOnly, params.chromaDcQuantScaleDecodeModeOnly); \
    DO_FIELD(DW6, ChromaAcQuantScaleDecodeModeOnly, params.chromaAcQuantScaleDecodeModeOnly); \
    DO_FIELD(DW7, SegmentQindexDeltaEncodeModeOnly, params.segmentQindexDeltaEncodeModeOnly); \
    DO_FIELD(DW7, SegmentLfLevelDeltaEncodeModeOnly, params.segmentLfLevelDeltaEncodeModeOnly);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HEVC_VP9_RDOQ_STATE)
    {
        _MHW_SETCMD_CALLBASE(HEVC_VP9_RDOQ_STATE);

        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.Intralumalambda[i].DW0.Lambdavalue0 = params.lambdaTab[0][0][i * 2];
            cmd.Intralumalambda[i].DW0.Lambdavalue1 = params.lambdaTab[0][0][i * 2 + 1];

            cmd.Intrachromalambda[i].DW0.Lambdavalue0 = params.lambdaTab[0][1][i * 2];
            cmd.Intrachromalambda[i].DW0.Lambdavalue1 = params.lambdaTab[0][1][i * 2 + 1];

            cmd.Interlumalambda[i].DW0.Lambdavalue0 = params.lambdaTab[1][0][i * 2];
            cmd.Interlumalambda[i].DW0.Lambdavalue1 = params.lambdaTab[1][0][i * 2 + 1];

            cmd.Interchromalambda[i].DW0.Lambdavalue0 = params.lambdaTab[1][1][i * 2];
            cmd.Interchromalambda[i].DW0.Lambdavalue1 = params.lambdaTab[1][1][i * 2 + 1];
        }

        for (uint8_t i = 0; i < 6; i++)
        {
            cmd.Intralumalambda12bit[i].DW0.Lambdavalue0 = params.lambdaTab[0][0][i * 2 + 64];
            cmd.Intralumalambda12bit[i].DW0.Lambdavalue1 = params.lambdaTab[0][0][i * 2 + 1 + 64];

            cmd.Intrachromalambda12bit[i].DW0.Lambdavalue0 = params.lambdaTab[0][1][i * 2 + 64];
            cmd.Intrachromalambda12bit[i].DW0.Lambdavalue1 = params.lambdaTab[0][1][i * 2 + 1 + 64];

            cmd.Interlumalambda12bit[i].DW0.Lambdavalue0 = params.lambdaTab[1][0][i * 2 + 64];
            cmd.Interlumalambda12bit[i].DW0.Lambdavalue1 = params.lambdaTab[1][0][i * 2 + 1 + 64];

            cmd.Interchromalambda12bit[i].DW0.Lambdavalue0 = params.lambdaTab[1][1][i * 2 + 64];
            cmd.Interchromalambda12bit[i].DW0.Lambdavalue1 = params.lambdaTab[1][1][i * 2 + 1 + 64];
        }

        cmd.DW1.DisableHtqPerformanceFix0 = params.disableHtqPerformanceFix0;
        cmd.DW1.DisableHtqPerformanceFix1 = params.disableHtqPerformanceFix1;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_TILE_CODING)
    {
        _MHW_SETCMD_CALLBASE(HCP_TILE_CODING);

#define DO_FIELDS()                                                             \
    DO_FIELD(DW1, NumberOfActiveBePipes, params.numberOfActiveBePipes);         \
    DO_FIELD(DW1, NumOfTileColumnsInAFrame, params.numOfTileColumnsInFrame);    \
    DO_FIELD(DW1, TileRowStoreSelect, params.tileRowStoreSelect);               \
    DO_FIELD(DW1, TileColumnStoreSelect, params.tileColumnStoreSelect);         \
    DO_FIELD(DW2, TileColumnPosition, params.tileStartLCUX);                    \
    DO_FIELD(DW2, TileRowPosition, params.tileStartLCUY);                       \
    DO_FIELD(DW2, Islasttileofcolumn, params.isLastTileofColumn);               \
    DO_FIELD(DW2, Islasttileofrow, params.isLastTileofRow);                     \
    DO_FIELD(DW2, NonFirstPassTile, params.nonFirstPassTile);                   \
    DO_FIELD(DW3, Tileheightinmincbminus1, params.tileHeightInMinCbMinus1);     \
    DO_FIELD(DW3, Tilewidthinmincbminus1, params.tileWidthInMinCbMinus1);       \
    DO_FIELD(DW4, BitstreamByteOffset, params.bitstreamByteOffset);             \
    DO_FIELD(DW4, BitstreamByteOffsetEnable, params.bitstreamByteOffsetEnable); \
    DO_FIELD(DW5, PakFrameStatisticsOffset, params.pakTileStatisticsOffset);    \
    DO_FIELD(DW6, CuLevelStreamoutOffset, params.cuLevelStreamoutOffset);       \
    DO_FIELD(DW7, SliceSizeStreamoutOffset, params.sliceSizeStreamoutOffset);   \
    DO_FIELD(DW8, CuRecordOffset, params.cuRecordOffset);                       \
    DO_FIELD(DW9, SseRowstoreOffset, params.sseRowstoreOffset);                 \
    DO_FIELD(DW10, SaoRowstoreOffset, params.saoRowstoreOffset);                \
    DO_FIELD(DW11, TileSizeStreamoutOffset, params.tileSizeStreamoutOffset);    \
    DO_FIELD(DW12, Vp9ProbabilityCounterStreamoutOffset, params.vp9ProbabilityCounterStreamoutOffset);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HCP_PALETTE_INITIALIZER_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_PALETTE_INITIALIZER_STATE);

        cmd.DW1.ActivePaletteInitializerTableEntries = params.predictorPaletteSize;

        uint32_t yentryIdx = 0;
        for (uint32_t i = 0; i < params.hevcSccPaletteSize; i += 3)
        {
            // First 64 color entries
            yentryIdx = i * 2 / 3;
            cmd.First64ColorEntries[i] = params.predictorPaletteEntries[0][yentryIdx]; // Y
            cmd.First64ColorEntries[i] |= ((uint32_t)params.predictorPaletteEntries[1][yentryIdx] << 16); // Cb

            cmd.First64ColorEntries[i + 1] = params.predictorPaletteEntries[2][yentryIdx]; // Cr
            cmd.First64ColorEntries[i + 1] |= ((uint32_t)params.predictorPaletteEntries[0][yentryIdx + 1] << 16); // Y

            cmd.First64ColorEntries[i + 2] = params.predictorPaletteEntries[1][yentryIdx + 1]; // Cb
            cmd.First64ColorEntries[i + 2] |= ((uint32_t)params.predictorPaletteEntries[2][yentryIdx + 1] << 16); // Cr

            // Second 64 color entries
            yentryIdx += 64;
            cmd.Second64ColorEntries[i] = params.predictorPaletteEntries[0][yentryIdx]; // Y
            cmd.Second64ColorEntries[i] |= ((uint32_t)params.predictorPaletteEntries[1][yentryIdx] << 16); // Cb

            cmd.Second64ColorEntries[i + 1] = params.predictorPaletteEntries[2][yentryIdx]; // Cr
            cmd.Second64ColorEntries[i + 1] |= ((uint32_t)params.predictorPaletteEntries[0][yentryIdx + 1] << 16); // Y

            cmd.Second64ColorEntries[i + 2] = params.predictorPaletteEntries[1][yentryIdx + 1]; // Cb
            cmd.Second64ColorEntries[i + 2] |= ((uint32_t)params.predictorPaletteEntries[2][yentryIdx + 1] << 16); // Cr
        }

        return MOS_STATUS_SUCCESS;
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__hcp__Impl)
};
}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HCP_IMPL_H__
