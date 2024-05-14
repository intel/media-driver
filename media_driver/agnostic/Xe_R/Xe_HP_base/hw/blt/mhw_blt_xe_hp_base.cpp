/*
* Copyright (c) 2020 - 2021, Intel Corporation
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
//! \file     mhw_blt_xe_hp_base.cpp
//! \brief    MHW interface for constructing commands for the BLT
//!
#include "mhw_blt_xe_hp_base.h"

mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::XY_BLOCK_COPY_BLT_CMD()
{
    DW0.Value                                            = 0x50400014;
    //DW0.DwordLength                                    = DWORD_LENGTH_EXCLUDES_DWORD_0_1;
    //DW0.NumberOfMultisamples                           = NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_1;
    //DW0.SpecialModeofOperation                         = SPECIAL_MODE_OF_OPERATION_NONE;
    //DW0.ColorDepth                                     = COLOR_DEPTH_8_BIT_COLOR;
    //DW0.InstructionTarget_Opcode                       = 0x0;
    //DW0.Client                                         = CLIENT_2D_PROCESSOR;
    DW1.Value                                            = 0x0;
    //DW1.DestinationPitch                               = 0x0;
    //DW1.DestinationAuxiliarysurfacemode                = DESTINATION_AUXILIARY_SURFACE_MODE_AUX_NONE;
    //DW1.DestinationMOCSvalue                           = 0x0;
    //DW1.DestinationControlSurfaceType                  = DESTINATION_CONTROL_SURFACE_TYPE_3D_CONTROL_SURFACE;
    //DW1.DestinationCompressionEnable                   = DESTINATION_COMPRESSION_ENABLE_COMPRESSION_DISABLE;
    //DW1.DestinationTiling                              = DESTINATION_TILING_LINEAR;
    DW2.Value                                            = 0x0;
    //DW2.DestinationX1Coordinate_Left                   = 0x0;
    //DW2.DestinationY1Coordinate_Top                    = 0x0;
    DW3.Value                                            = 0x0;
    //DW3.DestinationX2Coordinate_Right                  = 0x0;
    //DW3.DestinationY2Coordinate_Bottom                 = 0x0;
    DW4_5.Value[0] = DW4_5.Value[1]                      = 0x0;
    //DW4_5.DestinationBaseAddress                         = 0x0;
    DW6.Value                                            = 0x0;
    //DW6.DestinationXoffset                             = 0x0;
    //DW6.DestinationYoffset                             = 0x0;
    //DW6.DestinationTargetMemory                        = DESTINATION_TARGET_MEMORY_LOCAL_MEM;
    DW7.Value                                            = 0x0;
    //DW7.SourceX1Coordinate_Left                        = 0x0;
    //DW7.SourceY1Coordinate_Top                         = 0x0;
    DW8.Value                                            = 0x0;
    //DW8.SourcePitch                                    = 0x0;
    //DW8.SourceAuxiliarysurfacemode                     = SOURCE_AUXILIARY_SURFACE_MODE_AUX_NONE;
    //DW8.SourceMOCS                                     = 0x0;
    //DW8.SourceControlSurfaceType                       = SOURCE_CONTROL_SURFACE_TYPE_3D_CONTROL_SURFACE;
    //DW8.SourceCompressionEnable                        = SOURCE_COMPRESSION_ENABLE_COMPRESSION_DISABLE;
    //DW8.SourceTiling                                   = SOURCE_TILING_LINEAR;
    DW9_10.Value[0] = DW9_10.Value[1]                    = 0x0;
    //DW9.SourceBaseAddress                              = 0x0;
    DW11.Value                                            = 0x0;
    //DW11.SourceXoffset                                  = 0x0;
    //DW11.SourceYoffset                                  = 0x0;
    //DW11.SourceTargetMemory                             = SOURCE_TARGET_MEMORY_LOCAL_MEM;
    DW12.Value                                            = 0x0;
    //DW12.SourceCompressionFormat                        = 0x0;
    //DW12.SourceClearValueEnable                         = SOURCE_CLEAR_VALUE_ENABLE_DISABLE;
    //DW12.SourceClearAddressLow                          = 0x0;
    DW13.Value                                            = 0x0;
    //DW13.SourceClearAddressHigh                         = 0x0;
    DW14.Value                                            = 0x0;
    //DW14.DestinationCompressionFormat                   = 0x0;
    //DW14.DestinationClearValueEnable                    = DESTINATION_CLEAR_VALUE_ENABLE_DISABLE;
    //DW14.DestinationClearAddressLow                     = 0x0;
    DW15.Value                                            = 0x0;
    //DW15.DestinationClearAddressHigh                    = 0x0;
    DW16.Value = 0x0;
    //DW16.DestinationSurfaceHeight                       = 0x0;
    //DW16.DestinationSurfaceWidth                        = 0x0;
    //DW16.DestinationSurfaceType                         = DESTINATION_SURFACE_TYPE_SURFTYPE_1D;
    DW17.Value                                            = 0x0;
    //DW17.DestinationLOD                                 = 0x0;
    //DW17.DestinationSurfaceQpitch                       = 0x0;
    //DW17.DestinationSurfaceDepth                        = 0x0;
    DW18.Value                                            = 0x0;
    //DW18.DestinationHorizontalAlign                     = 0x0;
    //DW18.DestinationVerticalAlign                       = 0x0;
    //DW18.DestinationMipTailStartLOD                     = 0x0;
    //DW18.DestinationDepthStencilResource                = 0x0;
    //DW18.DestinationArrayIndex                          = 0x0;
    DW19.Value                                            = 0x0;
    //DW19.SourceSurfaceHeight                            = 0x0;
    //DW19.SourceSurfaceWidth                             = 0x0;
    //DW19.SourceSurfaceType                              = SOURCE_SURFACE_TYPE_SURFTYPE_1D;
    DW20.Value                                            = 0x0;
    //DW20.SourceLOD                                      = 0x0;
    //DW20.SourceSurfaceQpitch                            = 0x0;
    //DW20.SourceSurfaceDepth                             = 0x0;
    DW21.Value                                            = 0x0;
    //DW21.SourceHorizontalAlign                          = 0x0;
    //DW21.SourceVerticalAlign                            = 0x0;
    //DW21.SourceMipTailStartLOD                          = 0x0;
    //DW21.SourceDepthStencilResource                     = 0x0;
    //DW21.SourceArrayIndex                               = 0x0;
}

mhw_blt_state_xe_hp_base::XY_CTRL_SURF_COPY_BLT_CMD::XY_CTRL_SURF_COPY_BLT_CMD()
{
    DW0.Value                                        = 0x52000003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.DestinationMemoryType                        = DESTINATION_MEMORY_TYPE_LOCALMEM;
    //DW0.SourceMemoryType                             = SOURCE_MEMORY_TYPE_LOCALMEM;
    //DW0.InstructionTargetOpcode                      = INSTRUCTION_TARGETOPCODE_UNNAMED72;
    //DW0.Client                                       = CLIENT_2DPROCESSOR;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

}

mhw_blt_state_xe_hp_base::BCS_SWCTRL_CMD::BCS_SWCTRL_CMD()
{
    DW0.Value                                        = 0;
    DW0.Tile4Source                                  = TILE_4_SOURCE_XMAJOR;
    DW0.Tile4Destination                             = TILE_4_DESTINATION_XMAJOR;
    DW0.SystemMemoryThrottleThreshold                = 0x40;
    DW0.Mask                                         = 0x0;
}

MhwBltInterfaceXe_Hp_Base::MhwBltInterfaceXe_Hp_Base(PMOS_INTERFACE pOsInterface):
    MhwBltInterface(pOsInterface)
{

}


MOS_STATUS MhwBltInterfaceXe_Hp_Base::AddCtrlSurfCopyBlt(
    PMOS_COMMAND_BUFFER           pCmdBuffer,
    PMHW_CTRL_SURF_COPY_BLT_PARAM pCtrlSurfCopyBltParam)
{
    MHW_FUNCTION_ENTER;

    MHW_RESOURCE_PARAMS ResourceParams;
    MOS_STATUS          eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    mhw_blt_state_xe_hp_base::XY_CTRL_SURF_COPY_BLT_CMD cmd;

    cmd.DW0.SourceMemoryType         = pCtrlSurfCopyBltParam->dwSrcMemoryType;
    cmd.DW0.DestinationMemoryType    = pCtrlSurfCopyBltParam->dwDstMemoryType;
    cmd.DW0.SizeOfControlSurfaceCopy = pCtrlSurfCopyBltParam->dwSizeofControlSurface;

    MHW_CHK_NULL(m_osInterface);
    cmd.DW2.SourceMocs = (m_osInterface->pfnCachePolicyGetMemoryObject(
            MOS_MP_RESOURCE_USAGE_DEFAULT,
            m_osInterface->pfnGetGmmClientContext(m_osInterface))).DwordValue;
    cmd.DW4.DestinationMocs = (m_osInterface->pfnCachePolicyGetMemoryObject(
            MOS_MP_RESOURCE_USAGE_DEFAULT,
            m_osInterface->pfnGetGmmClientContext(m_osInterface))).DwordValue;


    // add source address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    InitMocsParams(ResourceParams, &cmd.DW2.Value, 26, 31);
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.presResource    = pCtrlSurfCopyBltParam->pSrcOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW1.Value);
    ResourceParams.dwLocationInCmd = 1;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    // add destination address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    InitMocsParams(ResourceParams, &cmd.DW4.Value, 26, 31);
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.presResource    = pCtrlSurfCopyBltParam->pDstOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW3.Value);
    ResourceParams.dwLocationInCmd = 3;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwBltInterfaceXe_Hp_Base::AddBlockCopyBlt(
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_FAST_COPY_BLT_PARAM pFastCopyBltParam,
    uint32_t                 srcOffset,
    uint32_t                 dstOffset)
{
    MHW_FUNCTION_ENTER;

    MHW_RESOURCE_PARAMS ResourceParams;
    MOS_STATUS          eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD cmd;

    MHW_CHK_NULL_RETURN(m_osInterface);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pSrcOsResource);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pSrcOsResource->pGmmResInfo);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pDstOsResource);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pDstOsResource->pGmmResInfo);

    BLT_TILE_TYPE dstTiledMode = static_cast<BLT_TILE_TYPE>(pFastCopyBltParam->pDstOsResource->pGmmResInfo->GetTileType());
    BLT_TILE_TYPE srcTiledMode = static_cast<BLT_TILE_TYPE>(pFastCopyBltParam->pSrcOsResource->pGmmResInfo->GetTileType());

    PGMM_RESOURCE_INFO  pSrcGmmResInfo = pFastCopyBltParam->pSrcOsResource->pGmmResInfo;
    PGMM_RESOURCE_INFO  pDstGmmResInfo = pFastCopyBltParam->pDstOsResource->pGmmResInfo;
    uint32_t sourceResourceWidth       = (uint32_t)pSrcGmmResInfo->GetBaseWidth();
    uint32_t sourceResourceHeight      = (uint32_t)pSrcGmmResInfo->GetBaseHeight();
    uint32_t dstResourceWidth          = (uint32_t)pDstGmmResInfo->GetBaseWidth();
    uint32_t dstResourceHeight         = (uint32_t)pDstGmmResInfo->GetBaseHeight();

    cmd.DW0.InstructionTargetOpcode = 0x41;
    cmd.DW0.ColorDepth = pFastCopyBltParam->dwColorDepth;
    cmd.DW1.DestinationPitch = pFastCopyBltParam->dwDstPitch -1;
    cmd.DW1.DestinationMocSvalue = 
        m_osInterface->pfnGetGmmClientContext(m_osInterface)->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_BLT_DESTINATION).DwordValue;

    cmd.DW1.DestinationControlSurfaceType = 1;// 1 is media; 0 is 3D;
    cmd.DW1.DestinationTiling             = GetBlockTilingMode(dstTiledMode);
    cmd.DW8.SourceControlSurfaceType      = 1; // 1 is media; 0 is 3D;
    cmd.DW8.SourceTiling = GetBlockTilingMode(srcTiledMode);
    cmd.DW8.SourceMocs =
        m_osInterface->pfnGetGmmClientContext(m_osInterface)->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_BLT_SOURCE).DwordValue;

    cmd.DW2.DestinationX1CoordinateLeft = 0;
    cmd.DW2.DestinationY1CoordinateTop = 0;
    cmd.DW3.DestinationX2CoordinateRight = pFastCopyBltParam->dwDstRight;
    cmd.DW3.DestinationY2CoordinateBottom = pFastCopyBltParam->dwDstBottom;

    cmd.DW7.SourceX1CoordinateLeft = pFastCopyBltParam->dwSrcLeft;
    cmd.DW7.SourceY1CoordinateTop = pFastCopyBltParam->dwSrcTop;
    cmd.DW8.SourcePitch = pFastCopyBltParam->dwSrcPitch -1;

   if (pFastCopyBltParam->pDstOsResource->pGmmResInfo->GetResFlags().Info.NonLocalOnly)
    {
        cmd.DW6.DestinationTargetMemory =
          mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::DESTINATION_TARGET_MEMORY::DESTINATION_TARGET_MEMORY_SYSTEM_MEM;
    }
    if (pFastCopyBltParam->pSrcOsResource->pGmmResInfo->GetResFlags().Info.NonLocalOnly)
    {
        cmd.DW11.SourceTargetMemory =
          mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::SOURCE_TARGET_MEMORY::SOURCE_TARGET_MEMORY_SYSTEM_MEM;
    }

        cmd.DW16.DestinationSurfaceHeight     = dstResourceHeight -1;
        cmd.DW16.DestinationSurfaceWidth      = dstResourceWidth -1;
        cmd.DW16.DestinationSurfaceType       = 1; // 0 is 1D, 1 is 2D
        cmd.DW19.SourceSurfaceHeight          = sourceResourceHeight - 1;
        cmd.DW19.SourceSurfaceWidth           = sourceResourceWidth - 1;
        cmd.DW19.SourceSurfaceType            = mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::SOURCE_SURFACE_TYPE::SOURCE_SURFACE_TYPE_SURFTYPE_2D;;


        uint32_t srcQPitch = pSrcGmmResInfo->GetQPitch();
        uint32_t dstQPitch = pDstGmmResInfo->GetQPitch();
        GMM_RESOURCE_TYPE   dstResType = pDstGmmResInfo->GetResourceType();
        GMM_RESOURCE_TYPE   srcResType = pSrcGmmResInfo->GetResourceType();

        cmd.DW17.DestinationSurfaceQpitch                       = dstQPitch >> 2;
        cmd.DW20.SourceSurfaceQpitch                            = srcQPitch >> 2;

        cmd.DW18.DestinationHorizontalAlign                     = pDstGmmResInfo->GetVAlign();;
        cmd.DW18.DestinationVerticalAlign                       = pDstGmmResInfo->GetHAlign();
        cmd.DW18.DestinationMipTailStartLOD                     = 0xf;

        cmd.DW21.SourceHorizontalAlign                          = pSrcGmmResInfo->GetVAlign();
        cmd.DW21.SourceVerticalAlign                            = pSrcGmmResInfo->GetHAlign();
        cmd.DW21.SourceMipTailStartLOD                          = 0xf;

        // mmc
        MOS_MEMCOMP_STATE srcMmcModel = MOS_MEMCOMP_DISABLED;
        MOS_MEMCOMP_STATE dstMmcModel = MOS_MEMCOMP_DISABLED;
        uint32_t srcCompressionFormat = 0;
        uint32_t dstCompressionFormat = 0;
        GMM_RESOURCE_FLAG inputFlags  = pSrcGmmResInfo->GetResFlags();
        GMM_RESOURCE_FLAG outFlags    = pDstGmmResInfo->GetResFlags();
        MHW_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, pFastCopyBltParam->pSrcOsResource, (PMOS_MEMCOMP_STATE) & (srcMmcModel)));
        MHW_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, pFastCopyBltParam->pSrcOsResource, &srcCompressionFormat));
        MHW_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, pFastCopyBltParam->pDstOsResource, (PMOS_MEMCOMP_STATE) & (dstMmcModel)));
        MHW_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, pFastCopyBltParam->pDstOsResource, &dstCompressionFormat));

        if (dstMmcModel != MOS_MEMCOMP_DISABLED) // will enable RC later
        {
            cmd.DW1.DestinationCompressionEnable = 1;
            cmd.DW14.DestinationCompressionFormat = dstCompressionFormat;
        }

        if (srcMmcModel != MOS_MEMCOMP_DISABLED)//will enable RC later
        {
            cmd.DW8.SourceCompressionEnable    = 1;
            cmd.DW12.SourceCompressionFormat   = srcCompressionFormat;
            cmd.DW8.SourceAuxiliarysurfacemode =
                mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::SOURCE_AUXILIARY_SURFACE_MODE_AUX_CCS_E;
            if (srcMmcModel == MOS_MEMCOMP_MC)
            {   // 1 is MC; 0 is RC;
                cmd.DW8.SourceControlSurfaceType =
                    mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::SOURCE_CONTROL_SURFACE_TYPE_MEDIA_CONTROL_SURFACE; 
                if (pFastCopyBltParam->dwPlaneNum >= 2)
                {
                    // luma/chroma is represented by the MSB of the 5 bit format and used only for media decompression.
                    if (pFastCopyBltParam->dwPlaneIndex == 0)  // first plane
                    {
                        cmd.DW12.SourceCompressionFormat = srcCompressionFormat & 0x0F;
                    }
                    else  // second or third
                    {
                        cmd.DW12.SourceCompressionFormat = srcCompressionFormat | 0x10;
                    }
                }
            }
        }

    // add source address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 0;
    ResourceParams.dwOffset        = srcOffset;
    ResourceParams.presResource    = pFastCopyBltParam->pSrcOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW9_10.Value[0]);
    ResourceParams.dwLocationInCmd = 9;
    ResourceParams.bIsWritable     = false;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    // add destination address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 0;
    ResourceParams.dwOffset        = dstOffset;
    ResourceParams.presResource    = pFastCopyBltParam->pDstOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW4_5.Value[0]);
    ResourceParams.dwLocationInCmd = 4;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);
    MHW_NORMALMESSAGE("Block BLT cmd: width = %d, hieght = %d, ColorDepth = %d, Source Pitch %d, mocs = %d,tiled %d,"
            "mmc model % d, mmc format % d, dst Pitch %d, mocs = %d,tiled %d, mmc model %d, MMC Format = %d",
             pFastCopyBltParam->dwDstRight, pFastCopyBltParam->dwDstBottom,
            cmd.DW0.ColorDepth, cmd.DW8.SourcePitch, cmd.DW8.SourceMocs, cmd.DW8.SourceTiling, srcMmcModel, cmd.DW12.SourceCompressionFormat,
            cmd.DW1.DestinationPitch, cmd.DW1.DestinationMocSvalue, cmd.DW1.DestinationTiling, dstMmcModel, cmd.DW14.DestinationCompressionFormat);

finish:
    return eStatus;
}

uint32_t MhwBltInterfaceXe_Hp_Base::GetBlockTilingMode(BLT_TILE_TYPE TileType)
{
   switch(TileType)
   {
      case BLT_NOT_TILED:
          return mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::DESTINATION_TILING_LINEAR;
      case BLT_TILED_Y:
          return mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::DESTINATION_TILING_XMAJOR;
      case BLT_TILED_64:
          return mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::DESTINATION_TILING_TILE64;
      case BLT_TILED_4:
          return mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::DESTINATION_TILING_TILE4;
      default:
          MHW_ASSERTMESSAGE("BLT: Can't support GMM TileType %d.", TileType);
   }
   return mhw_blt_state_xe_hp_base::XY_BLOCK_COPY_BLT_CMD::DESTINATION_TILING_LINEAR;
}