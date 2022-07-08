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
//! \file     media_blt_copy.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#define NOMINMAX
#include <algorithm>

#include "media_blt_copy.h"
#include "media_copy.h"
#include "mhw_mi.h"
#include "mos_utilities.h"
#define BIT( n )                            ( 1 << (n) )

//!
//! \brief    BltState constructor
//! \details  Initialize the BltState members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltState::BltState(PMOS_INTERFACE    osInterface) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_miInterface(nullptr),
    m_bltInterface(nullptr),
    m_cpInterface(nullptr)
{
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_blt = 1;
    m_mhwInterfaces = MhwInterfaces::CreateFactory(params, osInterface);
    if (m_mhwInterfaces != nullptr)
    {
        m_bltInterface = m_mhwInterfaces->m_bltInterface;
        m_miInterface  = m_mhwInterfaces->m_miInterface;
    }
}

//!
//! \brief    BltState constructor
//! \details  Initialize the BltState members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltState::BltState(PMOS_INTERFACE    osInterface, MhwInterfaces* mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_miInterface(nullptr),
    m_bltInterface(nullptr),
    m_cpInterface(nullptr)
{
    m_bltInterface = mhwInterfaces->m_bltInterface;
    m_miInterface  = mhwInterfaces->m_miInterface;
    m_cpInterface  = mhwInterfaces->m_cpInterface;
}


BltState::~BltState()
{
    // component interface will be relesed in media copy.
    if (m_mhwInterfaces)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }
}

//!
//! \brief    BltState initialize
//! \details  Initialize the BltState, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::Initialize()
{
    MOS_GPU_NODE            BltGpuNode;
    MOS_GPU_CONTEXT         BltGpuContext;
    MOS_GPUCTX_CREATOPTIONS createOption;

    BltGpuContext = MOS_GPU_CONTEXT_BLT;
    BltGpuNode    = MOS_GPU_NODE_BLT;

    BLT_CHK_NULL_RETURN(m_osInterface);
    BLT_CHK_NULL_RETURN(m_bltInterface);

    // Create BLT Context
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        BltGpuContext,
        BltGpuNode,
        &createOption));

    // Register context with the Batch Buffer completion event
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_BLT));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Copy main surface
//! \details  BLT engine will copy source surface to destination surface
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::CopyMainSurface(
    PMOS_SURFACE src,
    PMOS_SURFACE dst)
{
    BLT_CHK_STATUS_RETURN(CopyMainSurface(&src->OsResource, &dst->OsResource));
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Copy main surface
//! \details  BLT engine will copy source surface to destination surface
//! \param    src
//!           [in] Pointer to source resource
//! \param    dst
//!           [in] Pointer to destination resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::CopyMainSurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    BLT_STATE_PARAM bltStateParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_ZeroMemory(&bltStateParam, sizeof(BLT_STATE_PARAM));
    bltStateParam.bCopyMainSurface = true;
    bltStateParam.pSrcSurface      = src;
    bltStateParam.pDstSurface      = dst;

    BLT_CHK_STATUS_RETURN(SubmitCMD(&bltStateParam));

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return MOS_STATUS_SUCCESS;

}

