/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     mhw_vdbox_avp_impl.h
//! \brief    MHW VDBOX AVP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_AVP_IMPL_H__
#define __MHW_VDBOX_AVP_IMPL_H__

#include "mhw_vdbox_avp_itf.h"
#include "mhw_impl.h"
#include "mos_interface.h"

namespace mhw
{
namespace vdbox
{
namespace avp
{
static constexpr uint32_t AVP_AVP_BITSTREAM_BYTECOUNT_TILE_WITH_HEADER_AWM_REG0       = 0x1C2B48;
static constexpr uint32_t AVP_ENC_BITSTREAM_BYTE_COUNT_TILE_NO_HEADER_REG_OFFSET_INIT = 0x1C2B4C;
static constexpr uint32_t AVP_ENC_CABAC_BIN_COUNT_TILE_REG_OFFSET_INIT                = 0x1C2B50;
static constexpr uint32_t AVP_ENC_CABAC_INSERTION_COUNT_REG_OFFSET_INIT               = 0x1C2B54;
static constexpr uint32_t AVP_ENC_MIN_SIZE_PADDING_COUNT_REG_OFFSET_INIT              = 0x1C2B58;
static constexpr uint32_t AVP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT                   = 0x1C2B5C;
static constexpr uint32_t AVP_ENC_IMAGE_STATUS_CONTROL_REG_OFFSET_INIT                = 0x1C2B60;
static constexpr uint32_t AVP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT                     = 0x1C2B64;
static constexpr uint32_t AVP_DEC_ERROR_STATUS_ADDR_REG_OFFSET_INIT                   = 0x1C2B08;

static constexpr uint32_t MEMORY_ADDRESS_ATTRIBUTES_MOCS_CLEAN_MASK = 0xFFFFFF81;

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _AVP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);

        size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

    MOS_STATUS GetAvpBufSize(AvpBufferType bufferType, AvpBufferSizePar *avpBufSizeParam) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(avpBufSizeParam);
        MHW_ASSERT(avpBufSizeParam->bitDepthIdc == 0 || avpBufSizeParam->bitDepthIdc == 1);

        uint32_t bufferSize = 0;

        MHW_MI_CHK_STATUS(CalculateBufferSize(bufferType, avpBufSizeParam, avpBufferSize, avpBufferSizeExt, bufferSize));

        avpBufSizeParam->bufferSize = bufferSize * MHW_CACHELINE_SIZE;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS CalculateBufferSize(AvpBufferType    bufferType,
                                   AvpBufferSizePar *avpBufSizeParam,
                                   const uint8_t    avpBufferSizeTbl[][2][2],
                                   const uint8_t    avpBufferSizeExtTbl[][2][2],
                                   uint32_t         &bufferSize)
    {
        MHW_FUNCTION_ENTER;

        uint32_t sbPerFrmWid     = avpBufSizeParam->width;
        uint32_t sbPerFrmHgt     = avpBufSizeParam->height;
        uint32_t sbPerTileWid    = avpBufSizeParam->tileWidth;
        uint32_t totalSbPerFrame = sbPerFrmWid * sbPerFrmHgt;
        uint32_t index           = (uint32_t)bufferType;
        uint32_t maxCuPerSB      = avpBufSizeParam->isSb128x128 ? 256 : 64;

        switch (bufferType)
        {
            //Rowstore buffers, Total CLs = (#CLs_per_SB * num_of_SB_per_tile_width)
            case bsdLineBuffer:
            case spatialMvLineBuffer:
            case intraPredLineBuffer:
            case deblockLineYBuffer:
            case deblockLineUBuffer:
            case deblockLineVBuffer:
                bufferSize = sbPerTileWid * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
            case cdefLineBuffer:
                bufferSize = sbPerTileWid * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128]
                    + avpBufferSizeExtTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
                //Tile storage - tile line buffers, Total CLs = (#CLs_per_SB * num_of_SB_per_row) = (#CLs_per_SB * num_of_SB_per_frame_width)
            case bsdTileLineBuffer:
            case spatialMvTileLineBuffer:
            case intraPredTileLineBuffer:
            case deblockTileLineYBuffer:
            case deblockTileLineUBuffer:
            case deblockTileLineVBuffer:
                bufferSize = sbPerFrmWid * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
                //Tile storage - tile column buffers, Total CLs = (#CLs_per_SB * num_of_SB_per_column) = (#CLs_per_SB * num_of_SB_per_frame_height)
            case deblockTileColYBuffer:
            case deblockTileColUBuffer:
            case deblockTileColVBuffer:
                bufferSize = sbPerFrmHgt * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
                // Tile storage, per tile number
            case cdefTopLeftCornerBuffer:
                bufferSize = avpBufSizeParam->curFrameTileNum * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
            case cdefMetaTileLineBuffer:
                bufferSize = avpBufSizeParam->numTileCol;
                break;
            case lrTileLineYBuffer:
                bufferSize = avpBufSizeParam->numTileCol * 7;
                break;
            case lrTileLineUBuffer:
            case lrTileLineVBuffer:
                bufferSize = avpBufSizeParam->numTileCol * 5;
                break;
                // Tile storage, - tile line buffers, with extra size
            case cdefTileLineBuffer:
                bufferSize = sbPerFrmWid * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128]
                    + avpBufferSizeExtTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
                // Tile storage, - tile column buffers, with extra size
            case cdefTileColBuffer:
            case cdefMetaTileColBuffer:
            case superResTileColYBuffer:
            case superResTileColUBuffer:
            case superResTileColVBuffer:
            case lrTileColYBuffer:
            case lrTileColUBuffer:
            case lrTileColVBuffer:
            case lrMetaTileColBuffer:
                bufferSize = sbPerFrmHgt * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128]
                    + avpBufferSizeExtTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
                //frame buffer
            case segmentIdBuffer:
                bufferSize = ((avpBufSizeParam->isSb128x128)? 8 : 2) * totalSbPerFrame;
                break;
            case mvTemporalBuffer:
                bufferSize = ((avpBufSizeParam->isSb128x128) ? 16 : 4) * totalSbPerFrame;
                break;
            case frameStatusErrBuffer:
            case dbdStreamoutBuffer:
                bufferSize = 1;
                break;
            case tileSzStreamOutBuffer:
                bufferSize = avpBufSizeParam->numTileCol * avpBufSizeParam->numTileCol * MHW_CACHELINE_SIZE;
                break;
            case tileStatStreamOutBuffer:
                bufferSize = 512;
                break;
            case cuStreamoutBuffer:
                bufferSize = MOS_ALIGN_CEIL(totalSbPerFrame * maxCuPerSB * 8, MHW_CACHELINE_SIZE); // Each CU streams out 8 bytes
                break;
            case sseLineBuffer:
            case sseTileLineBuffer:
                if (avpBufSizeParam->numOfActivePipes > 1)
                {
                    // To be added for scalability case
                }
                else
                {
                    bufferSize = (sbPerFrmWid + 3) * MHW_CACHELINE_SIZE * (8 + 8) << 1; // 8 for Luma, 8 for Chroma
                }
                break;
            case lrTileColAlignBuffer:
            case fgTileColBuffer:
                bufferSize = sbPerFrmHgt * avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128]
                    + avpBufferSizeExtTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
            case fgSampleTmpBuffer:
                bufferSize = avpBufferSizeTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128]
                    + avpBufferSizeExtTbl[index][avpBufSizeParam->bitDepthIdc][avpBufSizeParam->isSb128x128];
                break;
            default:
                return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    bool IsRowStoreCachingSupported() override
    {
        return m_rowstoreCachingSupported;
    }

    bool IsBufferRowstoreCacheEnabled(AvpBufferType bufferType) override
    {
        bool rowstoreCacheEnabled = false;
        switch (bufferType)
        {
            case bsdLineBuffer:
                rowstoreCacheEnabled = m_btdlRowstoreCache.enabled ? true : false;
                break;
            case spatialMvLineBuffer:
                rowstoreCacheEnabled = m_smvlRowstoreCache.enabled ? true : false;
                break;
            case intraPredLineBuffer:
                rowstoreCacheEnabled = m_ipdlRowstoreCache.enabled ? true : false;
                break;
            case deblockLineYBuffer:
                rowstoreCacheEnabled = m_dflyRowstoreCache.enabled ? true : false;
                break;
            case deblockLineUBuffer:
                rowstoreCacheEnabled = m_dfluRowstoreCache.enabled ? true : false;
                break;
            case deblockLineVBuffer:
                rowstoreCacheEnabled = m_dflvRowstoreCache.enabled ? true : false;
                break;
            case cdefLineBuffer:
                rowstoreCacheEnabled = m_cdefRowstoreCache.enabled ? true : false;
                break;
            default:
                return false;
        }

        return rowstoreCacheEnabled;
    }

    MOS_STATUS GetAvpStateCmdSize(uint32_t* commandsSize, uint32_t* patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetAvpPrimitiveCmdSize(uint32_t* commandsSize, uint32_t* patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    AvpMmioRegisters* GetMmioRegisters(MHW_VDBOX_NODE_IND index) override
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return (&m_mmioRegisters[index]);
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return (&m_mmioRegisters[MHW_VDBOX_NODE_1]);
        }
    }

