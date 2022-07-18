/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_blt_copy_next.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#include "media_blt_copy_next.h"
#define BIT( n )                            ( 1 << (n) )

//!
//! \brief    BltStateNext constructor
//! \details  Initialize the BltStateNext members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateNext::BltStateNext(PMOS_INTERFACE    osInterface) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_cpInterface(nullptr)
{
    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_blt = 1;
    m_mhwInterfaces = MhwInterfacesNext::CreateFactory(params, osInterface);
    if (m_mhwInterfaces != nullptr)
    {
        m_miItf  = m_mhwInterfaces->m_miItf;
        m_bltItf = m_mhwInterfaces->m_bltItf;
    }
}

//!
//! \brief    BltStateNext constructor
//! \details  Initialize the BltStateNext members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateNext::BltStateNext(PMOS_INTERFACE    osInterface, MhwInterfacesNext* mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_cpInterface(nullptr)
{
    m_miItf        = mhwInterfaces->m_miItf;
    m_bltItf       = mhwInterfaces->m_bltItf;
    m_cpInterface  = mhwInterfaces->m_cpInterface;
}


BltStateNext::~BltStateNext()
{
    // component interface will be relesed in media copy.
    if (m_mhwInterfaces != nullptr)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }
}

//!
//! \brief    BltStateNext initialize
//! \details  Initialize the BltStateNext, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::Initialize()
{
    MOS_GPU_NODE            BltGpuNode;
    MOS_GPU_CONTEXT         BltGpuContext;
    MOS_GPUCTX_CREATOPTIONS createOption;

    BltGpuContext = MOS_GPU_CONTEXT_BLT;
    BltGpuNode    = MOS_GPU_NODE_BLT;

    BLT_CHK_NULL_RETURN(m_osInterface);
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
MOS_STATUS BltStateNext::CopyMainSurface(
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
MOS_STATUS BltStateNext::CopyMainSurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    BLT_STATE_PARAM BltStateNextParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_ZeroMemory(&BltStateNextParam, sizeof(BLT_STATE_PARAM));
    BltStateNextParam.bCopyMainSurface = true;
    BltStateNextParam.pSrcSurface      = src;
    BltStateNextParam.pDstSurface      = dst;

    BLT_CHK_STATUS_RETURN(SubmitCMD(&BltStateNextParam));

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
MOS_STATUS BltStateNext::SetupBltCopyParam(
    PMHW_FAST_COPY_BLT_PARAM pMhwBltParams,
    PMOS_RESOURCE            inputSurface,
    PMOS_RESOURCE            outputSurface,
    int                      planeIndex)
{
    BLT_CHK_NULL_RETURN(pMhwBltParams);
    BLT_CHK_NULL_RETURN(inputSurface);
    BLT_CHK_NULL_RETURN(outputSurface);

    uint32_t          BytesPerTexel = 1;
    MOS_SURFACE       ResDetails;
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(pMhwBltParams, sizeof(MHW_FAST_COPY_BLT_PARAM));
    ResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, inputSurface, &ResDetails));

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
    pMhwBltParams->dwDstRight = ResDetails.dwWidth;

    BLT_CHK_NULL_RETURN(outputSurface->pGmmResInfo);
    if (outputSurface->pGmmResInfo->GetResourceType() != RESOURCE_BUFFER)
    {
        BytesPerTexel = outputSurface->pGmmResInfo->GetBitsPerPixel() / 8;  // using Bytes.
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
       pMhwBltParams->dwDstBottom = (uint32_t)outputSurface->pGmmResInfo->GetSizeMainSurface() / ResDetails.dwPitch;
       if (false == m_blokCopyon)
       {// fastcopy
           pMhwBltParams->dwDstRight   = ResDetails.dwPitch / 4;  // Regard as 32 bit per pixel format, i.e. 4 byte per pixel.
           pMhwBltParams->dwColorDepth = 3;  //0:8bit 1:16bit 3:32bit 4:64bit
       }
    }
    else
    {
        BLT_CHK_NULL_RETURN(outputSurface->pGmmResInfo);
        int bytePerTexelScaling = GetBytesPerTexelScaling(ResDetails.Format);
        pMhwBltParams->dwDstBottom = ResDetails.dwHeight;

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
//! \param    pBltStateNextParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::SubmitCMD(
    PBLT_STATE_PARAM pBltStateNextParam)
{
    MOS_STATUS                   eStatus;
    MOS_COMMAND_BUFFER           cmdBuffer;
    MHW_FAST_COPY_BLT_PARAM      fastCopyBltParam;
    MOS_GPUCTX_CREATOPTIONS      createOption;
    int                          planeNum = 1;

    BLT_CHK_NULL_RETURN(m_miItf);
    BLT_CHK_NULL_RETURN(m_bltItf);

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
    auto& flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams = {};
    BLT_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    MOS_SURFACE       srcResDetails;
    MOS_SURFACE       dstResDetails;
    MOS_ZeroMemory(&srcResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&dstResDetails, sizeof(MOS_SURFACE));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateNextParam->pSrcSurface, &srcResDetails));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateNextParam->pDstSurface, &dstResDetails));

    if (srcResDetails.Format != dstResDetails.Format)
    {
        MCPY_ASSERTMESSAGE("BLT copy can't support CSC copy. input format = %d, output format = %d", srcResDetails.Format, dstResDetails.Format);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    planeNum = GetPlaneNum(dstResDetails.Format);

    if (pBltStateNextParam->bCopyMainSurface)
    {
        m_blokCopyon = true;
        BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
            &fastCopyBltParam,
            pBltStateNextParam->pSrcSurface,
            pBltStateNextParam->pDstSurface,
            0));

        BLT_CHK_STATUS_RETURN(m_bltItf->AddBlockCopyBlt(
            &cmdBuffer,
            &fastCopyBltParam,
            srcResDetails.YPlaneOffset.iSurfaceOffset,
            dstResDetails.YPlaneOffset.iSurfaceOffset));

        if (planeNum >= 2)
        {
            BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
             &fastCopyBltParam,
             pBltStateNextParam->pSrcSurface,
             pBltStateNextParam->pDstSurface,
             1));
            BLT_CHK_STATUS_RETURN(m_bltItf->AddBlockCopyBlt(
                 &cmdBuffer,
                 &fastCopyBltParam,
                 srcResDetails.UPlaneOffset.iSurfaceOffset,
                 dstResDetails.UPlaneOffset.iSurfaceOffset));

              if (planeNum == 3)
              {
                  BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
                      &fastCopyBltParam,
                      pBltStateNextParam->pSrcSurface,
                      pBltStateNextParam->pDstSurface,
                      2));
                  BLT_CHK_STATUS_RETURN(m_bltItf->AddBlockCopyBlt(
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
    flushDwParams = {};
    BLT_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
    // Add Batch Buffer end
    BLT_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    // Flush the command buffer
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false));

    return MOS_STATUS_SUCCESS;
}

uint32_t BltStateNext::GetBlkCopyColorDepth(
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

uint32_t BltStateNext::GetFastCopyColorDepth(
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

 int BltStateNext::GetBytesPerTexelScaling(MOS_FORMAT format)
{
  int  dstBytesPerTexel = 1;

   switch (format)
   {
       case Format_NV12:
            dstBytesPerTexel = 2;
           break;

       default:
           dstBytesPerTexel = 1;
    }
   return dstBytesPerTexel;
 }

int BltStateNext::GetPlaneNum(MOS_FORMAT format)
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