//!
//! \brief    Setup fast copy parameters
//! \details  Setup fast copy parameters for BLT Engine
//! \param    mhwParams
//!           [in/out] Pointer to MHW_FAST_COPY_BLT_PARAM
//! \param    inputSurface
//!           [in] Pointer to input surface
//! \param    outputSurface
//!           [in] Pointer to output surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::SetupBltCopyParam(
    PMHW_FAST_COPY_BLT_PARAM pMhwBltParams,
    PMOS_RESOURCE            inputSurface,
    PMOS_RESOURCE            outputSurface,
    int                      planeIndex)
{
    BLT_CHK_NULL_RETURN(pMhwBltParams);
    BLT_CHK_NULL_RETURN(inputSurface);
    BLT_CHK_NULL_RETURN(outputSurface);
    BLT_CHK_NULL_RETURN(outputSurface->pGmmResInfo);

    uint32_t          BytesPerTexel = 1;
    MOS_SURFACE       ResDetails;
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(pMhwBltParams, sizeof(MHW_FAST_COPY_BLT_PARAM));
    ResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, inputSurface, &ResDetails));

    uint32_t inputHeight = ResDetails.dwHeight;
    uint32_t inputWidth  = ResDetails.dwWidth;
    uint32_t inputPitch  = ResDetails.dwPitch;

    if (inputSurface->TileType != MOS_TILE_LINEAR)
    { 
        pMhwBltParams->dwSrcPitch = ResDetails.dwPitch / 4;
    }
    else
    {
        pMhwBltParams->dwSrcPitch = ResDetails.dwPitch;
    }
    
    pMhwBltParams->dwSrcTop    = ResDetails.RenderOffset.YUV.Y.YOffset;
    pMhwBltParams->dwSrcLeft   = ResDetails.RenderOffset.YUV.Y.XOffset;

    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    ResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, outputSurface, &ResDetails));

    uint32_t outputHeight = ResDetails.dwHeight;
    uint32_t outputWidth  = ResDetails.dwWidth;
    uint32_t outputPitch  = ResDetails.dwPitch;

    if (outputSurface->TileType != MOS_TILE_LINEAR)
    {
        pMhwBltParams->dwDstPitch = ResDetails.dwPitch/4;
    }
    else
    {
        pMhwBltParams->dwDstPitch  = ResDetails.dwPitch;
    }
    pMhwBltParams->dwDstTop    = ResDetails.RenderOffset.YUV.Y.YOffset;
    pMhwBltParams->dwDstLeft   = ResDetails.RenderOffset.YUV.Y.XOffset;

    int planeNum = GetPlaneNum(ResDetails.Format);
    pMhwBltParams->dwDstRight = std::min(inputWidth, outputWidth);

    if (outputSurface->pGmmResInfo->GetResourceType() != RESOURCE_BUFFER)
    {
        BytesPerTexel = outputSurface->pGmmResInfo->GetBitsPerPixel() / 8;  // using Bytes.
        if (ResDetails.Format == Format_P010 || ResDetails.Format == Format_P016)
        {
            BytesPerTexel = 2;
        }
    }

    if (true == m_blokCopyon)
    {
        pMhwBltParams->dwColorDepth = GetBlkCopyColorDepth(outputSurface->pGmmResInfo->GetResourceFormat(), BytesPerTexel);
    }
    else
    {
        pMhwBltParams->dwColorDepth = GetFastCopyColorDepth(outputSurface->pGmmResInfo->GetResourceFormat(), BytesPerTexel);
    }

    if( 1 == planeNum )
    {// handle as whole memory
       pMhwBltParams->dwDstBottom = std::min(inputHeight, outputHeight);
       if (false == m_blokCopyon)
       {// fastcopy
           pMhwBltParams->dwDstRight   = std::min(inputPitch, outputPitch) / 4;  // Regard as 32 bit per pixel format, i.e. 4 byte per pixel.
           pMhwBltParams->dwColorDepth = 3;  //0:8bit 1:16bit 3:32bit 4:64bit
       }
    }
    else
    {
        int bytePerTexelScaling    = GetBytesPerTexelScaling(ResDetails.Format);
        pMhwBltParams->dwDstBottom = std::min(inputHeight, outputHeight);

        if (1 == planeIndex || 2 == planeIndex)
        {
           pMhwBltParams->dwDstBottom = pMhwBltParams->dwDstBottom / bytePerTexelScaling;
           if (ResDetails.Format == Format_I420 || ResDetails.Format == Format_YV12)
           {
               pMhwBltParams->dwDstPitch  = pMhwBltParams->dwDstPitch / 2;
               pMhwBltParams->dwSrcPitch  = pMhwBltParams->dwSrcPitch / 2;
               pMhwBltParams->dwDstRight  = pMhwBltParams->dwDstRight / 2;
               pMhwBltParams->dwDstBottom = pMhwBltParams->dwDstBottom / 2;
           }
        }
    }
    pMhwBltParams->pSrcOsResource = inputSurface;
    pMhwBltParams->pDstOsResource = outputSurface;
    MCPY_NORMALMESSAGE("BLT params: format %d, planeNum %d, planeIndex %d, dwColorDepth %d, dwSrcTop %d,dwSrcLeft %d dwSrcPitch %d"
                       "dwDstTop %d, dwDstLeft %d, dwDstRight %d , dwDstBottom %d, dwDstPitch %d",
                       ResDetails.Format, planeNum, planeIndex, pMhwBltParams->dwColorDepth, pMhwBltParams->dwSrcTop, pMhwBltParams->dwSrcLeft,
                       pMhwBltParams->dwSrcPitch, pMhwBltParams->dwDstTop, pMhwBltParams->dwDstLeft, pMhwBltParams->dwDstRight, 
                       pMhwBltParams->dwDstBottom, pMhwBltParams->dwDstPitch);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Submit command2