private:
    bool                 m_rowstoreCachingSupported            = false;
    AvpMmioRegisters     m_mmioRegisters[MHW_VDBOX_NODE_MAX]   = {};    //!< AVP mmio registers

    void InitMmioRegisters()
    {
        AvpMmioRegisters *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

        mmioRegisters->avpAv1BitstreamByteCountTileRegOffset         = AVP_AVP_BITSTREAM_BYTECOUNT_TILE_WITH_HEADER_AWM_REG0;
        mmioRegisters->avpAv1BitstreamByteCountTileNoHeaderRegOffset = AVP_ENC_BITSTREAM_BYTE_COUNT_TILE_NO_HEADER_REG_OFFSET_INIT;
        mmioRegisters->avpAv1CabacBinCountTileRegOffset              = AVP_ENC_CABAC_BIN_COUNT_TILE_REG_OFFSET_INIT;
        mmioRegisters->avpAv1CabacInsertionCountRegOffset            = AVP_ENC_CABAC_INSERTION_COUNT_REG_OFFSET_INIT;
        mmioRegisters->avpAv1MinSizePaddingCountRegOffset            = AVP_ENC_MIN_SIZE_PADDING_COUNT_REG_OFFSET_INIT;
        mmioRegisters->avpAv1ImageStatusMaskRegOffset                = AVP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT;
        mmioRegisters->avpAv1ImageStatusControlRegOffset             = AVP_ENC_IMAGE_STATUS_CONTROL_REG_OFFSET_INIT;
        mmioRegisters->avpAv1QpStatusCountRegOffset                  = AVP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT;
        mmioRegisters->avpAv1DecErrorStatusAddrRegOffset             = AVP_DEC_ERROR_STATUS_ADDR_REG_OFFSET_INIT;

        m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
    }

    MOS_STATUS InitRowstoreUserFeatureSettings()
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
            m_btdlRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1BtdlRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_btdlRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_smvlRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1SmvlRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_smvlRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_ipdlRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1IpdlRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_ipdlRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_dflyRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1DflyRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_dflyRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_dfluRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1DfluRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_dfluRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_dflvRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1DflvRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_dflvRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_cdefRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableAv1CdefRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_cdefRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL
        }

        return MOS_STATUS_SUCCESS;
    }

    //AVP internal buffer size table [buffer_index][bitdepthIdc][IsSb128x128]
    const uint8_t avpBufferSize[avpInternalBufferMax][2][2] =
    {
        { 2 ,   8   ,   2   ,    8 },  //segmentIdBuffer,
        { 4 ,   16  ,   4   ,    16 }, //mvTemporalBuffer,
        { 2 ,   4   ,   2   ,    4 },  //bsdLineBuffer,
        { 2 ,   4   ,   2   ,    4 },  //bsdTileLineBuffer,
        { 2 ,   4   ,   4   ,    8 },  //intraPredLineBuffer,
        { 2 ,   4   ,   4   ,    8 },  //intraPredTileLineBuffer,
        { 4 ,   8   ,   4   ,    8 },  //spatialMvLineBuffer,
        { 4 ,   8   ,   4   ,    8 },  //spatialMvTileLineBuffer,
        { 1 ,   1   ,   1   ,    1 },  //lrMetaTileColBuffer,
        { 7 ,   7   ,   7   ,    7 },  //lrTileLineYBuffer,
        { 5 ,   5   ,   5   ,    5 },  //lrTileLineUBuffer,
        { 5 ,   5   ,   5   ,    5 },  //lrTileLineVBuffer,
        { 9 ,   17  ,   11  ,    21 }, //deblockLineYBuffer,
        { 3 ,   4   ,   3   ,    5 },  //deblockLineUBuffer,
        { 3 ,   4   ,   3   ,    5 },  //deblockLineVBuffer,
        { 9 ,   17  ,   11  ,    21 }, //deblockTileLineYBuffer,
        { 3 ,   4   ,   3   ,    5 },  //deblockTileLineVBuffer,
        { 3 ,   4   ,   3   ,    5 },  //deblockTileLineUBuffer,
        { 8 ,   16  ,   10  ,    20 }, //deblockTileColYBuffer,
        { 2 ,   4   ,   3   ,    5 },  //deblockTileColUBuffer,
        { 2 ,   4   ,   3   ,    5 },  //deblockTileColVBuffer,
        { 8 ,   16  ,   10  ,    20 }, //cdefLineBufferBuffer,
        { 8 ,   16  ,   10  ,    20 }, //cdefTileLineBufBuffer,
        { 8 ,   16  ,   10  ,    20 }, //cdefTileColBufBuffer,
        { 1 ,   1   ,   1   ,    1 },  //cdefMetaTileLineBuffer,
        { 1 ,   1   ,   1   ,    1 },  //cdefMetaTileColBuffer,
        { 1 ,   1   ,   1   ,    1 },  //cdefTopLeftCornerBuffer,
        { 22,   44  ,   29  ,    58 }, //superResTileColYBuffer,
        { 8 ,   16  ,   10  ,    20 }, //superResTileColUBuffer,
        { 8 ,   16  ,   10  ,    20 }, //superResTileColVBuffer,
        { 9 ,   17  ,   11  ,    22 }, //lrTileColYBuffer,
        { 5 ,   9   ,   6   ,    12 }, //lrTileColUBuffer,
        { 5 ,   9   ,   6   ,    12 }, //lrTileColVBuffer,
        { 0 ,   0   ,   0   ,    0 },  //frameStatusErrBuffer,
        { 0 ,   0   ,   0   ,    0 },  //dbdStreamoutBuffer,
        { 2 ,   4   ,   3   ,    5 },  //fgTileColBuffer
        { 96,   96  ,   192 ,    192 },//fgSampleTmpBuffer
        { 4,    8   ,   5   ,    10 }, //lrTileColAlignBuffer
    };

    const uint8_t avpBufferSizeExt[avpInternalBufferMax][2][2] =
    {
        { 0 ,    0    ,    0    ,    0 },  //segmentIdBuffer,
        { 0 ,    0    ,    0    ,    0 },  //mvTemporalBuffer,
        { 0 ,    0    ,    0    ,    0 },  //bsdLineBuffer,
        { 0 ,    0    ,    0    ,    0 },  //bsdTileLineBuffer,
        { 0 ,    0    ,    0    ,    0 },  //intraPredLineBuffer,
        { 0 ,    0    ,    0    ,    0 },  //intraPredTileLineBuffer,
        { 0 ,    0    ,    0    ,    0 },  //spatialMvLineBuffer,
        { 0 ,    0    ,    0    ,    0 },  //spatialMvTileLineBuffer,
        { 1 ,    1    ,    1    ,    1 },  //lrMetaTileColBuffer,
        { 0 ,    0    ,    0    ,    0 },  //lrTileLineYBuffer,
        { 0 ,    0    ,    0    ,    0 },  //lrTileLineUBuffer,
        { 0 ,    0    ,    0    ,    0 },  //lrTileLineVBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockLineYBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockLineUBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockLineVBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockTileLineYBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockTileLineVBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockTileLineUBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockTileColYBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockTileColUBuffer,
        { 0 ,    0    ,    0    ,    0 },  //deblockTileColVBuffer,
        { 1 ,    1    ,    2    ,    2 },  //cdefLineBuffer,
        { 1 ,    1    ,    2    ,    2 },  //cdefTileLineBuffer,
        { 1 ,    1    ,    2    ,    2 },  //cdefTileColBuffer,
        { 0 ,    0    ,    0    ,    0 },  //cdefMetaTileLineBuffer,
        { 1 ,    1    ,    1    ,    1 },  //cdefMetaTileColBuffer,
        { 0 ,    0    ,    0    ,    0 },  //cdefTopLeftCornerBuffer,
        { 22,    44   ,    29   ,    58 }, //superResTileColYBuffer,
        { 8 ,    16   ,    10   ,    20 }, //superResTileColUBuffer,
        { 8 ,    16   ,    10   ,    20 }, //superResTileColVBuffer,
        { 2 ,    2    ,    2    ,    2 },  //lrTileColYBuffer,
        { 1 ,    1    ,    1    ,    1 },  //lrTileColUBuffer,
        { 1 ,    1    ,    1    ,    1 },  //lrTileColVBuffer,
        { 0 ,    0    ,    0    ,    0 },  //frameStatusErrBuffer,
        { 0 ,    0    ,    0    ,    0 },  //dbdStreamoutBuffer,
        { 1 ,    1    ,    1    ,    1 },  //fgTileColBuffer,
        { 0 ,    0    ,    0    ,    0 },   //fgSampleTmpBuffer,
        { 1 ,    1    ,    1    ,    1 },  //lrTileColAlignBuffer,
    };

protected:
    using base_t = Itf;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};

    vdbox::RowStoreCache    m_btdlRowstoreCache = {};   //! Bitstream Decoder/Encode Line Rowstore (BTDL)
    vdbox::RowStoreCache    m_smvlRowstoreCache = {};   //! Spatial Motion Vector Line Rowstore (SMVL)
    vdbox::RowStoreCache    m_ipdlRowstoreCache = {};   //! Intra Prediction Line Rowstore (IPDL)
    vdbox::RowStoreCache    m_dflyRowstoreCache = {};   //! Deblocker Filter Line Y Buffer (DFLY)
    vdbox::RowStoreCache    m_dfluRowstoreCache = {};   //! Deblocker Filter Line U Buffe (DFLU)
    vdbox::RowStoreCache    m_dflvRowstoreCache = {};   //! Deblocker Filter Line V Buffe (DFLV)
    vdbox::RowStoreCache    m_cdefRowstoreCache = {};   //! CDEF Filter Line Buffer (CDEF)

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
        if (m_btdlRowstoreCache.enabled ||
            m_smvlRowstoreCache.enabled ||
            m_ipdlRowstoreCache.enabled ||
            m_dflyRowstoreCache.enabled ||
            m_dfluRowstoreCache.enabled ||
            m_dflvRowstoreCache.enabled ||
            m_cdefRowstoreCache.enabled)
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

    MOS_STATUS GetRowstoreCachingAddrs(mhw::vdbox::avp::AvpVdboxRowStorePar rowstoreParams) override
    {
        //BTDL
        if (m_btdlRowstoreCache.supported)
        {
            m_btdlRowstoreCache.enabled   = true;
            m_btdlRowstoreCache.dwAddress = 0;
        }

        //SMVL
        if (m_smvlRowstoreCache.supported)
        {
            m_smvlRowstoreCache.enabled   = true;
            m_smvlRowstoreCache.dwAddress = 128;
        }

        //IPDL
        if (m_ipdlRowstoreCache.supported)
        {
            m_ipdlRowstoreCache.enabled   = true;
            m_ipdlRowstoreCache.dwAddress = 384;
        }

        //DFLY
        if (m_dflyRowstoreCache.supported)
        {
            m_dflyRowstoreCache.enabled   = true;
            m_dflyRowstoreCache.dwAddress = 640;
        }

        //DFLU
        if (m_dfluRowstoreCache.supported)
        {
            m_dfluRowstoreCache.enabled   = true;
            m_dfluRowstoreCache.dwAddress = 1344;
        }

        //DFLV
        if (m_dflvRowstoreCache.supported)
        {
            m_dflvRowstoreCache.enabled   = true;
            m_dflvRowstoreCache.dwAddress = 1536;
        }

        //CDEF
        if (m_cdefRowstoreCache.supported)
        {
            m_cdefRowstoreCache.enabled   = true;
            m_cdefRowstoreCache.dwAddress = 1728;
        }

        return MOS_STATUS_SUCCESS;
    }

    // Programming Note: CodecHAL layer must add MFX wait command
    //                   for both KIN and VRT before and after AVP_PIPE_MODE_SELECT

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_MODE_SELECT);

