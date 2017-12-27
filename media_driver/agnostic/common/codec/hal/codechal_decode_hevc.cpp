/*
* Copyright (c) 2012-2017, Intel Corporation
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
//! \file     codechal_decode_hevc.cpp
//! \brief    Implements the decode interface extension for HEVC.
//! \details  Implements all functions required by CodecHal for HEVC decoding.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode.h"
#include "codechal_cenc_decode.h"
#include "codechal_decode_hevc.h"
#include "codechal_mmc_decode_hevc.h"
#include "codechal_decode_nv12top010.h"
#include "media_interfaces_nv12top010.h"

#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif
//==<Functions>=======================================================

uint8_t CodechalDecodeHevc::GetMvBufferIndex(
    uint8_t                         frameIdx)
{
    PCODECHAL_DECODE_HEVC_MV_LIST hevcMVBufList = & hevcMvList[0];

    uint8_t i;
    for (i = 0; i < CODEC_NUM_HEVC_MV_BUFFERS; i++)
    {
        if (!hevcMVBufList[i].bInUse)
        {
            hevcMVBufList[i].bInUse = true;
            hevcMVBufList[i].u8FrameId = frameIdx;
            break;
        }
    }
    if (i == CODEC_NUM_HEVC_MV_BUFFERS)
    {
        // Should never happen, something must be wrong
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to get avaiable MV buffer.");
    }

    return i;
}

MOS_STATUS CodechalDecodeHevc::AllocateResourcesFixedSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &resSyncObjectWaContextInUse));

    CodecHalAllocateDataList(
        pHevcRefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC);

    if (bShortFormatInUse)
    {
        // Second level batch buffer for HuC FW to use
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
        uint32_t u32Size = MOS_ALIGN_CEIL(CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * m_standardDecodeSizeNeeded,
            CODECHAL_PAGE_SIZE);
        CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &secondLevelBatchBuffer,
            nullptr,
            u32Size));
        secondLevelBatchBuffer.bSecondLevel = true;

        // DMEM buffer send to HuC FW
        u32DmemBufferSize = GetDmemBufferSize();

        for (uint32_t i = 0; i < CODECHAL_HEVC_NUM_DMEM_BUFFERS; i++)
        {
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resDmemBuffer[i],
                u32DmemBufferSize,
                "DmemBuffer"),
                "Failed to allocate Dmem Buffer.");
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::AllocateResourcesVariableSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t widthMax    = MOS_MAX(m_width, u32WidthLastMaxAlloced);
    uint32_t heightMax   = MOS_MAX(m_height, u32HeightLastMaxAlloced);
    CODECHAL_DECODE_VERBOSEMESSAGE("m_width = %d, Max Width = %d, m_height %d, Max Height = %d",
        m_width, widthMax, m_height, heightMax);

    uint8_t maxBitDepth       = (bIs10bitHEVC) ? 10 : 8;
    uint8_t chromaFormatPic   = pHevcPicParams->chroma_format_idc;
    uint8_t chromaFormat      = u8ChromaFormatinProfile;
    CODECHAL_DECODE_ASSERT(chromaFormat >= chromaFormatPic);

    uint32_t u32CtbLog2SizeYPic = pHevcPicParams->log2_diff_max_min_luma_coding_block_size +
        pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3;
    uint32_t ctbLog2SizeY    = MOS_MAX(u32CtbLog2SizeYPic, u32CtbLog2SizeYMax);

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth  = maxBitDepth;
    hcpBufSizeParam.ucChromaFormat = chromaFormat;
    hcpBufSizeParam.dwCtbLog2SizeY = ctbLog2SizeY;

    MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam;
    MOS_ZeroMemory(&reallocParam, sizeof(reallocParam));
    reallocParam.ucMaxBitDepth     = maxBitDepth;
    reallocParam.ucChromaFormat    = chromaFormat;
    reallocParam.dwCtbLog2SizeY    = ctbLog2SizeY;
    reallocParam.dwCtbLog2SizeYMax = u32CtbLog2SizeYMax;

    if (bIs8bitFrameIn10bitHevc)
    {
        uint32_t i;
        // Init 8bitRTIndexMap array, 0xff means doesn't map to any InternalNV12RTSurface.
        if (!bInternalNV12RTIndexMapInitilized)
        {
            for (i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; i++)
            {
                u32InternalNV12RTIndexMap[i] = 0xff;
            }

            bInternalNV12RTIndexMapInitilized = true;
        }

        if (u32InternalNV12RTIndexMap[CurrPic.FrameIdx] != 0xff)
        {
            if (!Mos_ResourceIsNull(&sInternalNV12RTSurfaces[u32InternalNV12RTIndexMap[CurrPic.FrameIdx]].OsResource))
            {
                m_osInterface->pfnFreeResource(m_osInterface,
                    &sInternalNV12RTSurfaces[u32InternalNV12RTIndexMap[CurrPic.FrameIdx]].OsResource);
            }
        }

        // Seek an available surface in InternalNV12RTSurface array.
        for (i = 0; i < CODECHAL_NUM_INTERNAL_NV12_RT_HEVC; i++)
        {
            if (Mos_ResourceIsNull(&sInternalNV12RTSurfaces[i].OsResource))
            {
                u32InternalNV12RTIndexMap[CurrPic.FrameIdx] = i;
                break;
            }
        }

        // If there is no available InternalNV12RTSurface in the array.
        if (i == CODECHAL_NUM_INTERNAL_NV12_RT_HEVC)
        {
            // Search an InternalNV12RTSurface to reuse.
            for (uint32_t j = 0; j < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; j++)
            {
                if (u32InternalNV12RTIndexMap[j] != 0xff && j != CurrPic.FrameIdx)
                {
                    uint32_t k;
                    // Check if InternalNV12RTSurface in reference list.
                    for (k = 0; k < CODEC_MAX_NUM_REF_FRAME_HEVC; k++)
                    {
                        if (j == pHevcPicParams->RefFrameList[k].FrameIdx)
                        {
                            break;
                        }
                    }

                    // If InternalNV12RTSurface is not in reference list, reuse it.
                    if (k == CODEC_MAX_NUM_REF_FRAME_HEVC)
                    {
                        u32InternalNV12RTIndexMap[CurrPic.FrameIdx] = u32InternalNV12RTIndexMap[j];
                        u32InternalNV12RTIndexMap[j] = 0xff;
                        break;
                    }
                }
            }
        }

        uint32_t internalNV12RTIndex = u32InternalNV12RTIndexMap[CurrPic.FrameIdx];

        if (Mos_ResourceIsNull(&sInternalNV12RTSurfaces[internalNV12RTIndex].OsResource)  ||
            sDestSurface.dwWidth  != sInternalNV12RTSurfaces[internalNV12RTIndex].dwWidth ||
            sDestSurface.dwHeight != sInternalNV12RTSurfaces[internalNV12RTIndex].dwHeight)
        {
            if (!Mos_ResourceIsNull(&sInternalNV12RTSurfaces[internalNV12RTIndex].OsResource))
            {
                m_osInterface->pfnFreeResource(m_osInterface, &sInternalNV12RTSurfaces[internalNV12RTIndex].OsResource);
            }

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateSurface(
                &sInternalNV12RTSurfaces[internalNV12RTIndex],
                sDestSurface.dwWidth,
                sDestSurface.dwHeight,
                "HevcInternalNV12RTSurfaces"),
                "Failed to allocate Hevc Internal NV12 dest surface data buffer.");
        }
    }

    if (!m_hcpInterface->IsHevcDfRowstoreCacheEnabled())
    {
        uint32_t mfdDeblockingFilterRowStoreScratchBufferPicWidthMax =
            MOS_MAX(m_width, u32MfdDeblockingFilterRowStoreScratchBufferPicWidth);

        reallocParam.dwPicWidth         = mfdDeblockingFilterRowStoreScratchBufferPicWidthMax;
        reallocParam.dwPicWidthAlloced  = u32MfdDeblockingFilterRowStoreScratchBufferPicWidth;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
            &reallocParam));
        if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resMfdDeblockingFilterRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resMfdDeblockingFilterRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMfdDeblockingFilterRowStoreScratchBuffer);
            }

            // Deblocking Filter Row Store Scratch buffer
            hcpBufSizeParam.dwPicWidth = mfdDeblockingFilterRowStoreScratchBufferPicWidthMax;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
                &hcpBufSizeParam));

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMfdDeblockingFilterRowStoreScratchBuffer,
                hcpBufSizeParam.dwBufferSize,
                "DeblockingScratchBuffer"),
                "Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u32MfdDeblockingFilterRowStoreScratchBufferPicWidth = mfdDeblockingFilterRowStoreScratchBufferPicWidthMax;
    }
    
    reallocParam.dwPicWidth         = widthMax;
    reallocParam.dwPicWidthAlloced  = u32WidthLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resDeblockingFilterTileRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&resDeblockingFilterTileRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resDeblockingFilterTileRowStoreScratchBuffer);
        }

        // Deblocking Filter Tile Row Store Scratch data surface
        hcpBufSizeParam.dwPicWidth = widthMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resDeblockingFilterTileRowStoreScratchBuffer,
            hcpBufSizeParam.dwBufferSize,
            "DeblockingTileScratchBuffer"),
            "Failed to allocate Deblocking Filter Tile Row Store Scratch Buffer.");
    }

    reallocParam.dwPicHeight        = heightMax;
    reallocParam.dwPicHeightAlloced = u32HeightLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resDeblockingFilterColumnRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&resDeblockingFilterColumnRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resDeblockingFilterColumnRowStoreScratchBuffer);
        }
        // Deblocking Filter Column Row Store Scratch data surface
        hcpBufSizeParam.dwPicHeight = heightMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resDeblockingFilterColumnRowStoreScratchBuffer,
            hcpBufSizeParam.dwBufferSize,
            "DeblockingColumnScratchBuffer"),
            "Failed to allocate Deblocking Filter Column Row Store Scratch Buffer.");
    }

    if (!m_hcpInterface->IsHevcDatRowstoreCacheEnabled())
    {
        uint32_t metadataLineBufferPicWidthMax =
            MOS_MAX(m_width, u32MetadataLineBufferPicWidth);

        reallocParam.dwPicWidth         = metadataLineBufferPicWidthMax;
        reallocParam.dwPicWidthAlloced  = u32MetadataLineBufferPicWidth;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
            &reallocParam));

        if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resMetadataLineBuffer))
        {
            if (!Mos_ResourceIsNull(&resMetadataLineBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMetadataLineBuffer);
            }

            // Metadata Line buffer
            hcpBufSizeParam.dwPicWidth = metadataLineBufferPicWidthMax;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
                &hcpBufSizeParam));

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMetadataLineBuffer,
                hcpBufSizeParam.dwBufferSize,
                "MetadataLineBuffer"),
                "Failed to allocate Metadata Line Buffer.");
        }

        //record the width and height used for allocation internal resources.
		u32MetadataLineBufferPicWidth = metadataLineBufferPicWidthMax;
    }

    reallocParam.dwPicWidth         = widthMax;
    reallocParam.dwPicWidthAlloced  = u32WidthLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resMetadataTileLineBuffer))
    {
        if (!Mos_ResourceIsNull(&resMetadataTileLineBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMetadataTileLineBuffer);
        }
        // Metadata Tile Line buffer
        hcpBufSizeParam.dwPicWidth = widthMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMetadataTileLineBuffer,
            hcpBufSizeParam.dwBufferSize,
            "MetadataTileLineBuffer"),
            "Failed to allocate Metadata Tile Line Buffer.");
    }

    reallocParam.dwPicHeight        = heightMax;
    reallocParam.dwPicHeightAlloced = u32HeightLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resMetadataTileColumnBuffer))
    {
        if (!Mos_ResourceIsNull(&resMetadataTileColumnBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMetadataTileColumnBuffer);
        }
        // Metadata Tile Column buffer
        hcpBufSizeParam.dwPicHeight = heightMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMetadataTileColumnBuffer,
            hcpBufSizeParam.dwBufferSize,
            "MetadataTileColumnBuffer"),
            "Failed to allocate Metadata Tile Column Buffer.");
    }

    if (!m_hcpInterface->IsHevcSaoRowstoreCacheEnabled())
    {
        uint32_t saoLineBufferPicWidthMax =
            MOS_MAX(m_width, u32SaoLineBufferPicWidth);

        reallocParam.dwPicWidth         = saoLineBufferPicWidthMax;
        reallocParam.dwPicWidthAlloced  = u32SaoLineBufferPicWidth;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
            &reallocParam));   
        if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resSaoLineBuffer))
        {
            if (!Mos_ResourceIsNull(&resSaoLineBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resSaoLineBuffer);
            }

            // SAO Line buffer
            hcpBufSizeParam.dwPicWidth = saoLineBufferPicWidthMax;    
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
                MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
                &hcpBufSizeParam));

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resSaoLineBuffer,
                hcpBufSizeParam.dwBufferSize,
                "SaoLineBuffer"),
                "Failed to allocate SAO Line Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u32SaoLineBufferPicWidth = saoLineBufferPicWidthMax;
    }
    
    reallocParam.dwPicWidth         = widthMax;
    reallocParam.dwPicWidthAlloced  = u32WidthLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resSaoTileLineBuffer))
    {
        if (!Mos_ResourceIsNull(&resSaoTileLineBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resSaoTileLineBuffer);
        }
        // SAO Tile Line buffer 
        hcpBufSizeParam.dwPicWidth = widthMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resSaoTileLineBuffer,
            hcpBufSizeParam.dwBufferSize,
            "SaoTileLineBuffer"),
            "Failed to allocate SAO Tile Line Buffer.");
    }

    reallocParam.dwPicHeight        = heightMax;
    reallocParam.dwPicHeightAlloced = u32HeightLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
        &reallocParam));
    if (reallocParam.bNeedBiggerSize || Mos_ResourceIsNull(&resSaoTileColumnBuffer))
    {
        if (!Mos_ResourceIsNull(&resSaoTileColumnBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resSaoTileColumnBuffer);
        }
        // SAO Tile Column buffer
        hcpBufSizeParam.dwPicHeight = heightMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
            &hcpBufSizeParam));

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resSaoTileColumnBuffer,
            hcpBufSizeParam.dwBufferSize,
            "SaoTileColumnBuffer"),
            "Failed to allocate SAO Tile Column Buffer.");
    }

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AllocateResource(this));
    }

    u32WidthLastMaxAlloced  = widthMax;
    u32HeightLastMaxAlloced = heightMax;
    u32CtbLog2SizeYMax      = ctbLog2SizeY;

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::AllocateMvTemporalBuffer(
    uint8_t hevcMvBuffIndex)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (hevcMvBuffIndex == CODEC_NUM_HEVC_MV_BUFFERS)
    {
        // Should never happen, something must be wrong
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to get avaiable MV buffer.");
        return     MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t widthMax  = MOS_MAX(m_width, u32WidthLastMaxAlloced);
    uint32_t heightMax = MOS_MAX(m_height, u32HeightLastMaxAlloced);

    MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam;
    MOS_ZeroMemory(&reallocParam, sizeof(reallocParam));

    reallocParam.dwPicWidth         = widthMax;
    reallocParam.dwPicWidthAlloced  = u32WidthLastMaxAlloced;
    reallocParam.dwPicHeight        = heightMax;
    reallocParam.dwPicHeightAlloced = u32HeightLastMaxAlloced;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
        &reallocParam));

    int32_t isResMvTemporalBufferNull = Mos_ResourceIsNull(&resMvTemporalBuffer[hevcMvBuffIndex]);

    if (reallocParam.bNeedBiggerSize || isResMvTemporalBufferNull)
    {
        MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
        MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));

        if (!isResMvTemporalBufferNull)
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMvTemporalBuffer[hevcMvBuffIndex]);
        }

        // MV Temporal buffers
        hcpBufSizeParam.dwPicWidth  = widthMax;
        hcpBufSizeParam.dwPicHeight = heightMax;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->GetHevcBufferSize(
            MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
            &hcpBufSizeParam));
        u32MVBufferSize = hcpBufSizeParam.dwBufferSize;

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMvTemporalBuffer[hevcMvBuffIndex],
            hcpBufSizeParam.dwBufferSize,
            "CurrentMvTemporalBuffer"),
            "Failed to allocate MV Temporal Buffer.");
    }

    return eStatus;
}

CodechalDecodeHevc::~CodechalDecodeHevc ()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
        
    m_osInterface->pfnDestroySyncResource(m_osInterface, &resSyncObjectWaContextInUse);

    CodecHalFreeDataList(pHevcRefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC);

    if (!m_hcpInterface->IsHevcDfRowstoreCacheEnabled())
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resMfdDeblockingFilterRowStoreScratchBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resDeblockingFilterTileRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resDeblockingFilterColumnRowStoreScratchBuffer);

    if (!m_hcpInterface->IsHevcDatRowstoreCacheEnabled())
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resMetadataLineBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMetadataTileLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMetadataTileColumnBuffer);

    if (!m_hcpInterface->IsHevcSaoRowstoreCacheEnabled())
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resSaoLineBuffer);
    }
    
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resSaoTileLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resSaoTileColumnBuffer);

    for (uint32_t i = 0; i < CODEC_NUM_HEVC_MV_BUFFERS; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resMvTemporalBuffer[i]);
    }

    if (bShortFormatInUse)
    {
        Mhw_FreeBb(m_osInterface, &secondLevelBatchBuffer, nullptr);

        for (uint32_t i = 0; i < CODECHAL_HEVC_NUM_DMEM_BUFFERS; i++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &resDmemBuffer[i]);
        }
    }

    if (!Mos_ResourceIsNull(&resCopyDataBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resCopyDataBuffer);
    }

    for (uint32_t i = 0; i < CODECHAL_NUM_INTERNAL_NV12_RT_HEVC; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &sInternalNV12RTSurfaces[i].OsResource);
    }

    if (m_decodeNV12ToP010 != nullptr)
    {
        MOS_Delete(m_decodeNV12ToP010);
    }
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_sfcState)
    {
        MOS_Delete(m_sfcState);
        m_sfcState = nullptr;
    }
#endif
    if (m_picMhwParams.PipeModeSelectParams)
    {
        MOS_Delete(m_picMhwParams.PipeModeSelectParams);
        m_picMhwParams.PipeModeSelectParams = nullptr;
    }
    if (m_picMhwParams.SurfaceParams)
    {
        MOS_Delete(m_picMhwParams.SurfaceParams);
        m_picMhwParams.SurfaceParams = nullptr;
    }
    if (m_picMhwParams.PipeBufAddrParams)
    {
        MOS_Delete(m_picMhwParams.PipeBufAddrParams);
        m_picMhwParams.PipeBufAddrParams = nullptr;
    }
    if (m_picMhwParams.IndObjBaseAddrParams)
    {
        MOS_Delete(m_picMhwParams.IndObjBaseAddrParams);
        m_picMhwParams.IndObjBaseAddrParams = nullptr;
    }
    if (m_picMhwParams.QmParams)
    {
        MOS_Delete(m_picMhwParams.QmParams);
        m_picMhwParams.QmParams = nullptr;
    }
    if (m_picMhwParams.HevcPicState)
    {
        MOS_Delete(m_picMhwParams.HevcPicState);
        m_picMhwParams.HevcPicState = nullptr;
    }
    if (m_picMhwParams.HevcTileState)
    {
        MOS_Delete(m_picMhwParams.HevcTileState);
        m_picMhwParams.HevcTileState = nullptr;
    }

    return;
}

uint32_t CodechalDecodeHevc::GetDmemBufferSize()
{
    return MOS_ALIGN_CEIL(sizeof(HUC_HEVC_S2L_BSS), CODECHAL_CACHELINE_SIZE);
}

MOS_STATUS CodechalDecodeHevc::SetHucDmemS2LPictureBss(
    PHUC_HEVC_S2L_PIC_BSS       hucHevcS2LPicBss)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(hucHevcS2LPicBss);

    hucHevcS2LPicBss->pic_width_in_min_cbs_y                       = pHevcPicParams->PicWidthInMinCbsY;
    hucHevcS2LPicBss->pic_height_in_min_cbs_y                      = pHevcPicParams->PicHeightInMinCbsY;
    hucHevcS2LPicBss->log2_min_luma_coding_block_size_minus3       = pHevcPicParams->log2_min_luma_coding_block_size_minus3;
    hucHevcS2LPicBss->log2_diff_max_min_luma_coding_block_size     = pHevcPicParams->log2_diff_max_min_luma_coding_block_size;
    hucHevcS2LPicBss->chroma_format_idc                            = pHevcPicParams->chroma_format_idc;
    hucHevcS2LPicBss->separate_colour_plane_flag                   = pHevcPicParams->separate_colour_plane_flag;
    hucHevcS2LPicBss->bit_depth_luma_minus8                        = pHevcPicParams->bit_depth_luma_minus8;
    hucHevcS2LPicBss->bit_depth_chroma_minus8                      = pHevcPicParams->bit_depth_chroma_minus8;
    hucHevcS2LPicBss->log2_max_pic_order_cnt_lsb_minus4            = pHevcPicParams->log2_max_pic_order_cnt_lsb_minus4;
    hucHevcS2LPicBss->sample_adaptive_offset_enabled_flag          = pHevcPicParams->sample_adaptive_offset_enabled_flag;
    hucHevcS2LPicBss->num_short_term_ref_pic_sets                  = pHevcPicParams->num_short_term_ref_pic_sets;
    hucHevcS2LPicBss->long_term_ref_pics_present_flag              = pHevcPicParams->long_term_ref_pics_present_flag;
    hucHevcS2LPicBss->num_long_term_ref_pics_sps                   = pHevcPicParams->num_long_term_ref_pic_sps;
    hucHevcS2LPicBss->sps_temporal_mvp_enable_flag                 = pHevcPicParams->sps_temporal_mvp_enabled_flag;
    hucHevcS2LPicBss->num_ref_idx_l0_default_active_minus1         = pHevcPicParams->num_ref_idx_l0_default_active_minus1;
    hucHevcS2LPicBss->num_ref_idx_l1_default_active_minus1         = pHevcPicParams->num_ref_idx_l1_default_active_minus1;
    hucHevcS2LPicBss->pic_init_qp_minus26                          = pHevcPicParams->init_qp_minus26;
    hucHevcS2LPicBss->dependent_slice_segments_enabled_flag        = pHevcPicParams->dependent_slice_segments_enabled_flag;
    hucHevcS2LPicBss->cabac_init_present_flag                      = pHevcPicParams->cabac_init_present_flag;
    hucHevcS2LPicBss->pps_slice_chroma_qp_offsets_present_flag     = pHevcPicParams->pps_slice_chroma_qp_offsets_present_flag;
    hucHevcS2LPicBss->weighted_pred_flag                           = pHevcPicParams->weighted_pred_flag;
    hucHevcS2LPicBss->weighted_bipred_flag                         = pHevcPicParams->weighted_bipred_flag;
    hucHevcS2LPicBss->output_flag_present_flag                     = pHevcPicParams->output_flag_present_flag;
    hucHevcS2LPicBss->tiles_enabled_flag                           = pHevcPicParams->tiles_enabled_flag;
    hucHevcS2LPicBss->entropy_coding_sync_enabled_flag             = pHevcPicParams->entropy_coding_sync_enabled_flag;
    hucHevcS2LPicBss->loop_filter_across_slices_enabled_flag       = pHevcPicParams->pps_loop_filter_across_slices_enabled_flag;
    hucHevcS2LPicBss->deblocking_filter_override_enabled_flag      = pHevcPicParams->deblocking_filter_override_enabled_flag;
    hucHevcS2LPicBss->pic_disable_deblocking_filter_flag           = pHevcPicParams->pps_deblocking_filter_disabled_flag;
    hucHevcS2LPicBss->lists_modification_present_flag              = pHevcPicParams->lists_modification_present_flag;
    hucHevcS2LPicBss->slice_segment_header_extension_present_flag  = pHevcPicParams->slice_segment_header_extension_present_flag;
    hucHevcS2LPicBss->high_precision_offsets_enabled_flag          = 0;
    hucHevcS2LPicBss->chroma_qp_offset_list_enabled_flag           = 0;

    uint32_t i;
    hucHevcS2LPicBss->CurrPicOrderCntVal                           = pHevcPicParams->CurrPicOrderCntVal;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        hucHevcS2LPicBss->PicOrderCntValList[i]                    = pHevcPicParams->PicOrderCntValList[i];
    }

    for (i = 0; i < 8; i++)
    {
        hucHevcS2LPicBss->RefPicSetStCurrBefore[i]                 = pHevcPicParams->RefPicSetStCurrBefore[i];
        hucHevcS2LPicBss->RefPicSetStCurrAfter[i]                  = pHevcPicParams->RefPicSetStCurrAfter[i];
        hucHevcS2LPicBss->RefPicSetLtCurr[i]                       = pHevcPicParams->RefPicSetLtCurr[i];
    }

    hucHevcS2LPicBss->RefFieldPicFlag                              = pHevcPicParams->RefFieldPicFlag;
    hucHevcS2LPicBss->RefBottomFieldFlag                           = (uint8_t)pHevcPicParams->RefBottomFieldFlag;
    hucHevcS2LPicBss->pps_beta_offset_div2                         = pHevcPicParams->pps_beta_offset_div2;
    hucHevcS2LPicBss->pps_tc_offset_div2                           = pHevcPicParams->pps_tc_offset_div2;
    hucHevcS2LPicBss->StRPSBits                                    = pHevcPicParams->wNumBitsForShortTermRPSInSlice;

    if (pHevcPicParams->tiles_enabled_flag)
    {
        hucHevcS2LPicBss->num_tile_columns_minus1                  = pHevcPicParams->num_tile_columns_minus1;
        hucHevcS2LPicBss->num_tile_rows_minus1                     = pHevcPicParams->num_tile_rows_minus1;

        for (i = 0; i < HEVC_NUM_MAX_TILE_COLUMN; i++)
        {
            hucHevcS2LPicBss->column_width[i]                      = u16TileColWidth[i];
        }

        for (i = 0; i < HEVC_NUM_MAX_TILE_ROW; i++)
        {
            hucHevcS2LPicBss->row_height[i]                        = u16TileRowHeight[i];
        }
    }

    hucHevcS2LPicBss->NumSlices                                    = (uint16_t)u32NumSlices;
    hucHevcS2LPicBss->num_extra_slice_header_bits                  = pHevcPicParams->num_extra_slice_header_bits;

    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        hucHevcS2LPicBss->RefIdxMapping[i]                         = RefIdxMapping[i];
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SetHucDmemS2LSliceBss(
    PHUC_HEVC_S2L_SLICE_BSS         hucHevcS2LSliceBss)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(hucHevcS2LSliceBss);

    for (uint32_t i = 0; i < u32NumSlices; i++)
    {
        hucHevcS2LSliceBss->BSNALunitDataLocation                  = pHevcSliceParams[i].slice_data_offset;
        hucHevcS2LSliceBss->SliceBytesInBuffer                     = pHevcSliceParams[i].slice_data_size;
        hucHevcS2LSliceBss++;
    }
    
    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SetHucDmemParams(
    PMOS_RESOURCE               dmemBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(dmemBuffer);

    CodechalResLock DmemLock(m_osInterface, dmemBuffer);
    auto hucHevcS2LBss = (PHUC_HEVC_S2L_BSS)DmemLock.Lock(CodechalResLock::writeOnly);

    CODECHAL_DECODE_CHK_NULL_RETURN(hucHevcS2LBss);
    hucHevcS2LBss->ProductFamily = m_huCProductFamily;
    hucHevcS2LBss->RevId = m_hwInterface->GetPlatform().usRevId;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetHucDmemS2LPictureBss(&hucHevcS2LBss->PictureBss));
    CODECHAL_DECODE_CHK_STATUS_RETURN(SetHucDmemS2LSliceBss(&hucHevcS2LBss->SliceBss[0]));

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetHevcHucDmemS2LBss(this, &hucHevcS2LBss->PictureBss, &hucHevcS2LBss->SliceBss[0]));
    }

    if (u32NumSlices < CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6)
    {
        u32DmemTransferSize = (uint32_t)((uint8_t *)&(hucHevcS2LBss->SliceBss[u32NumSlices]) - (uint8_t *)hucHevcS2LBss);
        u32DmemTransferSize = MOS_ALIGN_CEIL(u32DmemTransferSize, CODECHAL_CACHELINE_SIZE);
    }
    else
    {
        u32DmemTransferSize = u32DmemBufferSize;
    }
    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::GetAllTileInfo()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if ((pHevcPicParams->num_tile_columns_minus1 >= HEVC_NUM_MAX_TILE_COLUMN) ||
        (pHevcPicParams->num_tile_rows_minus1 >= HEVC_NUM_MAX_TILE_ROW))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("num_tile_columns_minus1 or num_tile_rows_minus1 is out of range!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t ctbSize     = 1 << (pHevcPicParams->log2_diff_max_min_luma_coding_block_size + pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    uint32_t widthInPix  = (1 << (pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) *
                            (pHevcPicParams->PicWidthInMinCbsY);
    uint32_t heightInPix = (1 << (pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) *
                            (pHevcPicParams->PicHeightInMinCbsY);
    uint32_t widthInCtb  = MOS_ROUNDUP_DIVIDE(widthInPix, ctbSize);
    uint32_t heightInCtb = MOS_ROUNDUP_DIVIDE(heightInPix, ctbSize);

    uint16_t *tileColWidth, *tileRowHeight;
    tileColWidth = & u16TileColWidth[0];
    tileRowHeight = & u16TileRowHeight[0];
    if (pHevcPicParams->uniform_spacing_flag == 1)
    {
        uint8_t i;
        for (i = 0; i <= pHevcPicParams->num_tile_columns_minus1; i++)
        {
            tileColWidth[i]  = (( i + 1 ) * widthInCtb) / (pHevcPicParams->num_tile_columns_minus1 + 1) -
                                 (     i    * widthInCtb) / (pHevcPicParams->num_tile_columns_minus1 + 1);
        }

        for (i = 0; i <= pHevcPicParams->num_tile_rows_minus1; i++)
        {
            tileRowHeight[i] = (( i + 1 ) * heightInCtb) / (pHevcPicParams->num_tile_rows_minus1 + 1) -
                                 (    i    * heightInCtb) / (pHevcPicParams->num_tile_rows_minus1 + 1);
        }
    }
    else
    {
        uint8_t i;
        tileColWidth[pHevcPicParams->num_tile_columns_minus1] = widthInCtb & 0xffff;
        for (i = 0; i < pHevcPicParams->num_tile_columns_minus1; i++)
        {
            tileColWidth[i] = pHevcPicParams->column_width_minus1[i] + 1;
            tileColWidth[pHevcPicParams->num_tile_columns_minus1] -= tileColWidth[i];
        }

        tileRowHeight[pHevcPicParams->num_tile_rows_minus1] = heightInCtb & 0xffff;
        for (i = 0; i < pHevcPicParams->num_tile_rows_minus1; i++)
        {
            tileRowHeight[i] = pHevcPicParams->row_height_minus1[i] + 1;
            tileRowHeight[pHevcPicParams->num_tile_rows_minus1] -= tileRowHeight[i];
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::InitializeBitstreamCat ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    m_incompletePicture = false;
    bCopyDataBufferInUse = false;
    u32CopyDataOffset = 0;

    // For multiple execution case, each execution for one frame will increase pDecoderInterface->dwFrameNum in CodecHalDecode_Decode.
    // This will lead to dump index error.
    // Need to make sure that pDecoderInterface->CurrPic won't update until all bitstream is copied.
    m_crrPic.PicFlags = PICTURE_INVALID;

    // Estimate Bytes in Bitstream per frame
    PCODEC_HEVC_SLICE_PARAMS hevcLastSliceParamsInFrame = pHevcSliceParams + (u32NumSlices - 1);
    u32EstiBytesInBitstream = MOS_ALIGN_CEIL(hevcLastSliceParamsInFrame->slice_data_offset + hevcLastSliceParamsInFrame->slice_data_size, 64);
    CODECHAL_DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", u32EstiBytesInBitstream);

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::CopyDataSurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContextForWa));
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer,
        false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        &cmdBuffer,         // pCmdBuffer
        &resDataBuffer,     // presSrc
        &resCopyDataBuffer, // presDst
        u32DataSize,        // u32CopyLength
        u32DataOffset,      // u32CopyInputOffset
        u32CopyDataOffset));// u32CopyOutputOffset
    
    u32CopyDataOffset += MOS_ALIGN_CEIL(u32DataSize, MHW_CACHELINE_SIZE);

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(
        &cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    // sync resource
    if (!m_incompletePicture)
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &resSyncObjectWaContextInUse;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &resSyncObjectWaContextInUse;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_videoContextForWaUsesNullHw));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContext));

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::CheckAndCopyBitstream()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_firstExecuteCall)    // first exec to decide allocate a larger buf or not
    {
        if (u32EstiBytesInBitstream > MOS_ALIGN_CEIL(u32DataOffset + u32DataSize, 64))    // bitstream contains more bytes than current data.
        {
            CODECHAL_DECODE_NORMALMESSAGE("Multiple Execution Call for HEVC triggered!");

            if (u32CopyDataBufferSize < u32EstiBytesInBitstream) // allocate an appropriate buffer
            {
                if (!Mos_ResourceIsNull(&resCopyDataBuffer))
                {
                    m_osInterface->pfnFreeResource(
                        m_osInterface,
                        &resCopyDataBuffer);
                }

                CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                    &resCopyDataBuffer,
                    u32EstiBytesInBitstream,
                    "HevcCopyDataBuffer"),
                    "Failed to allocate Hevc copy data buffer.");

                u32CopyDataBufferSize = u32EstiBytesInBitstream;
                CODECHAL_DECODE_NORMALMESSAGE("Create buffersize %d for MEC.", u32CopyDataBufferSize);
            }

            if (u32DataSize)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());

                bCopyDataBufferInUse = true;
            }

            m_incompletePicture = true;
        }
    }
    else
    {
        if (u32CopyDataOffset + u32DataSize > u32CopyDataBufferSize)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Bitstream size exceeds copy data buffer size!");
            return MOS_STATUS_UNKNOWN;
        }

        if (u32DataSize)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
            u32FrameIdx--;   // to keep u32FrameIdx as normal logic meaning.
        }

        if (u32CopyDataOffset >= u32EstiBytesInBitstream)
        {
            m_incompletePicture = false;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SetFrameStates ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    u32FrameIdx++;

    // Check HuC_status2 Imem loaded bit, if 0,return error
    // As driver doesn't know when can get reg value afer storing HuC_Status2 register,
    // Check the reg value here at the beginning of next frame
    // Check twice, first entry and second entry
    if (bShortFormatInUse &&
        u32FrameIdx < 3 &&
        m_statusQueryReportingEnabled &&
        (((m_decodeStatusBuf.m_decodeStatus->m_hucErrorStatus2 >> 32) & m_hucInterface->GetHucStatus2ImemLoadedMask()) == 0))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("HuC IMEM Loaded fails");
        return MOS_STATUS_UNKNOWN;
    }

    if(m_cencDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->SetParamsForDecode(this, m_hwInterface, m_debugInterface, &m_decodeParams));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                u32DataSize,
                u32DataOffset,
                CODECHAL_NUM_MEDIA_STATES));
        )
    }
    else
    {
        if (m_firstExecuteCall)    // For DRC Multiple Execution Call, no need to update every value in pHevcState except first execute
        {
            u32DataSize              = m_decodeParams.m_dataSize;
            u32DataOffset            = m_decodeParams.m_dataOffset;
            u32NumSlices             = m_decodeParams.m_numSlices;

            if (u32NumSlices > CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("Slice number doesn't support!");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            pHevcPicParams          = (PCODEC_HEVC_PIC_PARAMS)m_decodeParams.m_picParams;
            CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_sliceParams);
            pHevcSliceParams        = (PCODEC_HEVC_SLICE_PARAMS)m_decodeParams.m_sliceParams;
            pHevcIqMatrixParams     = (PCODECHAL_HEVC_IQ_MATRIX_PARAMS)m_decodeParams.m_iqMatrixBuffer;
            sDestSurface            = *(m_decodeParams.m_destSurface);
            resDataBuffer           = *(m_decodeParams.m_dataBuffer);
            
            CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeBitstreamCat());
        }
        else
        {
            u32DataSize              = m_decodeParams.m_dataSize;
            u32DataOffset            = 0;
            resDataBuffer           = *(m_decodeParams.m_dataBuffer);
        }
        
        CODECHAL_DECODE_CHK_STATUS_RETURN(CheckAndCopyBitstream());
        
        //For CENC case, the Entry has been initialized with value in SetParamsForDecode
        PCODEC_REF_LIST destEntry = pHevcRefList[pHevcPicParams->CurrPic.FrameIdx];
        MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
    }

    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(pHevcPicParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(pHevcIqMatrixParams);

    // Calculate bIs8bitFrameIn10bitHevc
    if (MEDIA_IS_WA(m_waTable, Wa8BitFrameIn10BitHevc)               &&
        bIs10bitHEVC                                    &&
        pHevcPicParams->bit_depth_luma_minus8   == 0    &&
        pHevcPicParams->bit_depth_chroma_minus8 == 0    &&
        m_decodeParams.m_destSurface->Format    == Format_P010
        )
    {
        bIs8bitFrameIn10bitHevc = true;
    }
    else
    {
        bIs8bitFrameIn10bitHevc = false;
    }

    // InitNV12ToP010Context
    if (bIs8bitFrameIn10bitHevc && m_decodeNV12ToP010 == nullptr)
    {
        m_decodeNV12ToP010 = Nv12ToP010Device::CreateFactory(m_osInterface);
        CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeNV12ToP010);
    }

    // Calculate bCurPicIntra
    bCurPicIntra = true;
    if (pHevcPicParams->IntraPicFlag == 0)
    {
        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            uint8_t frameIdx = pHevcPicParams->RefPicSetStCurrBefore[i];
            if (frameIdx < 15)
            {
                bCurPicIntra = false;
                break;
            }

            frameIdx = pHevcPicParams->RefPicSetStCurrAfter[i];
            if (frameIdx < 15)
            {
                bCurPicIntra = false;
                break;
            }

            frameIdx = pHevcPicParams->RefPicSetLtCurr[i];
            if (frameIdx < 15)
            {
                bCurPicIntra = false;
                break;
            }
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetPictureStructs());

    uint32_t i;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        bFrameUsedAsCurRef[i] = false;
        RefIdxMapping[i]      = -1;
    }

    // Calculate RefIdxMapping
    for (i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        uint8_t frameIdx = pHevcPicParams->RefPicSetStCurrBefore[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            bFrameUsedAsCurRef[frameIdx] = true;
        }

        frameIdx = pHevcPicParams->RefPicSetStCurrAfter[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            bFrameUsedAsCurRef[frameIdx] = true;
        }

        frameIdx = pHevcPicParams->RefPicSetLtCurr[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            bFrameUsedAsCurRef[frameIdx] = true;
        }
    }

    uint8_t curRefIdx = 0;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (bFrameUsedAsCurRef[i])
        {
            RefIdxMapping[i] = curRefIdx++;
        }
    }

    CODECHAL_DECODE_ASSERT(curRefIdx <= 8);

    u32MinCtbSize   = 1 << (pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_width         = pHevcPicParams->PicWidthInMinCbsY * u32MinCtbSize;
    m_height        = pHevcPicParams->PicHeightInMinCbsY * u32MinCtbSize;

    if (m_hcpInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        rowstoreParams.Mode             = CODECHAL_DECODE_MODE_HEVCVLD;
        rowstoreParams.dwPicWidth       = m_width;
        rowstoreParams.bMbaff           = false;
        rowstoreParams.ucBitDepthMinus8 = (uint8_t)MOS_MAX(pHevcPicParams->bit_depth_luma_minus8, pHevcPicParams->bit_depth_chroma_minus8);
        rowstoreParams.ucChromaFormat   = pHevcPicParams->chroma_format_idc;
        rowstoreParams.ucLCUSize        = 1 << (pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3 + 
            pHevcPicParams->log2_diff_max_min_luma_coding_block_size);
        m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams);
    }


    AllocateResourcesVariableSizes();
    
    // Calculate Tile info
    if (pHevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetAllTileInfo());
    }

    HcpDecPhase = CodechalHcpDecodePhaseInitialized;
    
    if (bCurPicIntra)
    {
        m_perfType = I_TYPE;
    }
    else
    {
        // Not possible to determine whether P or B is used for short format.
        // For long format iterating through all of the slices to determine P vs
        // B, so in order to avoid this, declare all other pictures MIXED_TYPE.
        m_perfType = MIXED_TYPE;
    }

    m_crrPic = pHevcPicParams->CurrPic;
    m_secondField =
        CodecHal_PictureIsBottomField(pHevcPicParams->CurrPic);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->CurrPic = m_crrPic;
        m_debugInterface->bSecondField = m_secondField;
        m_debugInterface->wFrameType = m_perfType;

    CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
        pHevcPicParams,
        nullptr));

    if (pHevcIqMatrixParams )
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(pHevcIqMatrixParams));
    }

    if (pHevcSliceParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSliceParams(
            pHevcSliceParams,
            nullptr,
            u32NumSlices,
            bShortFormatInUse));
    }
    )

    // Clear DMEM buffer program flag and increase the Dmem buffer index to program
    if (bShortFormatInUse)
    {
        bDmemBufferProgrammed = false;
        u32DmemBufferIdx++;
        u32DmemBufferIdx %= CODECHAL_HEVC_NUM_DMEM_BUFFERS;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    // Check if SFC can be supported
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->CheckAndInitialize(m_decodeParams.m_procParams, pHevcPicParams));
#endif
    CODECHAL_DEBUG_TOOL(
        if (!m_incompletePicture && !m_firstExecuteCall)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resCopyDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                u32EstiBytesInBitstream,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }
    )

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODEC_HEVC_PIC_PARAMS picParams               = pHevcPicParams;
    PCODEC_REF_LIST *hevcRefList                   = &pHevcRefList[0];
    PCODECHAL_DECODE_HEVC_MV_LIST hevcMVBufferList = &hevcMvList[0];

    CODEC_PICTURE prevPic = CurrPic;
    CurrPic = picParams->CurrPic;

    m_statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;

    if (CurrPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("currPic.FrameIdx is out of range!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    hevcRefList[CurrPic.FrameIdx]->RefPic             = CurrPic;
    hevcRefList[CurrPic.FrameIdx]->sFrameNumber       = (int16_t) picParams->CurrPicOrderCntVal;
    hevcRefList[CurrPic.FrameIdx]->iFieldOrderCnt[0]  = picParams->CurrPicOrderCntVal;
    hevcRefList[CurrPic.FrameIdx]->bIsIntra           = bCurPicIntra;
    hevcRefList[CurrPic.FrameIdx]->resRefPic          = sDestSurface.OsResource;

    uint8_t i;
    for(i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        hevcRefList[CurrPic.FrameIdx]->RefList[i] = picParams->RefFrameList[i];
    }

    if(!CodecHal_PictureIsInvalid(prevPic))
    {
        for(i = 0; i < CODEC_NUM_HEVC_MV_BUFFERS; i++)
        {
            hevcMVBufferList[i].bInUse    = false;
            hevcMVBufferList[i].u8FrameId = 0;
        }

        //Mark Referenced frame's MV buffer as used
        for(i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if(!CodecHal_PictureIsInvalid(picParams->RefFrameList[i]))
            {
                uint8_t index = picParams->RefFrameList[i].FrameIdx;
                if (index < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
                {
                    hevcMVBufferList[hevcRefList[index]->ucDMVIdx[0]].bInUse = true;
                    hevcMVBufferList[hevcRefList[index]->ucDMVIdx[0]].u8FrameId = index;
                }
            }
        }
    }

    //Find out an unused MvBuffer for current frame
    hevcMvBufferIndex = GetMvBufferIndex(
        CurrPic.FrameIdx);

    AllocateMvTemporalBuffer(hevcMvBufferIndex);

    hevcRefList[CurrPic.FrameIdx]->ucDMVIdx[0] = hevcMvBufferIndex;

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::DetermineDecodePhase()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t curPhase = HcpDecPhase;
    switch (curPhase)
    {
        case CodechalHcpDecodePhaseInitialized:
            if (bShortFormatInUse)
            {
                HcpDecPhase = CodechalHcpDecodePhaseLegacyS2L;
            }
            else
            {
                HcpDecPhase = CodechalHcpDecodePhaseLegacyLong;
            }
            break;
        case CodechalHcpDecodePhaseLegacyS2L:
            if (!bShortFormatInUse)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("invalid decode phase.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            HcpDecPhase = CodechalHcpDecodePhaseLegacyLong;
            break;
        default:
            CODECHAL_DECODE_ASSERTMESSAGE("invalid decode phase.");
            return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    //HEVC Decode Phase State Machine
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineDecodePhase());

    // Set HEVC Decode Phase, and execute it.
    if (bShortFormatInUse && HcpDecPhase == CodechalHcpDecodePhaseLegacyS2L)
    {
        if (m_secureDecoder)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPictureS2L());
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPictureLongFormat());
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::AddPictureS2LCmds(
    PMOS_COMMAND_BUFFER             cmdBufferInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBufferInUse);

    // Load HuC FW Kernel from WOPCM.
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS hucImemStateParams;
    MOS_ZeroMemory(&hucImemStateParams, sizeof(hucImemStateParams));
    hucImemStateParams.dwKernelDescriptor   = m_hucS2lKernelId;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(
        cmdBufferInUse,
        &hucImemStateParams));

    // Pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    MOS_ZeroMemory(&pipeModeSelectParams, sizeof(pipeModeSelectParams));
    pipeModeSelectParams.Mode               = m_mode;
    pipeModeSelectParams.bStreamOutEnabled  = m_streamOutEnabled;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(
        cmdBufferInUse,
        &pipeModeSelectParams));

    // Indirect object base addr
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode               = m_mode;
    indObjBaseAddrParams.dwDataSize         = bCopyDataBufferInUse ? u32CopyDataBufferSize : u32DataSize;
    indObjBaseAddrParams.dwDataOffset       = bCopyDataBufferInUse ? 0: u32DataOffset;
    indObjBaseAddrParams.presDataBuffer     = bCopyDataBufferInUse ? &resCopyDataBuffer : &resDataBuffer;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetBitstreamBuffer(&indObjBaseAddrParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucIndObjBaseAddrStateCmd(
        cmdBufferInUse,
        &indObjBaseAddrParams));

    // Virtual addr state
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS hucVirtualStateParams;
    MOS_ZeroMemory(&hucVirtualStateParams, sizeof(hucVirtualStateParams));
    hucVirtualStateParams.regionParams[0].presRegion    = &secondLevelBatchBuffer.OsResource;
    hucVirtualStateParams.regionParams[0].isWritable    = true;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(
        cmdBufferInUse,
        &hucVirtualStateParams));

    // DMEM state. Pass data from driver to HuC FW.
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS hucDmemStateParams;
    MOS_ZeroMemory(&hucDmemStateParams, sizeof(hucDmemStateParams));
    hucDmemStateParams.presHucDataSource    = &resDmemBuffer[u32DmemBufferIdx];
    hucDmemStateParams.dwDmemOffset         = HUC_DMEM_OFFSET_RTOS_GEMS;       // If RTOS is GEMS, offset is 0x2000.

    if (!bDmemBufferProgrammed)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetHucDmemParams(&resDmemBuffer[u32DmemBufferIdx]));
        bDmemBufferProgrammed = true;
    }

    hucDmemStateParams.dwDataLength      = u32DmemTransferSize;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(
        cmdBufferInUse,
        &hucDmemStateParams));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(cmdBufferInUse));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SendPictureS2L()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (bEnableSF2DMASubmits)
    {
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((CODECHAL_DECODE_MODE_HUC << 4) & 0xF0) | (m_perfType & 0xF)));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    PMOS_COMMAND_BUFFER cmdBufferInUse = &cmdBuffer;

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureS2LCmds(cmdBufferInUse));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::InitPicLongFormatMhwParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Reset all pic Mhw Params
    MOS_ZeroMemory(m_picMhwParams.PipeModeSelectParams, sizeof(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS ));
    MOS_ZeroMemory(m_picMhwParams.SurfaceParams,        sizeof(MHW_VDBOX_SURFACE_PARAMS          ));
    MOS_ZeroMemory(m_picMhwParams.PipeBufAddrParams,    sizeof(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS    ));
    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.QmParams,             sizeof(MHW_VDBOX_QM_PARAMS               ));
    MOS_ZeroMemory(m_picMhwParams.HevcPicState,         sizeof(MHW_VDBOX_HEVC_PIC_STATE          ));
    MOS_ZeroMemory(m_picMhwParams.HevcTileState,        sizeof(MHW_VDBOX_HEVC_TILE_STATE         ));

    PMOS_SURFACE destSurface = nullptr;
    if (bIs8bitFrameIn10bitHevc)
    {
        destSurface = &sInternalNV12RTSurfaces[u32InternalNV12RTIndexMap[pHevcPicParams->CurrPic.FrameIdx]];
    }
    else
    {
        destSurface = &sDestSurface;
    }

    m_picMhwParams.PipeModeSelectParams->Mode                = m_mode;
    m_picMhwParams.PipeModeSelectParams->bStreamOutEnabled   = m_streamOutEnabled;
    m_picMhwParams.PipeModeSelectParams->bShortFormatInUse   = bShortFormatInUse;

    m_picMhwParams.SurfaceParams->Mode                       = CODECHAL_DECODE_MODE_HEVCVLD;
    m_picMhwParams.SurfaceParams->psSurface                  = destSurface;
    m_picMhwParams.SurfaceParams->ucSurfaceStateId           = CODECHAL_HCP_DECODED_SURFACE_ID;
    m_picMhwParams.SurfaceParams->ChromaType                 = pHevcPicParams->chroma_format_idc;
    m_picMhwParams.SurfaceParams->ucBitDepthLumaMinus8       = pHevcPicParams->bit_depth_luma_minus8;
    m_picMhwParams.SurfaceParams->ucBitDepthChromaMinus8     = pHevcPicParams->bit_depth_chroma_minus8;
    m_picMhwParams.SurfaceParams->dwUVPlaneAlignment         = 1 << (pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

    m_picMhwParams.PipeBufAddrParams->Mode                   = m_mode;
    m_picMhwParams.PipeBufAddrParams->psPreDeblockSurface    = destSurface;

    if (bIs8bitFrameIn10bitHevc)
    {
        m_picMhwParams.PipeBufAddrParams->presP010RTSurface = &sDestSurface;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(m_picMhwParams.PipeBufAddrParams));

    m_picMhwParams.PipeBufAddrParams->presMfdDeblockingFilterRowStoreScratchBuffer       =
        &resMfdDeblockingFilterRowStoreScratchBuffer;
    m_picMhwParams.PipeBufAddrParams->presDeblockingFilterTileRowStoreScratchBuffer      =
        &resDeblockingFilterTileRowStoreScratchBuffer;
    m_picMhwParams.PipeBufAddrParams->presDeblockingFilterColumnRowStoreScratchBuffer    =
        &resDeblockingFilterColumnRowStoreScratchBuffer;

    m_picMhwParams.PipeBufAddrParams->presMetadataLineBuffer         = &resMetadataLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presMetadataTileLineBuffer     = &resMetadataTileLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presMetadataTileColumnBuffer   = &resMetadataTileColumnBuffer;
    m_picMhwParams.PipeBufAddrParams->presSaoLineBuffer              = &resSaoLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presSaoTileLineBuffer          = &resSaoTileLineBuffer;
    m_picMhwParams.PipeBufAddrParams->presSaoTileColumnBuffer        = &resSaoTileColumnBuffer;
    m_picMhwParams.PipeBufAddrParams->presCurMvTempBuffer            = &resMvTemporalBuffer[hevcMvBufferIndex];

    MOS_ZeroMemory(presReferences, sizeof(PMOS_RESOURCE)*CODEC_MAX_NUM_REF_FRAME_HEVC);

    if (!bCurPicIntra)
    {
        uint8_t i, k = 0, m = 0;

        PMOS_RESOURCE firstValidFrame = nullptr;
        PMOS_RESOURCE firstValidMvBuf = nullptr;

        for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (bFrameUsedAsCurRef[i])
            {
                uint8_t refFrameValue = pHevcPicParams->RefFrameList[i].FrameIdx;
                if (refFrameValue < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
                {
                    presReferences[k] = &(pHevcRefList[refFrameValue]->resRefPic);

                    for (uint8_t j = 0; j < CODEC_NUM_HEVC_MV_BUFFERS; j++)
                    {
                        if ((hevcMvList[j].bInUse) &&
                            (hevcMvList[j].u8FrameId == refFrameValue) && 
                            !Mos_ResourceIsNull(&resMvTemporalBuffer[j]))
                        {
                            m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[m++] = &resMvTemporalBuffer[j];
                            if (firstValidMvBuf == nullptr)
                            {
                                firstValidMvBuf = &resMvTemporalBuffer[j];
                            }
                            break;
                        }
                    }

                    if (firstValidFrame == nullptr)
                    {
                        firstValidFrame = presReferences[k];
                    }

                    k++;
                }
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            m_picMhwParams.PipeBufAddrParams->presReferences,
            sizeof(PMOS_RESOURCE)*CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC,
            presReferences,
            sizeof(PMOS_RESOURCE)*CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC));

        CODECHAL_DECODE_ASSERT(k <= 8);
        CODECHAL_DECODE_ASSERT(m <= 8);

        if (firstValidFrame == nullptr)
        {
            firstValidFrame = &destSurface->OsResource;
        }
        if (firstValidMvBuf == nullptr && 
            !Mos_ResourceIsNull(&resMvTemporalBuffer[hevcMvBufferIndex]))
        {
            firstValidMvBuf = &resMvTemporalBuffer[hevcMvBufferIndex];
        }

        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            // error concealment for the unset reference addresses and unset mv buffers
            if (m_picMhwParams.PipeBufAddrParams->presReferences[i] == nullptr)
            {
                m_picMhwParams.PipeBufAddrParams->presReferences[i] = firstValidFrame;
            }
        }

        for (uint32_t n = 0; n < CODEC_NUM_HEVC_MV_BUFFERS; n++)
        {
            if (m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n] == nullptr && 
                !Mos_ResourceIsNull(&resMvTemporalBuffer[n]))
            {
                m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n] = &resMvTemporalBuffer[n];
            }
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(m_picMhwParams.PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));

    m_picMhwParams.IndObjBaseAddrParams->Mode            = m_mode;
    m_picMhwParams.IndObjBaseAddrParams->dwDataSize      = bCopyDataBufferInUse ? u32CopyDataBufferSize : u32DataSize;
    m_picMhwParams.IndObjBaseAddrParams->dwDataOffset    = bCopyDataBufferInUse ? 0: u32DataOffset;
    m_picMhwParams.IndObjBaseAddrParams->presDataBuffer  = bCopyDataBufferInUse ? &resCopyDataBuffer : &resDataBuffer;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetBitstreamBuffer(m_picMhwParams.IndObjBaseAddrParams));
    }

    m_picMhwParams.QmParams->Standard        = CODECHAL_HEVC;
    m_picMhwParams.QmParams->pHevcIqMatrix   = (PMHW_VDBOX_HEVC_QM_PARAMS)pHevcIqMatrixParams;

    m_picMhwParams.HevcPicState->pHevcPicParams  = pHevcPicParams;

    m_picMhwParams.HevcTileState->pHevcPicParams = pHevcPicParams;
    m_picMhwParams.HevcTileState->pTileColWidth  = u16TileColWidth;
    m_picMhwParams.HevcTileState->pTileRowHeight = u16TileRowHeight;

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::AddPictureLongFormatCmds(
    PMOS_COMMAND_BUFFER             cmdBufferInUse,
    PIC_LONG_FORMAT_MHW_PARAMS      *picMhwParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBufferInUse);
    CODECHAL_DECODE_CHK_NULL_RETURN(picMhwParams);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(
        cmdBufferInUse,
        picMhwParams->PipeModeSelectParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
        cmdBufferInUse,
        picMhwParams->SurfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(
        cmdBufferInUse,
        picMhwParams->PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(
        cmdBufferInUse,
        picMhwParams->IndObjBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpQmStateCmd(
        cmdBufferInUse,
        picMhwParams->QmParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(
        cmdBufferInUse,
        picMhwParams->HevcPicState));

    if (pHevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpTileStateCmd(
            cmdBufferInUse,
            picMhwParams->HevcTileState));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SendPictureLongFormat()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (bEnableSF2DMASubmits)
    {
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((CODECHAL_DECODE_MODE_HEVCVLD << 4) & 0xF0) | (m_perfType & 0xF)));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    bool sendPrologWithFrameTracking = false;
    if (bShortFormatInUse)
    {
        sendPrologWithFrameTracking = bEnableSF2DMASubmits;
    }
    else
    {
        sendPrologWithFrameTracking = true;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, 
        sendPrologWithFrameTracking));

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicLongFormatMhwParams());

    CODECHAL_DEBUG_TOOL(
        for (uint16_t n = 0; n < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; n++)
        {
            if (m_picMhwParams.PipeBufAddrParams->presReferences[n])
            {
                MOS_SURFACE dstSurface;
                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.OsResource = *(m_picMhwParams.PipeBufAddrParams->presReferences[n]);
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->wRefIndex = n;
                std::string refSurfName = "RefSurf[" + std::to_string(static_cast<uint32_t>(m_debugInterface->wRefIndex))+"]";
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.c_str()));
            }

            if (m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n])
            {
                m_debugInterface->wRefIndex = n;
                // dump mvdata
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n],
                    CodechalDbgAttr::attrMvData,
                    "DEC",
                    u32MVBufferSize));
            }
        }
    );

    PMOS_COMMAND_BUFFER cmdBufferInUse = &cmdBuffer;

    //Send status report Start
    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(cmdBufferInUse));
    }

    if (m_statusQueryReportingEnabled && bShortFormatInUse &&
        HcpDecPhase == CodechalHcpDecodePhaseLegacyLong)
    {
        uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
                        m_decodeStatusBuf.m_storeDataOffset +
                        sizeof(uint32_t) * 2;

        // Check HuC_STATUS bit15, HW continue if bit15 > 0, otherwise send COND BB END cmd.
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->SendCondBbEndCmd(
            &m_decodeStatusBuf.m_statusBuffer,
            statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusMaskOffset,
            0,
            false,
            cmdBufferInUse));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureLongFormatCmds(cmdBufferInUse, &m_picMhwParams));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SendSliceS2L(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(hevcSliceState);
    CODECHAL_DECODE_CHK_NULL_RETURN(hevcSliceState->pHevcSliceParams);

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AddHucSecureState(
            this,
            hevcSliceState,
            cmdBuffer));
    }

    // Send HuC Stream Obj cmd
    PCODEC_HEVC_SLICE_PARAMS        slc = hevcSliceState->pHevcSliceParams;
    MHW_VDBOX_HUC_STREAM_OBJ_PARAMS hucStreamObjParams;
    MOS_ZeroMemory(&hucStreamObjParams, sizeof(hucStreamObjParams));
    hucStreamObjParams.dwIndStreamInLength  = hevcSliceState->dwLength;
    hucStreamObjParams.bStreamOutEnable     = hevcSliceState->bHucStreamOut ? 1 : 0;
    hucStreamObjParams.dwIndStreamInStartAddrOffset = slc->slice_data_offset;
    hucStreamObjParams.bHucProcessing               = true;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetHucStreamObj(
            &hucStreamObjParams));
    }

    hucStreamObjParams.bStreamInEnable                  = 1;
    hucStreamObjParams.bEmulPreventionByteRemoval       = 1;
    hucStreamObjParams.bStartCodeSearchEngine           = 1;
    hucStreamObjParams.ucStartCodeByte0                 = 0;
    hucStreamObjParams.ucStartCodeByte1                 = 0;
    hucStreamObjParams.ucStartCodeByte2                 = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStreamObjectCmd(
        cmdBuffer,
        &hucStreamObjParams));

    if (m_statusQueryReportingEnabled &&
        hevcSliceState->bLastSlice &&
        !hevcSliceState->bHucStreamOut)
    {
        uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
                                            m_decodeStatusBuf.m_storeDataOffset +
                                            sizeof(uint32_t) * 2;

        // Write HUC_STATUS2 mask
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource         = &m_decodeStatusBuf.m_statusBuffer;
        storeDataParams.dwResourceOffset    = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatus2MaskOffset;
        storeDataParams.dwValue             = m_hucInterface->GetHucStatus2ImemLoadedMask();
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // store HUC_STATUS2 register
        MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer      = &m_decodeStatusBuf.m_statusBuffer;
        storeRegParams.dwOffset             = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatus2RegOffset;
        storeRegParams.dwRegister           = mmioRegisters->hucStatus2RegOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
            cmdBuffer,
            &storeRegParams));
    }

    // Send HuC Start
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(
        cmdBuffer,
        hevcSliceState->bLastSlice ? true : false));

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::SendSliceLongFormat(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(hevcSliceState);
    CODECHAL_DECODE_CHK_NULL_RETURN(hevcSliceState->pHevcSliceParams);

    PCODEC_HEVC_SLICE_PARAMS slc = hevcSliceState->pHevcSliceParams;

    // Disable the flag when ref list is empty for P/B slices to avoid the GPU hang
    if (bCurPicIntra &&
        ! m_hcpInterface->IsHevcISlice(slc->LongSliceFlags.fields.slice_type))
    {
        slc->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag = 0;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSliceStateCmd(
        cmdBuffer,
        hevcSliceState));

    if (! m_hcpInterface->IsHevcISlice(slc->LongSliceFlags.fields.slice_type))
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS refIdxParams;
        refIdxParams.CurrPic = pHevcPicParams->CurrPic;
        refIdxParams.ucList = 0;
        refIdxParams.ucNumRefForList = slc->num_ref_idx_l0_active_minus1 + 1;
        eStatus = MOS_SecureMemcpy(&refIdxParams.RefPicList, sizeof(refIdxParams.RefPicList), &slc->RefPicList, sizeof(slc->RefPicList));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");
        refIdxParams.ppHevcRefList = hevcSliceState->ppHevcRefList;
        refIdxParams.poc_curr_pic = pHevcPicParams->CurrPicOrderCntVal;
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = pHevcPicParams->PicOrderCntValList[i];
        }

        refIdxParams.pRefIdxMapping = hevcSliceState->pRefIdxMapping;
        refIdxParams.RefFieldPicFlag = pHevcPicParams->RefFieldPicFlag;
        refIdxParams.RefBottomFieldFlag = pHevcPicParams->RefBottomFieldFlag;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
            cmdBuffer,
            nullptr,
            &refIdxParams));

        if (m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type))
        {
            refIdxParams.ucList = 1;
            refIdxParams.ucNumRefForList = slc->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
                cmdBuffer,
                nullptr,
                &refIdxParams));
        }
    }


    if ((pHevcPicParams->weighted_pred_flag &&
         m_hcpInterface->IsHevcPSlice(slc->LongSliceFlags.fields.slice_type)) ||
        (pHevcPicParams->weighted_bipred_flag &&
         m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type)))
    {
        MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS weightOffsetParams;

        weightOffsetParams.ucList = 0;

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[0],
            sizeof(weightOffsetParams.LumaWeights[0]),
            &slc->delta_luma_weight_l0,
            sizeof(slc->delta_luma_weight_l0));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[1],
            sizeof(weightOffsetParams.LumaWeights[1]),
            &slc->delta_luma_weight_l1,
            sizeof(slc->delta_luma_weight_l1));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        for (int32_t i = 0; i < 15; i++)
        {
            weightOffsetParams.LumaOffsets[0][i] = (int16_t)slc->luma_offset_l0[i];
            weightOffsetParams.LumaOffsets[1][i] = (int16_t)slc->luma_offset_l1[i];

            for (int32_t j = 0; j < 2; j++)
            {
                weightOffsetParams.ChromaOffsets[0][i][j] = (int16_t)slc->ChromaOffsetL0[i][j];
                weightOffsetParams.ChromaOffsets[1][i][j] = (int16_t)slc->ChromaOffsetL1[i][j];
            }
        }

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[0],
            sizeof(weightOffsetParams.ChromaWeights[0]),
            &slc->delta_chroma_weight_l0,
            sizeof(slc->delta_chroma_weight_l0));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[1],
            sizeof(weightOffsetParams.ChromaWeights[1]),
            &slc->delta_chroma_weight_l1,
            sizeof(slc->delta_chroma_weight_l1));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(
            cmdBuffer,
            nullptr,
            &weightOffsetParams));

        if (m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type))
        {
            weightOffsetParams.ucList = 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(
                cmdBuffer,
                nullptr,
                &weightOffsetParams));
        }
    }

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AddHcpSecureState(
            cmdBuffer,
            hevcSliceState));
    }

    MHW_VDBOX_HCP_BSD_PARAMS bsdParams;
    MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
    bsdParams.dwBsdDataLength = hevcSliceState->dwLength;
    bsdParams.dwBsdDataStartOffset = slc->slice_data_offset + hevcSliceState->dwOffset;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpBsdObjectCmd(
        cmdBuffer,
        &bsdParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);

    uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
                    m_decodeStatusBuf.m_storeDataOffset +
                    sizeof(uint32_t) * 2;

    uint32_t renderingFlags = m_videoContextUsesNullHw;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    PMOS_COMMAND_BUFFER cmdBufferInUse = &cmdBuffer;

    // If S2L and 2nd pass, ...
    // ... jump to 2nd level batch buffer.
    if( (bShortFormatInUse &&
         HcpDecPhase == CodechalHcpDecodePhaseLegacyLong) ||
         m_cencDecoder)
    {
        if (bEnableSF2DMASubmits)
        {
            #if (_DEBUG || _RELEASE_INTERNAL)
            secondLevelBatchBuffer.iLastCurrent = secondLevelBatchBuffer.iSize;
            #endif

            CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                &secondLevelBatchBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "DEC"));
            )
        }

        // Jump to 2nd level batch buffer.
        if(m_cencDecoder)
        {
            // pass the correct 2nd level batch buffer index set in CencDecode()
            uint8_t        sliceBatchBufferIdx;

            sliceBatchBufferIdx = pHevcRefList[CurrPic.FrameIdx]->ucCencBufIdx[0];

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->SetBatchBufferForDecode(
                m_debugInterface,
                sliceBatchBufferIdx,
                cmdBufferInUse));
        }
        else
        {
            // this is S2L conversion
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
                cmdBufferInUse,
                &secondLevelBatchBuffer));
        }
    }
    else
    {
        // Setup static slice state parameters
        MHW_VDBOX_HEVC_SLICE_STATE hevcSliceState;
        MOS_ZeroMemory(&hevcSliceState, sizeof(hevcSliceState));
        hevcSliceState.presDataBuffer               = bCopyDataBufferInUse ? &resCopyDataBuffer : &resDataBuffer;
        hevcSliceState.pHevcPicParams               = pHevcPicParams;
        hevcSliceState.ppHevcRefList                = pHevcRefList;
        hevcSliceState.pRefIdxMapping               = &RefIdxMapping[0];

        PCODEC_HEVC_SLICE_PARAMS slc                = pHevcSliceParams;
        for (uint32_t slcCount = 0; slcCount < u32NumSlices; slcCount++)
        {
            hevcSliceState.pHevcSliceParams = slc;
            hevcSliceState.dwLength         = slc->slice_data_size;
            hevcSliceState.dwSliceIndex     = slcCount;
            hevcSliceState.bLastSlice       = (slcCount == (u32NumSlices-1));

            // If S2L and 1st pass, send HuC commands.
            if (bShortFormatInUse)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(SendSliceS2L(cmdBufferInUse, &hevcSliceState));
            }
            else
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(SendSliceLongFormat(cmdBufferInUse, &hevcSliceState));
            }

            slc++;
        }

        // If S2L and 1st pass
        if (bShortFormatInUse && HcpDecPhase == CodechalHcpDecodePhaseLegacyS2L)
        {
            // Send VD Pipe Flush command for SKL+
            MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
            MOS_ZeroMemory(&vdpipeFlushParams , sizeof(vdpipeFlushParams));
            vdpipeFlushParams.Flags.bWaitDoneHEVC               = 1;
            vdpipeFlushParams.Flags.bFlushHEVC                  = 1;
            vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser     = 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
                cmdBufferInUse,
                &vdpipeFlushParams));

            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
                cmdBufferInUse,
                &flushDwParams));

            if (m_statusQueryReportingEnabled)
            {
                // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
                eStatus = m_hwInterface->SendCondBbEndCmd(
                            &m_decodeStatusBuf.m_statusBuffer, 
                            statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatus2MaskOffset,
                            0, 
                            false,
                            cmdBufferInUse);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);

                // Write HUC_STATUS mask
                MHW_MI_STORE_DATA_PARAMS storeDataParams;
                MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
                storeDataParams.pOsResource         = &m_decodeStatusBuf.m_statusBuffer;
                storeDataParams.dwResourceOffset    = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusMaskOffset;
                storeDataParams.dwValue             = m_hucInterface->GetHucStatusHevcS2lFailureMask();
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                    cmdBufferInUse,
                    &storeDataParams));

                // store HUC_STATUS register
                MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
                MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
                storeRegParams.presStoreBuffer      = &m_decodeStatusBuf.m_statusBuffer;
                storeRegParams.dwOffset             = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusRegOffset;
                storeRegParams.dwRegister           = mmioRegisters->hucStatusRegOffset;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
                    cmdBufferInUse,
                    &storeRegParams));
            }

            if (bEnableSF2DMASubmits)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(
                    &cmdBuffer));

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
                    cmdBufferInUse,
                    nullptr));
            }

            m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

            if (bEnableSF2DMASubmits)
            {
                CODECHAL_DEBUG_TOOL(
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                    cmdBufferInUse,
                    CODECHAL_NUM_MEDIA_STATES,
                    "DEC"));

                //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
                //    m_debugInterface,
                //    cmdBufferInUse));
                );

                //CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
                //    m_osInterface,
                //    cmdBufferInUse,
                //    renderingFlags));
            }

            return eStatus;
        }
    }

    // Send VD Pipe Flush command for SKL+
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams , sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC               = 1;
    vdpipeFlushParams.Flags.bFlushHEVC                  = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser     = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
        cmdBufferInUse,
        &vdpipeFlushParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBufferInUse,
        &flushDwParams));

    MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
    syncParams.GpuContext      = m_videoContext;
    if (bIs8bitFrameIn10bitHevc)
    {
        syncParams.presSyncResource = 
            &sInternalNV12RTSurfaces[u32InternalNV12RTIndexMap[pHevcPicParams->CurrPic.FrameIdx]].OsResource;
    }
    else
    {
        syncParams.presSyncResource         = &sDestSurface.OsResource;
    }
    syncParams.bReadOnly                = false;
    syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    if (!CodecHal_PictureIsField(pHevcPicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

        // Update the resource tag (s/w tag) for On-Demand Sync
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
    }

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
            cmdBufferInUse,
            &syncParams));
    }
    
    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;
        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = pHevcPicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = pHevcPicParams->CurrPic;
        decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
        if(bIs8bitFrameIn10bitHevc)
        {
            decodeStatusReport.m_currDecodedPicRes = sDestSurface.OsResource;
        }
        else
        {
            decodeStatusReport.m_currDecodedPicRes  = pHevcRefList[pHevcPicParams->CurrPic.FrameIdx]->resRefPic;
        }
#ifdef _DECODE_PROCESSING_SUPPORTED
        CODECHAL_DEBUG_TOOL(
            if (m_downsampledSurfaces)
            {
                m_downsampledSurfaces[pHevcPicParams->CurrPic.FrameIdx].OsResource =
                    m_sfcState->pSfcOutputSurface->OsResource;
                decodeStatusReport.m_currSfcOutputPicRes =
                    &m_downsampledSurfaces[pHevcPicParams->CurrPic.FrameIdx].OsResource;
            }
        )
#endif
        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField    = CodecHal_PictureIsBottomField(pHevcPicParams->CurrPic);
            decodeStatusReport.m_frameType      = m_perfType;
        );

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
            decodeStatusReport,
            cmdBufferInUse));
    }

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBufferInUse,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(
        &cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        cmdBufferInUse,
        nullptr));
   
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    bool syncCompleteFrame = bCopyDataBufferInUse;

    if (syncCompleteFrame)
    {
        //Sync up complete frame
        MOS_SYNC_PARAMS copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContextForWa;
        copyDataSyncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &copyDataSyncParams));

        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContext;
        copyDataSyncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &copyDataSyncParams));
    }

    CODECHAL_DEBUG_TOOL(
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                cmdBufferInUse,
                CODECHAL_NUM_MEDIA_STATES,
                "DEC"));

            //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
            //    m_debugInterface,
            //    cmdBufferInUse));
        }
    );

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        cmdBufferInUse,
        renderingFlags));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&sDestSurface);
    )

    // Reset status report
    if (m_statusQueryReportingEnabled)
    {
        bool resetStatusReport = true;

        if (resetStatusReport)
        {
                CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
                    m_videoContextUsesNullHw));
        }
    }

    if (bIs8bitFrameIn10bitHevc)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeNV12ToP010);
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_decodeNV12ToP010->Execute(
            &sInternalNV12RTSurfaces[u32InternalNV12RTIndexMap[pHevcPicParams->CurrPic.FrameIdx]].OsResource,
            &sDestSurface.OsResource));
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    if (bIs8bitFrameIn10bitHevc)
    {
        pHevcRefList[pHevcPicParams->CurrPic.FrameIdx]->resRefPic =
            sInternalNV12RTSurfaces[u32InternalNV12RTIndexMap[pHevcPicParams->CurrPic.FrameIdx]].OsResource;
    }
    else
    {
        pHevcRefList[pHevcPicParams->CurrPic.FrameIdx]->resRefPic =
            sDestSurface.OsResource;
    }

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!CodecHal_PictureIsField(pHevcPicParams->CurrPic))
    {
        MOS_SYNC_PARAMS syncParams      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &sDestSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    // Send Vebox and SFC cmds
    if (m_sfcState->bSfcPipeOut)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->RenderStart());
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::CalcDownsamplingParams(
    void                        *picParams,
    uint32_t                    *refSurfWidth,
    uint32_t                    *refSurfHeight,
    MOS_FORMAT                  *format,
    uint8_t                     *frameIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(picParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(refSurfWidth);
    CODECHAL_DECODE_CHK_NULL_RETURN(refSurfHeight);
    CODECHAL_DECODE_CHK_NULL_RETURN(format);
    CODECHAL_DECODE_CHK_NULL_RETURN(frameIdx);

    PCODEC_HEVC_PIC_PARAMS hevcPicParams = (PCODEC_HEVC_PIC_PARAMS)picParams;

    *refSurfWidth   = 0;
    *refSurfHeight  = 0;
    *format   = Format_NV12;
    *frameIdx        = hevcPicParams->CurrPic.FrameIdx;

    if (m_refSurfaces == nullptr)
    {
        uint32_t                         widthInPix, heightInPix;

        widthInPix      = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (hevcPicParams->PicWidthInMinCbsY);
        heightInPix     = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (hevcPicParams->PicHeightInMinCbsY);

        *refSurfWidth  = MOS_ALIGN_CEIL(widthInPix, 64);
        *refSurfHeight = MOS_ALIGN_CEIL(heightInPix, 64);

        if (bIs10bitHEVC)
        {
            *format = Format_P010;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevc::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeHevc, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevc::AllocateStandard (
    PCODECHAL_SETTINGS          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                         = settings->dwWidth;
    m_height                        = settings->dwHeight;
    bIs10bitHEVC                    = (settings->ucLumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? true : false;
    u8ChromaFormatinProfile         = settings->ucChromaFormat;
    bShortFormatInUse               = (m_cencDecoder == nullptr) && settings->bShortFormatInUse;

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_sfcState = MOS_New(CodechalHevcSfcState);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_sfcState);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif

    MOS_ZeroMemory(&CurrPic, sizeof(CurrPic));

    u32FrameIdx                     = 0;

    if (bShortFormatInUse)
    {
        // Legacy SF has 2 passes, 1st pass is S2L, 2nd pass is HEVC Long decode.
        m_decodePassNum  = 2;

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_SF_2_DMA_SUBMITS_ENABLE_ID,
            &userFeatureData);
        bEnableSF2DMASubmits = userFeatureData.u32Data ? true : false;
    }
    

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    MOS_ZeroMemory(&stateCmdSizeParams, sizeof(stateCmdSizeParams));
    stateCmdSizeParams.bShortFormat    = bShortFormatInUse;
    stateCmdSizeParams.bHucDummyStream = (m_secureDecoder ? m_secureDecoder->IsDummyStreamEnabled() : false);
    
    // Picture Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetHxxStateCommandSize(
            m_mode,
            &m_commandBufferSizeNeeded,
            &m_commandPatchListSizeNeeded,
            &stateCmdSizeParams));

    // Primitive Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetHxxPrimitiveCommandSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        bShortFormatInUse));

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesFixedSizes());

    // Prepare Pic Params
    m_picMhwParams.PipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);
    m_picMhwParams.SurfaceParams        = MOS_New(MHW_VDBOX_SURFACE_PARAMS);
    m_picMhwParams.PipeBufAddrParams    = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS);
    m_picMhwParams.IndObjBaseAddrParams = MOS_New(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS);
    m_picMhwParams.QmParams             = MOS_New(MHW_VDBOX_QM_PARAMS);
    m_picMhwParams.HevcPicState         = MOS_New(MHW_VDBOX_HEVC_PIC_STATE);
    m_picMhwParams.HevcTileState        = MOS_New(MHW_VDBOX_HEVC_TILE_STATE);

    MOS_ZeroMemory(m_picMhwParams.PipeModeSelectParams, sizeof(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.SurfaceParams, sizeof(MHW_VDBOX_SURFACE_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.PipeBufAddrParams, sizeof(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.QmParams, sizeof(MHW_VDBOX_QM_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.HevcPicState, sizeof(MHW_VDBOX_HEVC_PIC_STATE));
    MOS_ZeroMemory(m_picMhwParams.HevcTileState, sizeof(MHW_VDBOX_HEVC_TILE_STATE));

    return eStatus;
}

CodechalDecodeHevc::CodechalDecodeHevc (
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecode(hwInterface, debugInterface, standardInfo),
    bIs8bitFrameIn10bitHevc(false),
    bInternalNV12RTIndexMapInitilized(false),
    u32MfdDeblockingFilterRowStoreScratchBufferPicWidth(0),
    u32MetadataLineBufferPicWidth(0),
    u32SaoLineBufferPicWidth(0),
    u32DmemBufferIdx(0),
    u32CopyDataBufferSize(0),
    bCopyDataBufferInUse(false),
    u32MVBufferSize(0),
    bEnableSF2DMASubmits(false),
    u32WidthLastMaxAlloced(0),
    u32HeightLastMaxAlloced(0),
    u32CtbLog2SizeYMax(0)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(sInternalNV12RTSurfaces,                         sizeof(sInternalNV12RTSurfaces));
    MOS_ZeroMemory(&resDataBuffer,                                  sizeof(resDataBuffer));
    MOS_ZeroMemory(&resMfdDeblockingFilterRowStoreScratchBuffer,    sizeof(resMfdDeblockingFilterRowStoreScratchBuffer));
    MOS_ZeroMemory(&resDeblockingFilterTileRowStoreScratchBuffer,   sizeof(resDeblockingFilterTileRowStoreScratchBuffer));
    MOS_ZeroMemory(&resDeblockingFilterColumnRowStoreScratchBuffer, sizeof(resDeblockingFilterColumnRowStoreScratchBuffer));
    MOS_ZeroMemory(&resMetadataLineBuffer,                          sizeof(resMetadataLineBuffer));
    MOS_ZeroMemory(&resMetadataTileLineBuffer,                      sizeof(resMetadataTileLineBuffer));
    MOS_ZeroMemory(&resMetadataTileColumnBuffer,                    sizeof(resMetadataTileColumnBuffer));
    MOS_ZeroMemory(&resSaoLineBuffer,                               sizeof(resSaoLineBuffer));
    MOS_ZeroMemory(&resSaoTileLineBuffer,                           sizeof(resSaoTileLineBuffer));
    MOS_ZeroMemory(&resSaoTileColumnBuffer,                         sizeof(resSaoTileColumnBuffer));
    MOS_ZeroMemory(resMvTemporalBuffer,                             sizeof(resMvTemporalBuffer));
    MOS_ZeroMemory(resDmemBuffer,                                   sizeof(resDmemBuffer));
    MOS_ZeroMemory(&resCopyDataBuffer,                              sizeof(resCopyDataBuffer));
    MOS_ZeroMemory(&resSyncObjectWaContextInUse,                    sizeof(resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&m_picMhwParams,                                 sizeof(m_picMhwParams));

    m_hcpInUse = true;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeHevc::DumpPicParams(
    PCODEC_HEVC_PIC_PARAMS     picParams,
    void*                      extPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss << "PicWidthInMinCbsY: " << +picParams->PicWidthInMinCbsY << std::endl;
    oss << "PicHeightInMinCbsY: " << +picParams->PicHeightInMinCbsY << std::endl;
    //wFormatAndSequenceInfoFlags
    oss << "chroma_format_idc: " << +picParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag: " << +picParams->separate_colour_plane_flag << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "NoPicReorderingFlag: " << +picParams->NoPicReorderingFlag << std::endl;
    oss << "ReservedBits1: " << +picParams->ReservedBits1 << std::endl;
    oss << "wFormatAndSequenceInfoFlags: " << +picParams->wFormatAndSequenceInfoFlags << std::endl;
    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;
    oss << "sps_max_dec_pic_buffering_minus1: " << +picParams->sps_max_dec_pic_buffering_minus1 << std::endl;
    oss << "log2_min_luma_coding_block_size_minus3: " << +picParams->log2_min_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_luma_coding_block_size: " << +picParams->log2_diff_max_min_luma_coding_block_size << std::endl;
    oss << "log2_min_transform_block_size_minus2: " << +picParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "log2_diff_max_min_transform_block_size: " << +picParams->log2_diff_max_min_transform_block_size << std::endl;
    oss << "max_transform_hierarchy_depth_intra: " << +picParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter: " << +picParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "num_short_term_ref_pic_sets: " << +picParams->num_short_term_ref_pic_sets << std::endl;
    oss << "num_long_term_ref_pic_sps: " << +picParams->num_long_term_ref_pic_sps << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1: " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1: " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "init_qp_minus26: " << +picParams->init_qp_minus26 << std::endl;
    oss << "ucNumDeltaPocsOfRefRpsIdx: " << +picParams->ucNumDeltaPocsOfRefRpsIdx << std::endl;
    oss << "wNumBitsForShortTermRPSInSlice: " << +picParams->wNumBitsForShortTermRPSInSlice << std::endl;
    oss << "ReservedBits2: " << +picParams->ReservedBits2 << std::endl;
    //dwCodingParamToolFlags
    oss << "scaling_list_enabled_flag: " << +picParams->scaling_list_enabled_flag << std::endl;
    oss << "amp_enabled_flag: " << +picParams->amp_enabled_flag << std::endl;
    oss << "sample_adaptive_offset_enabled_flag: " << +picParams->sample_adaptive_offset_enabled_flag << std::endl;
    oss << "pcm_enabled_flag: " << +picParams->pcm_enabled_flag << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1: " << +picParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1: " << +picParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "log2_min_pcm_luma_coding_block_size_minus3: " << +picParams->log2_min_pcm_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_pcm_luma_coding_block_size: " << +picParams->log2_diff_max_min_pcm_luma_coding_block_size << std::endl;
    oss << "pcm_loop_filter_disabled_flag: " << +picParams->pcm_loop_filter_disabled_flag << std::endl;
    oss << "long_term_ref_pics_present_flag: " << +picParams->long_term_ref_pics_present_flag << std::endl;
    oss << "sps_temporal_mvp_enabled_flag: " << +picParams->sps_temporal_mvp_enabled_flag << std::endl;
    oss << "strong_intra_smoothing_enabled_flag: " << +picParams->strong_intra_smoothing_enabled_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag: " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "output_flag_present_flag: " << +picParams->output_flag_present_flag << std::endl;
    oss << "num_extra_slice_header_bits: " << +picParams->num_extra_slice_header_bits << std::endl;
    oss << "sign_data_hiding_enabled_flag: " << +picParams->sign_data_hiding_enabled_flag << std::endl;
    oss << "cabac_init_present_flag: " << +picParams->cabac_init_present_flag << std::endl;
    oss << "ReservedBits3: " << +picParams->ReservedBits3 << std::endl;
    oss << "dwCodingParamToolFlags: " << +picParams->dwCodingParamToolFlags << std::endl;
    //dwCodingSettingPicturePropertyFlags
    oss << "constrained_intra_pred_flag: " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag: " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag: " << +picParams->cu_qp_delta_enabled_flag << std::endl;
    oss << "diff_cu_qp_delta_depth: " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_slice_chroma_qp_offsets_present_flag: " << +picParams->pps_slice_chroma_qp_offsets_present_flag << std::endl;
    oss << "weighted_pred_flag: " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag: " << +picParams->weighted_bipred_flag << std::endl;
    oss << "transquant_bypass_enabled_flag: " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "tiles_enabled_flag: " << +picParams->tiles_enabled_flag << std::endl;
    oss << "entropy_coding_sync_enabled_flag: " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "uniform_spacing_flag: " << +picParams->uniform_spacing_flag << std::endl;
    oss << "loop_filter_across_tiles_enabled_flag: " << +picParams->loop_filter_across_tiles_enabled_flag << std::endl;
    oss << "pps_loop_filter_across_slices_enabled_flag: " << +picParams->pps_loop_filter_across_slices_enabled_flag << std::endl;
    oss << "deblocking_filter_override_enabled_flag: " << +picParams->deblocking_filter_override_enabled_flag << std::endl;
    oss << "pps_deblocking_filter_disabled_flag: " << +picParams->pps_deblocking_filter_disabled_flag << std::endl;
    oss << "lists_modification_present_flag: " << +picParams->lists_modification_present_flag << std::endl;
    oss << "slice_segment_header_extension_present_flag: " << +picParams->slice_segment_header_extension_present_flag << std::endl;
    oss << "IrapPicFlag: " << +picParams->IrapPicFlag << std::endl;
    oss << "IdrPicFlag: " << +picParams->IdrPicFlag << std::endl;
    oss << "IntraPicFlag: " << +picParams->IntraPicFlag << std::endl;
    oss << "ReservedBits4: " << +picParams->ReservedBits4 << std::endl;
    oss << "dwCodingSettingPicturePropertyFlags: " << +picParams->dwCodingSettingPicturePropertyFlags << std::endl;
    oss << "pps_cb_qp_offset: " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset: " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1: " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1: " << +picParams->num_tile_rows_minus1 << std::endl;
    //Dump column width
    oss << "column_width_minus1[19]:";
    for (uint8_t i = 0; i < 19; i++)
        oss << picParams->column_width_minus1[i] << " ";
    oss << std::endl;

    //Dump row height
    oss << "row_height_minus1[21]:";
    for (uint8_t i = 0; i < 21; i++)
        oss << picParams->row_height_minus1[i] << " ";
    oss << std::endl;

    oss << "pps_beta_offset_div2: " << +picParams->pps_beta_offset_div2 << std::endl;
    oss << "pps_tc_offset_div2: " << +picParams->pps_tc_offset_div2 << std::endl;
    oss << "log2_parallel_merge_level_minus2: " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "CurrPicOrderCntVal: " << +picParams->CurrPicOrderCntVal << std::endl;

    oss.setf(std::ios::dec, std::ios::basefield);
    //Dump RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    //Dump POC List
    oss << "PicOrderCntValList[15]:";
    for (uint8_t i = 0; i < 15; i++)
        oss << std::hex << picParams->PicOrderCntValList[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List
    oss << "RefPicSetStCurrBefore[8]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetStCurrBefore[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List
    oss << "RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetStCurrAfter[i] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List
    oss << "RefPicSetLtCurr[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetLtCurr[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List with POC
    oss << "RefPicSetStCurrBefore[8] (POC): ";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetStCurrBefore[i]] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List with POC
    oss << "RefPicSetStCurrAfter[16] (POC):";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetStCurrAfter[i]] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List with POC
    oss << "RefPicSetLtCurr[16] (POC): ";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetLtCurr[i]] << " ";
    oss << std::endl;

    oss << "RefFieldPicFlag: " << +picParams->RefFieldPicFlag << std::endl;
    oss << "RefBottomFieldFlag: " << +picParams->RefBottomFieldFlag << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevc::DumpSliceParams(
    PCODEC_HEVC_SLICE_PARAMS     sliceParams,
    void*                        extSliceParams,
    uint32_t                     numSlices,
    bool                         shortFormatInUse)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    PCODEC_HEVC_SLICE_PARAMS     hevcSliceControl    = nullptr;

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint16_t j = 0; j < numSlices; j++)
    {
        hevcSliceControl = &sliceParams[j];

        oss << "====================================================================================================" << std::endl;
        oss << "Data for Slice number = " << +j << std::endl;
        oss << "slice_data_size: " << +hevcSliceControl->slice_data_size << std::endl;
        oss << "slice_data_offset: " << +hevcSliceControl->slice_data_offset << std::endl;

        if (!shortFormatInUse)
        {
            //Dump Long format specific
            oss << "ByteOffsetToSliceData: " << +hevcSliceControl->ByteOffsetToSliceData << std::endl;
            oss << "slice_segment_address: " << +hevcSliceControl->slice_segment_address << std::endl;

            //Dump RefPicList[2][15]
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[0][" << +i << "]";
                oss << "FrameIdx: " << +hevcSliceControl->RefPicList[0][i].FrameIdx;
                oss << ", PicFlags: " << +hevcSliceControl->RefPicList[0][i].PicFlags;
                oss << std::endl;
            }
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[1][" << +i << "]";
                oss << "FrameIdx: " << +hevcSliceControl->RefPicList[1][i].FrameIdx;
                oss << ", PicFlags: " << +hevcSliceControl->RefPicList[1][i].PicFlags;
                oss << std::endl;
            }

            oss << "last_slice_of_pic: " << +hevcSliceControl->LongSliceFlags.fields.LastSliceOfPic << std::endl;
            oss << "dependent_slice_segment_flag: " << +hevcSliceControl->LongSliceFlags.fields.dependent_slice_segment_flag << std::endl;
            oss << "slice_type: " << +hevcSliceControl->LongSliceFlags.fields.slice_type << std::endl;
            oss << "color_plane_id: " << +hevcSliceControl->LongSliceFlags.fields.color_plane_id << std::endl;
            oss << "slice_sao_luma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_luma_flag << std::endl;
            oss << "slice_sao_chroma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_chroma_flag << std::endl;
            oss << "mvd_l1_zero_flag: " << +hevcSliceControl->LongSliceFlags.fields.mvd_l1_zero_flag << std::endl;
            oss << "cabac_init_flag: " << +hevcSliceControl->LongSliceFlags.fields.cabac_init_flag << std::endl;
            oss << "slice_temporal_mvp_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag << std::endl;
            oss << "slice_deblocking_filter_disabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag << std::endl;
            oss << "collocated_from_l0_flag: " << +hevcSliceControl->LongSliceFlags.fields.collocated_from_l0_flag << std::endl;
            oss << "slice_loop_filter_across_slices_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag << std::endl;
            oss << "reserved: " << +hevcSliceControl->LongSliceFlags.fields.reserved << std::endl;
            oss << "collocated_ref_idx: " << +hevcSliceControl->collocated_ref_idx << std::endl;
            oss << "num_ref_idx_l0_active_minus1: " << +hevcSliceControl->num_ref_idx_l0_active_minus1 << std::endl;
            oss << "num_ref_idx_l1_active_minus1: " << +hevcSliceControl->num_ref_idx_l1_active_minus1 << std::endl;
            oss << "slice_qp_delta: " << +hevcSliceControl->slice_qp_delta << std::endl;
            oss << "slice_cb_qp_offset: " << +hevcSliceControl->slice_cb_qp_offset << std::endl;
            oss << "slice_cr_qp_offset: " << +hevcSliceControl->slice_cr_qp_offset << std::endl;
            oss << "slice_beta_offset_div2: " << +hevcSliceControl->slice_beta_offset_div2 << std::endl;
            oss << "slice_tc_offset_div2: " << +hevcSliceControl->slice_tc_offset_div2 << std::endl;
            oss << "luma_log2_weight_denom: " << +hevcSliceControl->luma_log2_weight_denom << std::endl;
            oss << "delta_chroma_log2_weight_denom: " << +hevcSliceControl->delta_chroma_log2_weight_denom << std::endl;

            //Dump luma_offset[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "luma_offset_l0[" << +i << "]: " << (+hevcSliceControl->luma_offset_l0[i]) << std::endl;
                oss << "luma_offset_l1[" << +i << "]: " << (+hevcSliceControl->luma_offset_l1[i]) << std::endl;
            }
            //Dump delta_luma_weight[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_luma_weight_l0[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
                oss << "delta_luma_weight_l1[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
            }
            //Dump chroma_offset[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "ChromaOffsetL0[" << +i << "][0]: " << (+hevcSliceControl->ChromaOffsetL0[i][0]) << std::endl;

                oss << "ChromaOffsetL0[" << +i << "][1]: " << (+hevcSliceControl->ChromaOffsetL0[i][1]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][0]: " << (+hevcSliceControl->ChromaOffsetL1[i][0]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][1]: " << (+hevcSliceControl->ChromaOffsetL1[i][1]) << std::endl;
            }
            //Dump delta_chroma_weight[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_chroma_weight_l0[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l0[i][0] << std::endl;
                oss << "delta_chroma_weight_l0[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l0[i][1] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l1[i][0] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l1[i][1] << std::endl;
            }

            oss << "five_minus_max_num_merge_cand: " << +hevcSliceControl->five_minus_max_num_merge_cand << std::endl;

        }

        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSlcParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs;
        if (j == 0)
        {
            ofs.open(fileName, std::ios::out);
        }
        else
        {
            ofs.open(fileName, std::ios::app);
        }
        ofs << oss.str();
        ofs.close();
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevc::DumpIQParams(
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    uint32_t idx;
    uint32_t idx2;
    // 4x4 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "Qmatrix_HEVC_ucScalingLists0[" << std::dec << +idx2 << "]:" << std::endl;

        oss << "ucScalingLists0[" << +idx2 << "]:";
        for (uint8_t i = 0; i < 16; i++)
            oss << std::hex << +matrixData->ucScalingLists0[idx2][i] << " ";
        oss << std::endl;
    }

    // 8x8 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "ucScalingLists1[" << std::dec << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 56; idx += 8)
        {
            oss << "ucScalingLists1[" << std::dec << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << std::hex << +matrixData->ucScalingLists1[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    // 16x16 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "ucScalingLists2[" << std::dec << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 56; idx += 8)
        {
            oss << "ucScalingLists2[" << std::dec << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << std::hex << +matrixData->ucScalingLists2[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }
    // 32x32 block
    for (idx2 = 0; idx2 < 2; idx2++)
    {
        oss << "ucScalingLists3[" << std::dec << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 56; idx += 8)
        {
            oss << "ucScalingLists3[" << std::dec << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << std::hex << +matrixData->ucScalingLists3[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    //DC16x16 block
    oss << "ucScalingListDCCoefSizeID2: ";
    for (uint8_t i = 0; i < 6; i++)
        oss << std::hex << +matrixData->ucScalingListDCCoefSizeID2[i] << " ";

    oss << std::endl;

    //DC32x32 block

    oss << "ucScalingListDCCoefSizeID3: ";
    oss << +matrixData->ucScalingListDCCoefSizeID3[0] << " " << +matrixData->ucScalingListDCCoefSizeID3[1] << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    
    return MOS_STATUS_SUCCESS;
}

#endif