//! \details  Submit BLT command2
//! \param    pBltStateParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState::SubmitCMD(
    PBLT_STATE_PARAM pBltStateParam)
{
    MOS_STATUS                   eStatus;
    MOS_COMMAND_BUFFER           cmdBuffer;
    MHW_FAST_COPY_BLT_PARAM      fastCopyBltParam;
    MOS_GPUCTX_CREATOPTIONS      createOption;
    int                          planeNum = 1;

    // no gpucontext will be created if the gpu context has been created before.
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_BLT,
        MOS_GPU_NODE_BLT,
        &createOption));
    // Set GPU context
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_BLT));

    // Initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Add flush DW
    MHW_MI_FLUSH_DW_PARAMS FlushDwParams;
    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &FlushDwParams));

    MOS_SURFACE       srcResDetails;
    MOS_SURFACE       dstResDetails;
    MOS_ZeroMemory(&srcResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&dstResDetails, sizeof(MOS_SURFACE));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateParam->pSrcSurface, &srcResDetails));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateParam->pDstSurface, &dstResDetails));

    if (srcResDetails.Format != dstResDetails.Format)
    {
        MCPY_ASSERTMESSAGE("BLT copy can't support CSC copy. input format = %d, output format = %d", srcResDetails.Format, dstResDetails.Format);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    planeNum = GetPlaneNum(dstResDetails.Format);

    if (pBltStateParam->bCopyMainSurface)
    {
        m_blokCopyon = true;
        BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
            &fastCopyBltParam,
            pBltStateParam->pSrcSurface,
            pBltStateParam->pDstSurface,
            0));

        MHW_MI_LOAD_REGISTER_IMM_PARAMS RegisterDwParams;
        MOS_ZeroMemory(&RegisterDwParams, sizeof(RegisterDwParams));
        RegisterDwParams.dwRegister = mhw_blt_state::BCS_SWCTRL_CMD::REGISTER_OFFSET;

        mhw_blt_state::BCS_SWCTRL_CMD swctrl;
        if (pBltStateParam->pSrcSurface->TileType != MOS_TILE_LINEAR)
        {
           swctrl.DW0.TileYSource = 1;
           swctrl.DW0.Mask |= BIT(0);
        }
        if (pBltStateParam->pDstSurface->TileType != MOS_TILE_LINEAR)
        {//output tiled
           swctrl.DW0.TileYDestination = 1;
           swctrl.DW0.Mask |= BIT(1);
        }

        RegisterDwParams.dwData = swctrl.DW0.Value;
        m_miInterface->AddMiLoadRegisterImmCmd(&cmdBuffer, &RegisterDwParams);

        BLT_CHK_STATUS_RETURN(m_bltInterface->AddBlockCopyBlt(
            &cmdBuffer,
            &fastCopyBltParam,
            srcResDetails.YPlaneOffset.iSurfaceOffset,
            dstResDetails.YPlaneOffset.iSurfaceOffset));

        if (planeNum >= 2)
        {
            BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
             &fastCopyBltParam,
             pBltStateParam->pSrcSurface,
             pBltStateParam->pDstSurface,
             1));
            BLT_CHK_STATUS_RETURN(m_bltInterface->AddBlockCopyBlt(
                 &cmdBuffer,
                 &fastCopyBltParam,
                 srcResDetails.UPlaneOffset.iSurfaceOffset,
                 dstResDetails.UPlaneOffset.iSurfaceOffset));

              if (planeNum == 3)
              {
                  BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
                      &fastCopyBltParam,
                      pBltStateParam->pSrcSurface,
                      pBltStateParam->pDstSurface,
                      2));
                  BLT_CHK_STATUS_RETURN(m_bltInterface->AddBlockCopyBlt(
                      &cmdBuffer,
                      &fastCopyBltParam,
                      srcResDetails.VPlaneOffset.iSurfaceOffset,
                      dstResDetails.VPlaneOffset.iSurfaceOffset));
              }
              else if (planeNum > 3)
              {
                  MCPY_ASSERTMESSAGE("illegal usage");
                  return MOS_STATUS_INVALID_PARAMETER;
              }
         }
    }
    // Add flush DW
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &FlushDwParams));

    // Add Batch Buffer end
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // Flush the command buffer
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false));

    return MOS_STATUS_SUCCESS;
}