#define DO_FIELDS()                                                                                     \
    DO_FIELD(DW1, CodecSelect, params.codecSelect);                                                     \
    DO_FIELD(DW1, PicStatusErrorReportEnable, params.picStatusErrorReportEnable ? 1 : 0);               \
    DO_FIELD(DW1, CodecStandardSelect, params.codecStandardSelect);                                     \
    DO_FIELD(DW1, MultiEngineMode, params.multiEngineMode);                                             \
    DO_FIELD(DW1, PipeWorkingMode, params.pipeWorkingMode);                                             \
    DO_FIELD(DW1, TileBasedEngine, params.tileBasedReplayMode ? 1 : 0);                                 \
    DO_FIELD(DW3, PicStatusErrorReportId, params.picStatusErrorReportEnable ? 1 : 0);                   \
    DO_FIELD(DW5, PhaseIndicator, params.phaseIndicator);                                               \
                                                                                                        \
    DO_FIELD(DW1, FrameReconstructionDisable, params.frameReconDisable);                                \
    DO_FIELD(DW1, VdencMode, params.vdencMode);                                                         \
    DO_FIELD(DW1, MotionCompMemTrackerCounterEnable, params.motionCompMemTrackerCounterEnable ? 1 : 0)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIC_STATE);

        //DW9..DW29
        //It specifies the Warp Parameter set for each of the 7reference frames [LAST_FRAME .. ALTREF_FRAME]
        MOS_SecureMemcpy(cmd.WarpParametersArrayReference1To7Projectioncoeff0To5,
            sizeof(params.warpParamsArrayProjection),
            params.warpParamsArrayProjection,
            sizeof(params.warpParamsArrayProjection)
        );

