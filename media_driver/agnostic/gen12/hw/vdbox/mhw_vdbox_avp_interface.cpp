/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     mhw_vdbox_avp_interface.cpp
//! \brief    MHW interface for constructing AVP commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox AVP commands across all platforms
//!

#include "mhw_vdbox_avp_interface.h"

void MhwVdboxAvpPipeBufAddrParams::Initialize()
{
    m_mode = 0;

    for (auto i = 0; i < 8; i++)
    {
        m_references[i] = nullptr;
    }

    m_decodedPic                      = nullptr;  //!< Decoded Output Frame Buffer
    m_intrabcDecodedOutputFrameBuffer = nullptr;  //!< IntraBC Decoded output frame buffer
    m_cdfTableInitializationBuffer    = nullptr;  //!< CDF Tables Initialization Buffer
    m_cdfTableBwdAdaptationBuffer     = nullptr;  //!< CDF Tables Backward Adaptation Buffer

    m_segmentIdReadBuffer                                    = nullptr;  //!< AV1 Segment ID Read Buffer
    m_segmentIdWriteBuffer                                   = nullptr;  //!< AV1 Segment ID Write Buffer

    for (auto i = 0; i < 9; i++)
    {
        m_colMvTemporalBuffer[i] = nullptr;
    }

    m_curMvTemporalBuffer                                    = nullptr;  //!< Current MV temporal buffer
    m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer     = nullptr;  //!< Handle of Bitstream Decode Line Rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
    m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = nullptr;  //!< Handle of Bitstream Decode Tile Line buffer
    m_intraPredictionLineRowstoreReadWriteBuffer             = nullptr;  //!< Handle of Intra Prediction Line Rowstore Read/Write Buffer
    m_intraPredictionTileLineRowstoreReadWriteBuffer         = nullptr;  //!< Handle of Intra Prediction Tile Line Rowstore Read/Write Buffer
    m_spatialMotionVectorLineReadWriteBuffer                 = nullptr;  //!< Handle of Spatial Motion Vector Line rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
    m_spatialMotionVectorCodingTileLineReadWriteBuffer       = nullptr;  //!< Handle of Spatial Motion Vector Tile Line buffer
    m_loopRestorationMetaTileColumnReadWriteBuffer           = nullptr;  //!< Loop Restoration Meta Tile Column Read/Write Buffer Address
    m_loopRestorationFilterTileReadWriteLineYBuffer          = nullptr;  //!< Loop Restoration Filter Tile Read/Write Line Y Buffer Address
    m_loopRestorationFilterTileReadWriteLineUBuffer          = nullptr;  //!< Loop Restoration Filter Tile Read/Write Line U Buffer Address
    m_loopRestorationFilterTileReadWriteLineVBuffer          = nullptr;  //!< Loop Restoration Filter Tile Read/Write Line V Buffer Address
    m_deblockerFilterLineReadWriteYBuffer                    = nullptr;  //!< Deblocker Filter Line Read/Write Y Buffer Address
    m_deblockerFilterLineReadWriteUBuffer                    = nullptr;  //!< Deblocker Filter Line Read/Write U Buffer Address
    m_deblockerFilterLineReadWriteVBuffer                    = nullptr;  //!< Deblocker Filter Line Read/Write V Buffer Address
    m_deblockerFilterTileLineReadWriteYBuffer                = nullptr;  //!< Deblocker Filter Tile Line Read/Write Y Buffer Address
    m_deblockerFilterTileLineReadWriteVBuffer                = nullptr;  //!< Deblocker Filter Tile Line Read/Write V Buffer Address
    m_deblockerFilterTileLineReadWriteUBuffer                = nullptr;  //!< Deblocker Filter Tile Line Read/Write U Buffer Address
    m_deblockerFilterTileColumnReadWriteYBuffer              = nullptr;  //!< Deblocker Filter Tile Column Read/Write Y Buffer Address
    m_deblockerFilterTileColumnReadWriteUBuffer              = nullptr;  //!< Deblocker Filter Tile Column Read/Write U Buffer Address
    m_deblockerFilterTileColumnReadWriteVBuffer              = nullptr;  //!< Deblocker Filter Tile Column Read/Write V Buffer Address
    m_cdefFilterLineReadWriteBuffer                          = nullptr;  //!< CDEF Filter Line Read/Write Y Buffer Address
    m_cdefFilterTileLineReadWriteBuffer                      = nullptr;  //!< CDEF Filter Tile Line Read/Write Y Buffer Address
    m_cdefFilterTileColumnReadWriteBuffer                    = nullptr;  //!< CDEF Filter Tile Column Read/Write Y Buffer Address
    m_cdefFilterMetaTileLineReadWriteBuffer                  = nullptr;  //!< CDEF Filter Meta Tile Line Read/Write Buffer Address
    m_cdefFilterMetaTileColumnReadWriteBuffer                = nullptr;  //!< CDEF Filter Meta Tile Column Read/Write Buffer Address
    m_cdefFilterTopLeftCornerReadWriteBuffer                 = nullptr;  //!< CDEF Filter Top-Left Corner Read/Write Buffer Address
    m_superResTileColumnReadWriteYBuffer                     = nullptr;  //!< Super-Res Tile Column Read/Write Y Buffer Address
    m_superResTileColumnReadWriteUBuffer                     = nullptr;  //!< Super-Res Tile Column Read/Write U Buffer Address
    m_superResTileColumnReadWriteVBuffer                     = nullptr;  //!< Super-Res Tile Column Read/Write V Buffer Address
    m_loopRestorationFilterTileColumnReadWriteYBuffer        = nullptr;  //!< Loop Restoration Filter Tile Column Read/Write Y Buffer Address
    m_loopRestorationFilterTileColumnReadWriteUBuffer        = nullptr;  //!< Loop Restoration Filter Tile Column Read/Write U Buffer Address
    m_loopRestorationFilterTileColumnReadWriteVBuffer        = nullptr;  //!< Loop Restoration Filter Tile Column Read/Write V Buffer Address
    m_decodedFrameStatusErrorBuffer                          = nullptr;  //!< Decoded Frame Status/Error Buffer Base Address
    m_decodedBlockDataStreamoutBuffer                        = nullptr;  //!< Decoded Block Data Streamout Buffer Address

    //MMC supported
    m_preDeblockSurfMmcState = {};
    m_streamOutBufMmcState = {};
}

MOS_STATUS MhwVdboxAvpInterface::AddAvpSurfaceCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS            params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddAvpDecodeSurfaceStateCmd(cmdBuffer, params));
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    return MOS_STATUS_SUCCESS;
}

MhwVdboxAvpInterface::MhwVdboxAvpInterface(
    PMOS_INTERFACE osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool decodeInUse)
{
    MHW_FUNCTION_ENTER;

    m_osInterface = osInterface;
    m_miInterface = miInterface;
    m_cpInterface = cpInterface;
    m_decodeInUse = decodeInUse;

    MHW_ASSERT(m_osInterface);
    MHW_ASSERT(m_miInterface);
    MHW_ASSERT(m_cpInterface);

    m_waTable = osInterface->pfnGetWaTable(osInterface);
    m_skuTable = osInterface->pfnGetSkuTable(osInterface);

    if (m_osInterface->bUsesGfxAddress)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else // bUsesPatchList
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }

}