uint32_t BltState::GetBlkCopyColorDepth(
    GMM_RESOURCE_FORMAT dstFormat,
    uint32_t            BytesPerTexel)
{
    uint32_t bitsPerTexel = BytesPerTexel * BLT_BITS_PER_BYTE;

    switch (bitsPerTexel)
    {
     case 8:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_8BITCOLOR;
         break;
     case 16:
         switch (dstFormat)
         {
           case GMM_FORMAT_B5G5R5A1_UNORM:
               return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
               break;
           default:
               return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_16BITCOLOR;;
               break;
         }
         break;
     case 64:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_64BITCOLOR;
         break;
     case 96:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
         break;
     case 128:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_128BITCOLOR;
         break;
     default:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
         break;
    }

 }

uint32_t BltState::GetFastCopyColorDepth(
     GMM_RESOURCE_FORMAT dstFormat,
     uint32_t            BytesPerTexel)
 {
     uint32_t bitsPerTexel = BytesPerTexel * BLT_BITS_PER_BYTE;

     switch (bitsPerTexel)
     {
     case 8:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_8BITCOLOR;
         break;
     case 16:
         switch (dstFormat)
         {
         case GMM_FORMAT_B5G5R5A1_UNORM:
             return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
             break;
         default:
             return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_16BITCOLOR_565;
             break;
         }
         break;
     case 64:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_64BITCOLOR_FOR64KBTILING;
         break;
     case 128:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_128BITCOLOR_FOR64KBTILING;
         break;
     default:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
         break;
     }
 }

 int BltState::GetBytesPerTexelScaling(MOS_FORMAT format)
{
  int  dstBytesPerTexel = 1;

   switch (format)
   {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            dstBytesPerTexel = 2;
           break;

       default:
           dstBytesPerTexel = 1;
    }
   return dstBytesPerTexel;
 }

int BltState::GetPlaneNum(MOS_FORMAT format)
{

  int  planeNum = 1;

   switch (format)
   {
       case Format_NV12:
       case Format_P010:
       case Format_P016:
            planeNum = 2;
           break;
       case Format_YV12:
       case Format_I420:
       case Format_444P:
       case Format_RGBP:
       case Format_BGRP:
       case Format_IMC3:
       case Format_411P:
       case Format_422V:
       case Format_422H:
            planeNum = 3;
            break;
       default:
            planeNum = 1;
           break;
    }
   return planeNum;
 }