#define DO_FIELDS()                                                                                          \
    DO_FIELD(DW1, FrameWidthInPixelMinus1, params.frameWidthMinus1);                                         \
    DO_FIELD(DW1, FrameHeightInPixelMinus1, params.frameHeightMinus1);                                       \
                                                                                                             \
    DO_FIELD(DW2, SequenceChromaSubsamplingFormat, params.chromaFormat);                                     \
    DO_FIELD(DW2, SequencePixelBitDepthIdc, params.bitDepthIdc);                                             \
    DO_FIELD(DW2, SequenceSuperblockSizeUsed, params.superblockSizeUsed);                                    \
    DO_FIELD(DW2, SequenceEnableOrderHintFlag, params.enableOrderHint ? 1 : 0);                              \
    DO_FIELD(DW2, SequenceOrderHintBitsMinus1, params.orderHintBitsMinus1);                                  \
    DO_FIELD(DW2, SequenceEnableFilterIntraFlag, params.enableFilterIntra ? 1 : 0);                          \
    DO_FIELD(DW2, SequenceEnableIntraEdgeFilterFlag, params.enableIntraEdgeFilter ? 1 : 0);                  \
    DO_FIELD(DW2, SequenceEnableDualFilterFlag, params.enableDualFilter ? 1 : 0);                            \
    DO_FIELD(DW2, SequenceEnableInterIntraCompoundFlag, params.enableInterIntraCompound ? 1 : 0);            \
    DO_FIELD(DW2, SequenceEnableMaskedCompoundFlag, params.enableMaskedCompound ? 1 : 0);                    \
    DO_FIELD(DW2, SequenceEnableJointCompoundFlag, params.enableJointCompound ? 1 : 0);                      \
    DO_FIELD(DW2, HeaderPresentFlag, params.headerPresent ? 1 : 0);                                          \
    DO_FIELD(DW2, EnableBistreamStitchingInHardware, params.autoBistreamStitchingInHardware);                \
                                                                                                             \
    DO_FIELD(DW3, AllowScreenContentToolsFlag, params.allowScreenContentTools ? 1 : 0);                      \
    DO_FIELD(DW3, ForceIntegerMvFlag, params.forceIntegerMv ? 1 : 0);                                        \
    DO_FIELD(DW3, AllowWarpedMotionFlag, params.allowWarpedMotion ? 1: 0);                                   \
    DO_FIELD(DW3, UseCdefFilterFlag, params.enableCDEF ? 1 : 0);                                             \
    DO_FIELD(DW3, UseSuperResFlag, params.enableSuperres ? 1 : 0);                                           \
    DO_FIELD(DW3, FrameLevelLoopRestorationFilterEnableFlag, params.enableRestoration ? 1 : 0);              \
    DO_FIELD(DW3, LargeScaleTileEnableFlag, params.enableLargeScaleTile ? 1 : 0);                            \
    DO_FIELD(DW3, PostCdefFilteredReconPixelsWriteoutEn, params.postCdefReconPixelStreamoutEn ? 1 : 0);      \
    DO_FIELD(DW3, FrameType, params.frameType);                                                              \
    DO_FIELD(DW3, IntraonlyFlag, (params.frameType == keyFrame || params.frameType == intraOnlyFrame));      \
    DO_FIELD(DW3, ErrorResilientModeFlag, params.errorResilientMode);                                        \
    DO_FIELD(DW3, AllowIntrabcFlag, params.allowIntraBC ? 1 : 0);                                            \
    DO_FIELD(DW3, PrimaryReferenceFrameIdx, params.primaryRefFrame);                                         \
    DO_FIELD(DW3, ApplyFilmGrainFlag, params.applyFilmGrainFlag);                                            \
                                                                                                             \
    DO_FIELD(DW4, SegmentationEnableFlag, params.segmentParams.m_enabled);                                   \
    DO_FIELD(DW4, SegmentationUpdateMapFlag, params.segmentParams.m_updateMap);                              \
    DO_FIELD(DW4, SegmentationTemporalUpdateFlag, params.segmentParams.m_temporalUpdate);                    \
    DO_FIELD(DW4, LastActiveSegmentId, params.segmentParams.m_lastActiveSegmentId);                          \
    DO_FIELD(DW4, PreSkipSegmentIdFlag, params.segmentParams.m_preSkipSegmentIdFlag);                        \
    DO_FIELD(DW4, DeltaQPresentFlag, params.deltaQPresentFlag ? 1 : 0);                                      \
    DO_FIELD(DW4, DeltaQRes, params.log2DeltaQRes);                                                          \
    DO_FIELD(DW4, FrameCodedLosslessMode, params.codedLossless);                                             \
    DO_FIELD(DW4, SegmentMapIsZeroFlag, params.segmentParams.m_segmentMapIsZeroFlag);                        \
    DO_FIELD(DW4, SegmentIdBufferStreamInEnableFlag, params.segmentParams.m_segIdBufStreamInEnable);         \
    DO_FIELD(DW4, SegmentIdBufferStreamOutEnableFlag, params.segmentParams.m_segIdBufStreamOutEnable);       \
    DO_FIELD(DW4, BaseQindex, params.baseQindex);                                                            \
    DO_FIELD(DW4, YDcDeltaQ, params.yDcDeltaQ);                                                              \
                                                                                                             \
    DO_FIELD(DW5, UDcDeltaQ, params.uDcDeltaQ);                                                              \
    DO_FIELD(DW5, UAcDeltaQ, params.uAcDeltaQ);                                                              \
    DO_FIELD(DW5, VDcDeltaQ, params.vDcDeltaQ);                                                              \
    DO_FIELD(DW5, VAcDeltaQ, params.vAcDeltaQ);                                                              \
                                                                                                             \
    DO_FIELD(DW6, AllowHighPrecisionMv, params.allowHighPrecisionMV);                                        \
    DO_FIELD(DW6, FrameLevelReferenceModeSelect, params.referenceSelect ? 1 : 0);                            \
    DO_FIELD(DW6, McompFilterType, params.interpFilter);                                                     \
    DO_FIELD(DW6, MotionModeSwitchableFlag, params.motionModeSwitchable);                                    \
    DO_FIELD(DW6, UseReferenceFrameMvSetFlag, params.useReferenceFrameMvSet);                                \
    DO_FIELD(DW6, ReferenceFrameSignBiasI0To7, params.refFrameBiasFlag);                                     \
    DO_FIELD(DW6, CurrentFrameOrderHint, params.currentOrderHint);                                           \
                                                                                                             \
    DO_FIELD(DW7, ReducedTxSetUsed, params.reducedTxSetUsed ? 1 : 0);                                        \
    DO_FIELD(DW7, FrameTransformMode, params.txMode);                                                        \
    DO_FIELD(DW7, SkipModePresentFlag, params.skipModePresent ? 1 : 0);                                      \
    DO_FIELD(DW7, SkipModeFrame0, params.skipModeFrame[0]);                                                  \
    DO_FIELD(DW7, SkipModeFrame1, params.skipModeFrame[1]);                                                  \
    DO_FIELD(DW7, ReferenceFrameSideI0To7, params.refFrameSide);                                             \
                                                                                                             \
    DO_FIELD(DW8, GlobalMotionType1, params.globalMotionType[0]);                                            \
    DO_FIELD(DW8, GlobalMotionType2, params.globalMotionType[1]);                                            \
    DO_FIELD(DW8, GlobalMotionType3, params.globalMotionType[2]);                                            \
    DO_FIELD(DW8, GlobalMotionType4, params.globalMotionType[3]);                                            \
    DO_FIELD(DW8, GlobalMotionType5, params.globalMotionType[4]);                                            \
    DO_FIELD(DW8, GlobalMotionType6, params.globalMotionType[5]);                                            \
    DO_FIELD(DW8, GlobalMotionType7, params.globalMotionType[6]);                                            \
    DO_FIELD(DW8, FrameLevelGlobalMotionInvalidFlags, params.frameLevelGlobalMotionInvalidFlags);            \
                                                                                                             \
    DO_FIELD(DW30, ReferenceFrameIdx0, params.refFrameIdx[intraFrame]);                                      \
    DO_FIELD(DW30, ReferenceFrameIdx1, params.refFrameIdx[lastFrame]);                                       \
    DO_FIELD(DW30, ReferenceFrameIdx2, params.refFrameIdx[last2Frame]);                                      \
    DO_FIELD(DW30, ReferenceFrameIdx3, params.refFrameIdx[last3Frame]);                                      \
    DO_FIELD(DW30, ReferenceFrameIdx4, params.refFrameIdx[goldenFrame]);                                     \
    DO_FIELD(DW30, ReferenceFrameIdx5, params.refFrameIdx[bwdRefFrame]);                                     \
    DO_FIELD(DW30, ReferenceFrameIdx6, params.refFrameIdx[altRef2Frame]);                                    \
    DO_FIELD(DW30, ReferenceFrameIdx7, params.refFrameIdx[altRefFrame]);                                     \
                                                                                                             \
    DO_FIELD(DW31, Value, params.refFrameRes[intraFrame]);                                                   \
                                                                                                             \
    DO_FIELD(DW32, Value, params.refFrameRes[lastFrame]);                                                    \
                                                                                                             \
    DO_FIELD(DW33, Value, params.refFrameRes[last2Frame]);                                                   \
                                                                                                             \
    DO_FIELD(DW34, Value, params.refFrameRes[last3Frame]);                                                   \
                                                                                                             \
    DO_FIELD(DW35, Value, params.refFrameRes[goldenFrame]);                                                  \
                                                                                                             \
    DO_FIELD(DW36, Value, params.refFrameRes[bwdRefFrame]);                                                  \
                                                                                                             \
    DO_FIELD(DW37, Value, params.refFrameRes[altRef2Frame]);                                                 \
                                                                                                             \
    DO_FIELD(DW38, Value, params.refFrameRes[altRefFrame]);                                                  \
                                                                                                             \
    DO_FIELD(DW39, Value, params.refScaleFactor[intraFrame]);                                                \
                                                                                                             \
    DO_FIELD(DW40, Value, params.refScaleFactor[lastFrame]);                                                 \
                                                                                                             \
    DO_FIELD(DW41, Value, params.refScaleFactor[last2Frame]);                                                \
                                                                                                             \
    DO_FIELD(DW42, Value, params.refScaleFactor[last3Frame]);                                                \
                                                                                                             \
    DO_FIELD(DW43, Value, params.refScaleFactor[goldenFrame]);                                               \
                                                                                                             \
    DO_FIELD(DW44, Value, params.refScaleFactor[bwdRefFrame]);                                               \
                                                                                                             \
    DO_FIELD(DW45, Value, params.refScaleFactor[altRef2Frame]);                                              \
                                                                                                             \
    DO_FIELD(DW46, Value, params.refScaleFactor[altRefFrame]);                                               \
                                                                                                             \
    DO_FIELD(DW47, ReferenceFrameOrderHint0ForIntraFrame, params.refOrderHints[intraFrame]);                 \
    DO_FIELD(DW47, ReferenceFrameOrderHint1ForLastFrame, params.refOrderHints[lastFrame]);                   \
    DO_FIELD(DW47, ReferenceFrameOrderHint2ForLast2Frame, params.refOrderHints[last2Frame]);                 \
    DO_FIELD(DW47, ReferenceFrameOrderHint3ForLast3Frame, params.refOrderHints[last3Frame]);                 \
                                                                                                             \
    DO_FIELD(DW48, ReferenceFrameOrderHint4ForGoldenFrame, params.refOrderHints[goldenFrame]);               \
    DO_FIELD(DW48, ReferenceFrameOrderHint5ForBwdrefFrame, params.refOrderHints[bwdRefFrame]);               \
    DO_FIELD(DW48, ReferenceFrameOrderHint6ForAltref2Frame, params.refOrderHints[altRef2Frame]);             \
    DO_FIELD(DW48, ReferenceFrameOrderHint7ForAltrefFrame, params.refOrderHints[altRefFrame]);               \
                                                                                                             \
    DO_FIELD(DW51, Nonfirstpassflag, params.notFirstPass ? 1 : 0);                                           \
    DO_FIELD(DW51, VdencPakOnlyPass, params.vdencPackOnlyPass ? 1 : 0);                                      \
    DO_FIELD(DW51, FrameszoverstatusenFramebitratemaxreportmask, params.frameBitRateMaxReportMask ? 1 : 0);  \
    DO_FIELD(DW51, FrameszunderstatusenFramebitrateminreportmask, params.frameBitRateMinReportMask ? 1 : 0); \
                                                                                                             \
    DO_FIELD(DW52, Framebitratemax, params.frameBitRateMax);                                                 \
    DO_FIELD(DW52, Framebitratemaxunit, params.frameBitRateMaxUnit);                                         \
                                                                                                             \
    DO_FIELD(DW53, Framebitratemin, params.frameBitRateMin);                                                 \
    DO_FIELD(DW53, Framebitrateminunit, params.frameBitRateMinUnit);                                         \
                                                                                                             \
    DO_FIELD(DW54_55, Value[0], params.frameDeltaQindexMax[0]);                                              \
    DO_FIELD(DW54_55, Value[1], params.frameDeltaQindexMax[1]);                                              \
                                                                                                             \
    DO_FIELD(DW56, Framedeltaqindexmin, params.frameDeltaQindexMin);                                         \
                                                                                                             \
    DO_FIELD(DW57_58, Value[0], params.frameDeltaLFMax[0]);                                                  \
    DO_FIELD(DW57_58, Value[1], params.frameDeltaLFMax[1]);                                                  \
                                                                                                             \
    DO_FIELD(DW59, Framedeltalfmin, params.frameDeltaLFMin);                                                 \
                                                                                                             \
    DO_FIELD(DW60_61, Value[0], params.frameDeltaQindexLFMaxRange[0]);                                       \
    DO_FIELD(DW60_61, Value[1], params.frameDeltaQindexLFMaxRange[1]);                                       \
                                                                                                             \
    DO_FIELD(DW62, Framedeltaqindexlfminrange, params.frameDeltaQindexLFMinRange);                           \
                                                                                                             \
    DO_FIELD(DW63, Minframsize, params.minFramSize);                                                         \
    DO_FIELD(DW63, Minframesizeunits, params.minFramSizeUnits);                                              \
                                                                                                             \
    DO_FIELD(DW65, Class0SseThreshold0, params.class0_SSE_Threshold0);                                       \
    DO_FIELD(DW65, Class0SseThreshold1, params.class0_SSE_Threshold1);                                       \
                                                                                                             \
    DO_FIELD(DW74, Rdmult, params.rdmult);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_INLOOP_FILTER_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_INLOOP_FILTER_STATE);

#define DO_FIELDS()                                                                                             \
    DO_FIELD(DW1, LumaYDeblockerFilterLevelVertical, params.loopFilterLevel[0]);                                \
    DO_FIELD(DW1, LumaYDeblockerFilterLevelHorizontal, params.loopFilterLevel[1]);                              \
    DO_FIELD(DW1, ChromaUDeblockerFilterLevel, params.loopFilterLevel[2]);                                      \
    DO_FIELD(DW1, ChromaVDeblockerFilterLevel, params.loopFilterLevel[3]);                                      \
    DO_FIELD(DW1, DeblockerFilterSharpnessLevel, params.loopFilterSharpness);                                   \
    DO_FIELD(DW1, DeblockerFilterModeRefDeltaEnableFlag, params.loopFilterDeltaEnabled);                        \
    DO_FIELD(DW1, DeblockerDeltaLfResolution, params.deltaLfRes);                                               \
    DO_FIELD(DW1, DeblockerFilterDeltaLfMultiFlag, params.deltaLfMulti);                                        \
    DO_FIELD(DW1, DeblockerFilterDeltaLfPresentFlag, params.loopFilterDeltaUpdate);                             \
                                                                                                                \
    DO_FIELD(DW2, DeblockerFilterRefDeltas0, params.loopFilterRefDeltas[0]);                                    \
    DO_FIELD(DW2, DeblockerFilterRefDeltas1, params.loopFilterRefDeltas[1]);                                    \
    DO_FIELD(DW2, DeblockerFilterRefDeltas2, params.loopFilterRefDeltas[2]);                                    \
    DO_FIELD(DW2, DeblockerFilterRefDeltas3, params.loopFilterRefDeltas[3]);                                    \
                                                                                                                \
    DO_FIELD(DW3, DeblockerFilterRefDeltas4, params.loopFilterRefDeltas[4]);                                    \
    DO_FIELD(DW3, DeblockerFilterRefDeltas5, params.loopFilterRefDeltas[5]);                                    \
    DO_FIELD(DW3, DeblockerFilterRefDeltas6, params.loopFilterRefDeltas[6]);                                    \
    DO_FIELD(DW3, DeblockerFilterRefDeltas7, params.loopFilterRefDeltas[7]);                                    \
                                                                                                                \
    DO_FIELD(DW4, DeblockerFilterModeDeltas0, params.loopFilterModeDeltas[0]);                                  \
    DO_FIELD(DW4, DeblockerFilterModeDeltas1, params.loopFilterModeDeltas[1]);                                  \
                                                                                                                \
    DO_FIELD(DW5, CdefYStrength0, params.cdefYStrength[0]);                                                     \
    DO_FIELD(DW5, CdefYStrength1, params.cdefYStrength[1]);                                                     \
    DO_FIELD(DW5, CdefYStrength2, params.cdefYStrength[2]);                                                     \
    DO_FIELD(DW5, CdefYStrength3, params.cdefYStrength[3]);                                                     \
    DO_FIELD(DW5, CdefBits, params.cdefBits);                                                                   \
    DO_FIELD(DW5, CdefFilterDampingFactorMinus3, params.cdefDampingMinus3);                                     \
                                                                                                                \
    DO_FIELD(DW6, CdefYStrength4, params.cdefYStrength[4]);                                                     \
    DO_FIELD(DW6, CdefYStrength5, params.cdefYStrength[5]);                                                     \
    DO_FIELD(DW6, CdefYStrength6, params.cdefYStrength[6]);                                                     \
    DO_FIELD(DW6, CdefYStrength7, params.cdefYStrength[7]);                                                     \
                                                                                                                \
    DO_FIELD(DW7, CdefUvStrength0, params.cdefUVStrength[0]);                                                   \
    DO_FIELD(DW7, CdefUvStrength1, params.cdefUVStrength[1]);                                                   \
    DO_FIELD(DW7, CdefUvStrength2, params.cdefUVStrength[2]);                                                   \
    DO_FIELD(DW7, CdefUvStrength3, params.cdefUVStrength[3]);                                                   \
                                                                                                                \
    DO_FIELD(DW8, CdefUvStrength4, params.cdefUVStrength[4]);                                                   \
    DO_FIELD(DW8, CdefUvStrength5, params.cdefUVStrength[5]);                                                   \
    DO_FIELD(DW8, CdefUvStrength6, params.cdefUVStrength[6]);                                                   \
    DO_FIELD(DW8, CdefUvStrength7, params.cdefUVStrength[7]);                                                   \
                                                                                                                \
    DO_FIELD(DW9, SuperResUpscaledFrameWidthMinus1, params.superresUpscaledWidthMinus1);                        \
    DO_FIELD(DW9, SuperResDenom, params.superresDenom);                                                         \
                                                                                                                \
    DO_FIELD(DW10, FrameLoopRestorationFilterTypeForLumaY, params.LoopRestorationType[0]);                      \
    DO_FIELD(DW10, FrameLoopRestorationFilterTypeForChromaU, params.LoopRestorationType[1]);                    \
    DO_FIELD(DW10, FrameLoopRestorationFilterTypeForChromaV, params.LoopRestorationType[2]);                    \
    DO_FIELD(DW10, LoopRestorationUnitSizeForLumaY, params.LoopRestorationSizeLuma);                            \
    DO_FIELD(DW10, UseSameLoopRestorationUnitSizeForChromasUvFlag, params.UseSameLoopRestorationSizeForChroma); \
                                                                                                                \
    DO_FIELD(DW11, LumaPlaneXStepQn, params.lumaPlaneXStepQn);                                                  \
                                                                                                                \
    DO_FIELD(DW12, LumaPlaneX0Qn, params.lumaPlaneX0Qn);                                                        \
                                                                                                                \
    DO_FIELD(DW13, ChromaPlaneXStepQn, params.chromaPlaneXStepQn);                                              \
                                                                                                                \
    DO_FIELD(DW14, ChromaPlaneX0Qn, params.chromaPlaneX0Qn)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_TILE_CODING)
    {
        _MHW_SETCMD_CALLBASE(AVP_TILE_CODING);

#define DO_FIELDS()                                                                             \
    DO_FIELD(DW1, FrameTileId, params.tileId);                                                  \
    DO_FIELD(DW1, TgTileNum, params.tgTileNum);                                                   \
    DO_FIELD(DW1, TileGroupId, params.tileGroupId);                                             \
                                                                                                \
    DO_FIELD(DW2, TileColumnPositionInSbUnit, params.tileColPositionInSb);                      \
    DO_FIELD(DW2, TileRowPositionInSbUnit, params.tileRowPositionInSb);                         \
                                                                                                \
    DO_FIELD(DW3, TileWidthInSuperblockUnitMinus1, params.tileWidthInSbMinus1);                 \
    DO_FIELD(DW3, TileHeightInSuperblockUnitMinus1, params.tileHeightInSbMinus1);               \
                                                                                                \
    DO_FIELD(DW4, FirstTileInAFrame, params.firstTileInAFrame ? 1 : 0);                         \
    DO_FIELD(DW4, IslasttileofcolumnFlag, params.lastTileOfColumn ? 1 : 0);                     \
    DO_FIELD(DW4, IslasttileofrowFlag, params.lastTileOfRow ? 1 : 0);                           \
    DO_FIELD(DW4, IsstarttileoftilegroupFlag, params.firstTileOfTileGroup ? 1 : 0);             \
    DO_FIELD(DW4, IsendtileoftilegroupFlag, params.lastTileOfTileGroup ? 1 : 0);                \
    DO_FIELD(DW4, IslasttileofframeFlag, params.lastTileOfFrame ? 1 : 0);                       \
    DO_FIELD(DW4, DisableCdfUpdateFlag, params.disableCdfUpdateFlag ? 1 : 0);                   \
    DO_FIELD(DW4, DisableFrameContextUpdateFlag, params.disableFrameContextUpdateFlag ? 1 : 0); \
                                                                                                \
    DO_FIELD(DW5, NumberOfActiveBePipes, params.numOfActiveBePipes);                            \
    DO_FIELD(DW5, NumOfTileColumnsMinus1InAFrame, params.numOfTileColumnsInFrame - 1);          \
    DO_FIELD(DW5, NumOfTileRowsMinus1InAFrame, params.numOfTileRowsInFrame - 1);                \
                                                                                                \
    DO_FIELD(DW6, OutputDecodedTileColumnPositionInSbUnit, params.outputDecodedTileColPos);     \
    DO_FIELD(DW6, OutputDecodedTileRowPositionInSbUnit, params.outputDecodedTileRowPos)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_SEGMENT_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_SEGMENT_STATE);

        const uint8_t seg = params.currentSegmentId;

#define DO_FIELDS()                                                                                                      \
    DO_FIELD(DW1, SegmentId, seg);                                                                                       \
                                                                                                                         \
    DO_FIELD(DW2, SegmentFeatureMask, params.av1SegmentParams.m_featureMask[seg]);                                       \
    DO_FIELD(DW2, SegmentDeltaQindex, params.av1SegmentParams.m_featureData[seg][segLvlAltQ]);                           \
    DO_FIELD(DW2, SegmentBlockSkipFlag, params.av1SegmentParams.m_featureData[seg][segLvlSkip]);                         \
    DO_FIELD(DW2, SegmentBlockGlobalmvFlag, params.av1SegmentParams.m_featureData[seg][segLvlGlobalMv]);                 \
    DO_FIELD(DW2, SegmentLosslessFlag, params.av1SegmentParams.m_losslessFlag[seg]);                                     \
    DO_FIELD(DW2, SegmentLumaYQmLevel, params.av1SegmentParams.m_qmLevelY[seg]);                                         \
    DO_FIELD(DW2, SegmentChromaUQmLevel, params.av1SegmentParams.m_qmLevelU[seg]);                                       \
    DO_FIELD(DW2, SegmentChromaVQmLevel, params.av1SegmentParams.m_qmLevelV[seg]);                                       \
                                                                                                                         \
    DO_FIELD(DW3, SegmentDeltaLoopFilterLevelLumaVertical, params.av1SegmentParams.m_featureData[seg][segLvlAltLfYv]);   \
    DO_FIELD(DW3, SegmentDeltaLoopFilterLevelLumaHorizontal, params.av1SegmentParams.m_featureData[seg][segLvlAltLfYh]); \
    DO_FIELD(DW3, SegmentDeltaLoopFilterLevelChromaU, params.av1SegmentParams.m_featureData[seg][segLvlAltLfU]);         \
    DO_FIELD(DW3, SegmentDeltaLoopFilterLevelChromaV, params.av1SegmentParams.m_featureData[seg][segLvlAltLfV]);         \
    DO_FIELD(DW3, SegmentReferenceFrame, params.av1SegmentParams.m_featureData[seg][segLvlRefFrame])

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        // Reference Picture Base Address. Only one control DW17 for all references

        bool firstRefPic = true;

        // m_references[8] follows the order INTRA_FRAME->LAST_FRAME->LAST2_FRAME->LAST3_FRAME->GOLDEN_FRAME->
        // BWDREF_FRAME-.ALTREF2_FRAME->ALTREF_FRAME.
        for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
        {
            // Reference Picture Buffer
            if (!Mos_ResourceIsNull(params.refs[i]))
            {
                MOS_SURFACE details = {};
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refs[i], &details));

                if (firstRefPic)
                {
                    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStatePreDeblock);
                    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStatePreDeblock);
                    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.TileMode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
                    firstRefPic                                                                          = false;
                }

                resourceParams.presResource       = params.refs[i];
                resourceParams.pdwCmd             = (cmd.ReferenceFrameBufferBaseAddressRefaddr07[i].DW0_1.Value);
                resourceParams.dwOffset           = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd    = _MHW_CMD_DW_LOCATION(ReferenceFrameBufferBaseAddressRefaddr07[i]);
                resourceParams.bIsWritable        = false;
                resourceParams.dwSharedMocsOffset = 17 - resourceParams.dwLocationInCmd;

                InitMocsParams(resourceParams, &cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        //Decoded Output Frame Buffer
        cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStatePreDeblock);
        cmd.DecodedOutputFrameBufferAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStatePreDeblock);
        cmd.DecodedOutputFrameBufferAddressAttributes.DW0.TileMode                           = GetHwTileType(params.decodedPic->TileType, params.decodedPic->TileModeGMM, params.decodedPic->bGMMTileEnabled);

        resourceParams.presResource    = &(params.decodedPic->OsResource);
        resourceParams.dwOffset        = params.decodedPic->dwOffset;
        resourceParams.pdwCmd          = (cmd.DecodedOutputFrameBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DecodedOutputFrameBufferAddress);
        resourceParams.bIsWritable     = true;

        InitMocsParams(resourceParams, &cmd.DecodedOutputFrameBufferAddressAttributes.DW0.Value, 1, 6);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        //IntraBC Decoded Output Frame buffer
        if (!Mos_ResourceIsNull(params.intrabcDecodedOutputFrameBuffer))
        {
            resourceParams.presResource    = params.intrabcDecodedOutputFrameBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.IntrabcDecodedOutputFrameBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(IntrabcDecodedOutputFrameBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            //This surface should not have memory compression turned on
            cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = 0;
            cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.CompressionType                    = 0;
            cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.TileMode                           = GetHwTileType(params.intrabcDecodedOutputFrameBuffer->TileType, params.intrabcDecodedOutputFrameBuffer->TileModeGMM, params.intrabcDecodedOutputFrameBuffer->bGMMTileEnabled);
        }

        // CDF Table Initialization Buffer
        if (!Mos_ResourceIsNull(params.cdfTableInitBuffer))
        {
            resourceParams.presResource    = params.cdfTableInitBuffer;
            resourceParams.dwOffset        = params.cdfTableInitBufferOffset;
            resourceParams.pdwCmd          = (cmd.CdfTablesInitializationBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdfTablesInitializationBufferAddress);
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.CdfTablesInitializationBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.cdfTableBwdAdaptBuffer))
        {
            resourceParams.presResource    = params.cdfTableBwdAdaptBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdfTablesBackwardAdaptationBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdfTablesBackwardAdaptationBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdfTablesBackwardAdaptationBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        //resourceParams.dwSharedMocsOffset = 0;

        if (!Mos_ResourceIsNull(params.segmentIdReadBuffer))
        {
            resourceParams.presResource    = params.segmentIdReadBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.Av1SegmentIdReadBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Av1SegmentIdReadBufferAddress);
            resourceParams.bIsWritable     = true;

            resourceParams.dwSharedMocsOffset = 35 - resourceParams.dwLocationInCmd;

            InitMocsParams(resourceParams, &cmd.Av1SegmentIdReadBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // AV1 Segment Id Write Buffer
        if (!Mos_ResourceIsNull(params.segmentIdWriteBuffer))
        {
            resourceParams.presResource    = params.segmentIdWriteBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.Av1SegmentIdWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Av1SegmentIdWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            resourceParams.dwSharedMocsOffset = 38 - resourceParams.dwLocationInCmd;

            InitMocsParams(resourceParams, &cmd.Av1SegmentIdWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        //Collocated MV Temporal buffers
        for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
        {
            if (!Mos_ResourceIsNull(params.colMvTempBuffer[i]))
            {
                resourceParams.presResource       = params.colMvTempBuffer[i];
                resourceParams.dwOffset           = 0;
                resourceParams.pdwCmd             = (cmd.CollocatedMotionVectorTemporalBufferBaseAddressTmvaddr07[i].DW0_1.Value);
                resourceParams.dwLocationInCmd    = _MHW_CMD_DW_LOCATION(CollocatedMotionVectorTemporalBufferBaseAddressTmvaddr07[i]);
                resourceParams.bIsWritable        = true;
                resourceParams.dwSharedMocsOffset = 55 - resourceParams.dwLocationInCmd;

                InitMocsParams(resourceParams, &cmd.CollocatedMotionVectorTemporalBufferBaseAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        // Current Motion Vector Temporal Buffer
        if (!Mos_ResourceIsNull(params.curMvTempBuffer))
        {
            resourceParams.presResource    = params.curMvTempBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CurrentFrameMotionVectorWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CurrentFrameMotionVectorWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CurrentFrameMotionVectorWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;
        // Bitstream Decode Line Rowstore Buffer
        if (m_btdlRowstoreCache.enabled)
        {
            cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress.DW0_1.BaseAddress                                         = m_btdlRowstoreCache.dwAddress;
        }
        else if (!Mos_ResourceIsNull(params.bsLineRowstoreBuffer))
        {
            resourceParams.presResource    = params.bsLineRowstoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Bitstream Decode Tile Line Rowstore Buffer
        if (!Mos_ResourceIsNull(params.bsTileLineRowstoreBuffer))
        {
            resourceParams.presResource    = params.bsTileLineRowstoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;
        // Intra Prediction Line Rowstore Read/Write Buffer
        if (m_ipdlRowstoreCache.enabled)
        {
            cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.IntraPredictionLineRowstoreReadWriteBufferAddress.DW0_1.BaseAddress                                         = m_ipdlRowstoreCache.dwAddress;
        }
        else if (!Mos_ResourceIsNull(params.intraPredLineRowstoreBuffer))
        {
            resourceParams.presResource    = params.intraPredLineRowstoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.IntraPredictionLineRowstoreReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(IntraPredictionLineRowstoreReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.intraPredTileLineRowstoreBuffer))
        {
            resourceParams.presResource    = params.intraPredTileLineRowstoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(IntraPredictionTileLineRowstoreReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Spatial Motion Vector Line Buffer
        if (m_smvlRowstoreCache.enabled)
        {
            cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.SpatialMotionVectorLineReadWriteBufferAddress.DW0_1.BaseAddress                                         = m_smvlRowstoreCache.dwAddress;
        }
        else if (!Mos_ResourceIsNull(params.spatialMVLineBuffer))
        {
            resourceParams.presResource    = params.spatialMVLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SpatialMotionVectorLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SpatialMotionVectorLineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Spatial Motion Vector Tile Line Buffer
        if (!Mos_ResourceIsNull(params.spatialMVCodingTileLineBuffer))
        {
            resourceParams.presResource    = params.spatialMVCodingTileLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SpatialMotionVectorCodingTileLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SpatialMotionVectorCodingTileLineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SpatialMotionVectorTileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        //Loop Restoration Meta Tile Column Read/Write Buffer
        if (!Mos_ResourceIsNull(params.lrMetaTileColumnBuffer))
        {
            resourceParams.presResource    = params.lrMetaTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationMetaTileColumnReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationMetaTileColumnReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationMetaTileColumnReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        //Deblocker Filter Control Parameters Line Read Write Buffer
        if (!Mos_ResourceIsNull(params.lrTileLineYBuffer))
        {
            resourceParams.presResource    = params.lrTileLineYBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileReadWriteLineYBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileReadWriteLineYBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileReadWriteLineYBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        //Deblocker Filter Control Parameters Tile Line Read Write Buffer
        if (!Mos_ResourceIsNull(params.lrTileLineUBuffer))
        {
            resourceParams.presResource    = params.lrTileLineUBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileReadWriteLineUBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileReadWriteLineUBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileReadWriteLineUBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        //Deblocker Filter Control Parameters Tile Column Read Write Buffer
        if (!Mos_ResourceIsNull(params.lrTileLineVBuffer))
        {
            resourceParams.presResource    = params.lrTileLineVBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileReadWriteLineVBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileReadWriteLineVBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileReadWriteLineVBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Line Read Write Y Buffer
        if (m_dflyRowstoreCache.enabled)
        {
            cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.DeblockerFilterLineReadWriteYBufferAddress.DW0_1.BaseAddress                                         = m_dflyRowstoreCache.dwAddress;
        }
        else if (!Mos_ResourceIsNull(params.deblockLineYBuffer))
        {
            resourceParams.presResource    = params.deblockLineYBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterLineReadWriteYBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterLineReadWriteYBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Line Read Write U Buffer
        if (m_dfluRowstoreCache.enabled)
        {
            cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.DeblockerFilterLineReadWriteUBufferAddress.DW0_1.BaseAddress                                         = m_dfluRowstoreCache.dwAddress;
        }
        else if(!Mos_ResourceIsNull(params.deblockLineUBuffer))
        {
            resourceParams.presResource    = params.deblockLineUBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterLineReadWriteUBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterLineReadWriteUBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }
        // Deblocker Filter Line Read Write V Buffer
        if (m_dflvRowstoreCache.enabled)
        {
            cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.DeblockerFilterLineReadWriteVBufferAddress.DW0_1.BaseAddress                                         = m_dflvRowstoreCache.dwAddress;
        }
        else if (!Mos_ResourceIsNull(params.deblockLineVBuffer))
        {
            resourceParams.presResource    = params.deblockLineVBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterLineReadWriteVBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterLineReadWriteVBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Tile Line Read Write Y Buffer
        if (!Mos_ResourceIsNull(params.deblockTileLineYBuffer))
        {
            resourceParams.presResource    = params.deblockTileLineYBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterTileLineReadWriteYBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterTileLineReadWriteYBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterTileLineReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Tile Line Read Write V Buffer
        if (!Mos_ResourceIsNull(params.deblockTileLineVBuffer))
        {
            resourceParams.presResource    = params.deblockTileLineVBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterTileLineReadWriteVBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterTileLineReadWriteVBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterTileLineReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Tile Line Read Write U Buffer
        if (!Mos_ResourceIsNull(params.deblockTileLineUBuffer))
        {
            resourceParams.presResource    = params.deblockTileLineUBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterTileLineReadWriteUBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterTileLineReadWriteUBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterTileLineReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Tile Column Read Write Y Buffer
        if (!Mos_ResourceIsNull(params.deblockTileColumnYBuffer))
        {
            resourceParams.presResource    = params.deblockTileColumnYBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterTileColumnReadWriteYBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterTileColumnReadWriteYBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterTileColumnReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Tile Column Read Write U Buffer
        if (!Mos_ResourceIsNull(params.deblockTileColumnUBuffer))
        {
            resourceParams.presResource    = params.deblockTileColumnUBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterTileColumnReadWriteUBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterTileColumnReadWriteUBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterTileColumnReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Deblocker Filter Tile Column Read Write V Buffer
        if (!Mos_ResourceIsNull(params.deblockTileColumnVBuffer))
        {
            resourceParams.presResource    = params.deblockTileColumnVBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DeblockerFilterTileColumnReadWriteVBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DeblockerFilterTileColumnReadWriteVBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DeblockerFilterTileColumnReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Cdef Filter Line Read Write Y Buffer
        if (m_cdefRowstoreCache.enabled)
        {
            cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.CdefFilterLineReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.CdefFilterLineReadWriteBufferAddress.DW0_1.BaseAddress                                         = m_cdefRowstoreCache.dwAddress;
        }
        else if (!Mos_ResourceIsNull(params.cdefLineBuffer))
        {
            resourceParams.presResource    = params.cdefLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdefFilterLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdefFilterLineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Cdef Filter Tile Line Read Write Y Buffer
        if (!Mos_ResourceIsNull(params.cdefTileLineBuffer))
        {
            resourceParams.presResource    = params.cdefTileLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdefFilterTileLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdefFilterTileLineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdefFilterTileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Cdef Filter Tile Line Read Write U Buffer
        if (!Mos_ResourceIsNull(params.cdefTileColumnBuffer))
        {
            resourceParams.presResource    = params.cdefTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdefFilterTileColumnReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdefFilterTileColumnReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdefFilterTileColumnReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Cdef Filter Tile Line Read Write V Buffer
        if (!Mos_ResourceIsNull(params.cdefMetaTileLineBuffer))
        {
            resourceParams.presResource    = params.cdefMetaTileLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdefFilterMetaTileLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdefFilterMetaTileLineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdefFilterMetaTileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Cdef Filter Tile Column Read Write Y Buffer
        if (!Mos_ResourceIsNull(params.cdefMetaTileColumnBuffer))
        {
            resourceParams.presResource    = params.cdefMetaTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdefFilterMetaTileColumnReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdefFilterMetaTileColumnReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdefFilterMetaTileColumnReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Cdef Filter Top Left Corner Read Write Buffer
        if (!Mos_ResourceIsNull(params.cdefTopLeftCornerBuffer))
        {
            resourceParams.presResource    = params.cdefTopLeftCornerBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CdefFilterTopLeftCornerReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CdefFilterTopLeftCornerReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CdefFilterTopLeftCornerReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Super-Res Tile Column Read Write Y Buffer
        if (!Mos_ResourceIsNull(params.superResTileColumnYBuffer))
        {
            resourceParams.presResource    = params.superResTileColumnYBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SuperResTileColumnReadWriteYBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SuperResTileColumnReadWriteYBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SuperResTileColumnReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Super-Res Tile Column Read Write U Buffer
        if (!Mos_ResourceIsNull(params.superResTileColumnUBuffer))
        {
            resourceParams.presResource    = params.superResTileColumnUBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SuperResTileColumnReadWriteUBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SuperResTileColumnReadWriteUBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SuperResTileColumnReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Super-Res Tile Column Read Write V Buffer
        if (!Mos_ResourceIsNull(params.superResTileColumnVBuffer))
        {
            resourceParams.presResource    = params.superResTileColumnVBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SuperResTileColumnReadWriteVBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SuperResTileColumnReadWriteVBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SuperResTileColumnReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Loop Restoration Filter Tile Column Read Write Y Buffer
        if (!Mos_ResourceIsNull(params.lrTileColumnYBuffer))
        {
            resourceParams.presResource    = params.lrTileColumnYBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileColumnReadWriteYBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Loop Restoration Filter Tile Column Read Write U Buffer
        if (!Mos_ResourceIsNull(params.lrTileColumnUBuffer))
        {
            resourceParams.presResource    = params.lrTileColumnUBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileColumnReadWriteUBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Loop Restoration Filter Tile Column Read Write V Buffer
        if (!Mos_ResourceIsNull(params.lrTileColumnVBuffer))
        {
            resourceParams.presResource    = params.lrTileColumnVBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileColumnReadWriteVBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Decoded Frame Status Error Buffer
        if (!Mos_ResourceIsNull(params.decodedFrameStatusErrorBuffer))
        {
            resourceParams.presResource    = params.decodedFrameStatusErrorBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DecodedFrameStatusErrorBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DecodedFrameStatusErrorBufferBaseAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DecodedFrameStatusErrorBufferBaseAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Decoded Block Data Streamout Buffer
        if (!Mos_ResourceIsNull(params.decodedBlockDataStreamoutBuffer))
        {
            resourceParams.presResource    = params.decodedBlockDataStreamoutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DecodedBlockDataStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DecodedBlockDataStreamoutBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.DecodedBlockDataStreamoutBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // below is encode part

        // Original Uncompressed Picture Source Buffer
        if (!Mos_ResourceIsNull(params.originalPicSourceBuffer))
        {
            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.originalPicSourceBuffer, &details));

            cmd.OriginalUncompressedPictureSourceBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStateRawSurf);
            cmd.OriginalUncompressedPictureSourceBufferAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStateRawSurf);
            cmd.OriginalUncompressedPictureSourceBufferAddressAttributes.DW0.TileMode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

            resourceParams.presResource    = params.originalPicSourceBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.OriginalUncompressedPictureSourceBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(OriginalUncompressedPictureSourceBufferAddress);
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.OriginalUncompressedPictureSourceBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Downscaled Uncompressed Picture Source Buffer
        if (!Mos_ResourceIsNull(params.dsPictureSourceBuffer))
        {
            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.dsPictureSourceBuffer, &details));

            cmd.DownscaledUncompressedPictureSourceBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStateRawSurf);
            cmd.DownscaledUncompressedPictureSourceBufferAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStateRawSurf);
            cmd.DownscaledUncompressedPictureSourceBufferAddressAttributes.DW0.TileMode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

            resourceParams.presResource    = params.dsPictureSourceBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.DownscaledUncompressedPictureSourceBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DownscaledUncompressedPictureSourceBufferAddress);
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.DownscaledUncompressedPictureSourceBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Tile Size Streamout Buffer

        if (!Mos_ResourceIsNull(params.tileSizeStreamoutBuffer))
        {
            resourceParams.presResource    = params.tileSizeStreamoutBuffer;
            resourceParams.dwOffset        = params.tileSizeStreamoutBufferOffset;
            resourceParams.pdwCmd          = (cmd.TileSizeStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(TileSizeStreamoutBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.TileSizeStreamoutBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // Tile Statistics Streamout Buffer
        if (!Mos_ResourceIsNull(params.tileStatisticsPakStreamoutBuffer))
        {
            resourceParams.presResource    = params.tileStatisticsPakStreamoutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.TileStatisticsStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(TileStatisticsStreamoutBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.TileStatisticsStreamoutBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // CU Streamout Buffer
        if (!Mos_ResourceIsNull(params.cuStreamoutBuffer))
        {
            resourceParams.presResource    = params.cuStreamoutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CUStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(CUStreamoutBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CUStreamoutBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SSE Line Read / Write Buffer
        if (!Mos_ResourceIsNull(params.sseLineBuffer))
        {
            resourceParams.presResource    = params.sseLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SSELineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SSELineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SSELineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SSE Tile Line Read/Write Buffer
        if (!Mos_ResourceIsNull(params.sseTileLineBuffer))
        {
            resourceParams.presResource    = params.sseTileLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SSETileLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SSETileLineReadWriteBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.SSETileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // PostCDEF pixels Buffer
        if (params.postCDEFpixelsBuffer != nullptr)
        {
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable                   = MmcEnabled(params.postCdefSurfMmcState);
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.CompressionType                                      = MmcRcEnabled(params.postCdefSurfMmcState);
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.TileMode                                             = GetHwTileType(params.postCDEFpixelsBuffer->TileType,
                params.postCDEFpixelsBuffer->TileModeGMM,
                params.postCDEFpixelsBuffer->bGMMTileEnabled);

            resourceParams.presResource    = &params.postCDEFpixelsBuffer->OsResource;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.PostCDEFpixelsBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(PostCDEFpixelsBufferAddress);
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.PostCDEFpixelsBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_INTER_PRED_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_INTER_PRED_STATE);

#define DO_FIELDS()                                                                      \
    DO_FIELD(DW1, SavedOrderHintsForAllReferences00, params.savedRefOrderHints[0][0]);   \
    DO_FIELD(DW1, SavedOrderHintsForAllReferences01, params.savedRefOrderHints[0][1]);   \
    DO_FIELD(DW1, SavedOrderHintsForAllReferences02, params.savedRefOrderHints[0][2]);   \
    DO_FIELD(DW1, SavedOrderHintsForAllReferences03, params.savedRefOrderHints[0][3]);   \
                                                                                         \
    DO_FIELD(DW2, SavedOrderHintsForAllReferences04, params.savedRefOrderHints[0][4]);   \
    DO_FIELD(DW2, SavedOrderHintsForAllReferences05, params.savedRefOrderHints[0][5]);   \
    DO_FIELD(DW2, SavedOrderHintsForAllReferences06, params.savedRefOrderHints[0][6]);   \
    DO_FIELD(DW2, ActiveReferenceBitmaskForMotionFieldProjection, params.refMaskMfProj); \
                                                                                         \
    DO_FIELD(DW3, SavedOrderHintsForAllReferences10, params.savedRefOrderHints[1][0]);   \
    DO_FIELD(DW3, SavedOrderHintsForAllReferences11, params.savedRefOrderHints[1][1]);   \
    DO_FIELD(DW3, SavedOrderHintsForAllReferences12, params.savedRefOrderHints[1][2]);   \
    DO_FIELD(DW3, SavedOrderHintsForAllReferences13, params.savedRefOrderHints[1][3]);   \
                                                                                         \
    DO_FIELD(DW4, SavedOrderHintsForAllReferences14, params.savedRefOrderHints[1][4]);   \
    DO_FIELD(DW4, SavedOrderHintsForAllReferences15, params.savedRefOrderHints[1][5]);   \
    DO_FIELD(DW4, SavedOrderHintsForAllReferences16, params.savedRefOrderHints[1][6]);   \
                                                                                         \
    DO_FIELD(DW5, SavedOrderHintsForAllReferences20, params.savedRefOrderHints[2][0]);   \
    DO_FIELD(DW5, SavedOrderHintsForAllReferences21, params.savedRefOrderHints[2][1]);   \
    DO_FIELD(DW5, SavedOrderHintsForAllReferences22, params.savedRefOrderHints[2][2]);   \
    DO_FIELD(DW5, SavedOrderHintsForAllReferences23, params.savedRefOrderHints[2][3]);   \
                                                                                         \
    DO_FIELD(DW6, SavedOrderHintsForAllReferences24, params.savedRefOrderHints[2][4]);   \
    DO_FIELD(DW6, SavedOrderHintsForAllReferences25, params.savedRefOrderHints[2][5]);   \
    DO_FIELD(DW6, SavedOrderHintsForAllReferences26, params.savedRefOrderHints[2][6]);   \
                                                                                         \
    DO_FIELD(DW7, SavedOrderHintsForAllReferences30, params.savedRefOrderHints[3][0]);   \
    DO_FIELD(DW7, SavedOrderHintsForAllReferences31, params.savedRefOrderHints[3][1]);   \
    DO_FIELD(DW7, SavedOrderHintsForAllReferences32, params.savedRefOrderHints[3][2]);   \
    DO_FIELD(DW7, SavedOrderHintsForAllReferences33, params.savedRefOrderHints[3][3]);   \
                                                                                         \
    DO_FIELD(DW8, SavedOrderHintsForAllReferences34, params.savedRefOrderHints[3][4]);   \
    DO_FIELD(DW8, SavedOrderHintsForAllReferences35, params.savedRefOrderHints[3][5]);   \
    DO_FIELD(DW8, SavedOrderHintsForAllReferences36, params.savedRefOrderHints[3][6]);   \
                                                                                         \
    DO_FIELD(DW9, SavedOrderHintsForAllReferences40, params.savedRefOrderHints[4][0]);   \
    DO_FIELD(DW9, SavedOrderHintsForAllReferences41, params.savedRefOrderHints[4][1]);   \
    DO_FIELD(DW9, SavedOrderHintsForAllReferences42, params.savedRefOrderHints[4][2]);   \
    DO_FIELD(DW9, SavedOrderHintsForAllReferences43, params.savedRefOrderHints[4][3]);   \
                                                                                         \
    DO_FIELD(DW10, SavedOrderHintsForAllReferences44, params.savedRefOrderHints[4][4]);  \
    DO_FIELD(DW10, SavedOrderHintsForAllReferences45, params.savedRefOrderHints[4][5]);  \
    DO_FIELD(DW10, SavedOrderHintsForAllReferences46, params.savedRefOrderHints[4][6]);  \
                                                                                         \
    DO_FIELD(DW11, SavedOrderHintsForAllReferences50, params.savedRefOrderHints[5][0]);  \
    DO_FIELD(DW11, SavedOrderHintsForAllReferences51, params.savedRefOrderHints[5][1]);  \
    DO_FIELD(DW11, SavedOrderHintsForAllReferences52, params.savedRefOrderHints[5][2]);  \
    DO_FIELD(DW11, SavedOrderHintsForAllReferences53, params.savedRefOrderHints[5][3]);  \
                                                                                         \
    DO_FIELD(DW12, SavedOrderHintsForAllReferences54, params.savedRefOrderHints[5][4]);  \
    DO_FIELD(DW12, SavedOrderHintsForAllReferences55, params.savedRefOrderHints[5][5]);  \
    DO_FIELD(DW12, SavedOrderHintsForAllReferences56, params.savedRefOrderHints[5][6]);  \
                                                                                         \
    DO_FIELD(DW13, SavedOrderHintsForAllReferences60, params.savedRefOrderHints[6][0]);  \
    DO_FIELD(DW13, SavedOrderHintsForAllReferences61, params.savedRefOrderHints[6][1]);  \
    DO_FIELD(DW13, SavedOrderHintsForAllReferences62, params.savedRefOrderHints[6][2]);  \
    DO_FIELD(DW13, SavedOrderHintsForAllReferences63, params.savedRefOrderHints[6][3]);  \
                                                                                         \
    DO_FIELD(DW14, SavedOrderHintsForAllReferences64, params.savedRefOrderHints[6][4]);  \
    DO_FIELD(DW14, SavedOrderHintsForAllReferences65, params.savedRefOrderHints[6][5]);  \
    DO_FIELD(DW14, SavedOrderHintsForAllReferences66, params.savedRefOrderHints[6][6])

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_IND_OBJ_BASE_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_IND_OBJ_BASE_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        if (!Mos_ResourceIsNull(params.dataBuffer))
        {
            resourceParams.presResource    = params.dataBuffer;
            resourceParams.dwOffset        = params.dataOffset;
            resourceParams.pdwCmd          = &(cmd.AvpIndirectBitstreamObjectBaseAddress.DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.dwSize          = params.dataSize;
            resourceParams.bIsWritable     = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            InitMocsParams(resourceParams, &cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
        }

        if (!Mos_ResourceIsNull(params.pakBaseObjectBuffer))
        {
            resourceParams.presResource    = params.pakBaseObjectBuffer;
            resourceParams.dwOffset        = params.pakBaseObjectOffset;
            resourceParams.pdwCmd          = &(cmd.AvpIndirectBitstreamObjectBaseAddress.DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(AvpIndirectBitstreamObjectBaseAddress);
            resourceParams.dwSize          = MOS_ALIGN_FLOOR(params.pakBaseObjectSize, PAGE_SIZE);
            resourceParams.bIsWritable     = true;
            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            InitMocsParams(resourceParams, &cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.mvObjectBuffer))
        {
            resourceParams.presResource    = params.mvObjectBuffer;
            resourceParams.dwOffset        = params.mvObjectOffset;
            resourceParams.pdwCmd          = &(cmd.AvpIndirectCuObjectBaseAddress.DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(AvpIndirectCuObjectBaseAddress);
            resourceParams.dwSize          = MOS_ALIGN_CEIL(params.mvObjectSize, PAGE_SIZE);
            resourceParams.bIsWritable     = false;

            // no upper bound for indirect CU object
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.AvpIndirectCuObjectMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_SURFACE_STATE);

#define DO_FIELDS()                                                             \
    DO_FIELD(DW1, SurfaceId, params.surfaceStateId);                            \
    DO_FIELD(DW1, SurfacePitchMinus1, params.pitch - 1);                        \
                                                                                \
    DO_FIELD(DW2, SurfaceFormat, static_cast<uint32_t>(params.srcFormat));      \
    DO_FIELD(DW2, YOffsetForUCbInPixel, params.uOffset);                        \
    DO_FIELD(DW3, YOffsetForVCr, params.vOffset);                               \
                                                                                \
    DO_FIELD(DW4, CompressionFormat, params.compressionFormat)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(AVP_BSD_OBJECT);

#define DO_FIELDS()                                                              \
        DO_FIELD(DW1, TileIndirectBsdDataLength, params.bsdDataLength);          \
        DO_FIELD(DW2, TileIndirectDataStartAddress, params.bsdDataStartOffset)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PAK_INSERT_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(AVP_PAK_INSERT_OBJECT);

        uint32_t dwordsUsed = cmd.dwSize;

        uint32_t byteSize         = (params.bitSize + 7) >> 3;
        uint32_t dataBitsInLastDw = params.bitSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        dwordsUsed += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t);

#define DO_FIELDS()                                                                                 \
    DO_FIELD(DW0, DwordLength, OP_LENGTH(dwordsUsed));                                              \
                                                                                                    \
    DO_FIELD(DW1, EndofsliceflagLastdstdatainsertcommandflag, params.endOfHeaderInsertion ? 1 : 0); \
    DO_FIELD(DW1, LastheaderflagLastsrcheaderdatainsertcommandflag, params.lastHeader ? 1 : 0);     \
    DO_FIELD(DW1, DatabitsinlastdwSrcdataendingbitinclusion50, dataBitsInLastDw);                   \
    DO_FIELD(DW1, DatabyteoffsetSrcdatastartingbyteoffset10, 0);                                    \
    DO_FIELD(DW1, IndirectPayloadEnable, 0)

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__Impl)
};
}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_IMPL_H__
