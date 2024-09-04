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
//! \file     mhw_blt_impl.h
//! \brief    MHW BLT interface common base
//! \details
//!

#ifndef __MHW_BLT_IMPL_H__
#define __MHW_BLT_IMPL_H__

#include "mhw_blt_itf.h"
#include "mhw_impl.h"
#include "media_copy_common.h"

namespace mhw
{
namespace blt
{

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _BLT_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Add Block copy Command
    //! \details  BLT function to add block copy command
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] pFastCopyBltParam
    //!           Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \param    [in] srcOffset
    //!           input surface's soruce offset
    //! \param    [in] outOffset
    //!           output surface's soruce offset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddBlockCopyBlt(
            PMOS_COMMAND_BUFFER      pCmdBuffer,
            PMHW_FAST_COPY_BLT_PARAM pBlockCopyBltParam, 
            uint32_t                 srcOffset,
            uint32_t                 dstOffset) override
    {
        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_NULL_RETURN(pBlockCopyBltParam);

        auto& par = MHW_GETPAR_F(XY_BLOCK_COPY_BLT)();
        par = {};
        par.dwColorDepth    = pBlockCopyBltParam->dwColorDepth;
        par.dwSrcPitch      = pBlockCopyBltParam->dwSrcPitch;
        par.dwDstPitch      = pBlockCopyBltParam->dwDstPitch;
        par.dwSrcTop        = pBlockCopyBltParam->dwSrcTop;
        par.dwSrcLeft       = pBlockCopyBltParam->dwSrcLeft;
        par.dwDstTop        = pBlockCopyBltParam->dwDstTop;
        par.dwDstBottom     = pBlockCopyBltParam->dwDstBottom;
        par.dwDstLeft       = pBlockCopyBltParam->dwDstLeft;
        par.dwDstRight      = pBlockCopyBltParam->dwDstRight;
        par.dwSrcOffset     = srcOffset;
        par.dwDstOffset     = dstOffset;
        par.dwPlaneIndex    = pBlockCopyBltParam->dwPlaneIndex;
        par.dwPlaneNum      = pBlockCopyBltParam->dwPlaneNum;
        par.pSrcOsResource  = pBlockCopyBltParam->pSrcOsResource;
        par.pDstOsResource  = pBlockCopyBltParam->pDstOsResource;

        MHW_CHK_STATUS_RETURN(MHW_ADDCMD_F(XY_BLOCK_COPY_BLT)(pCmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(XY_BLOCK_COPY_BLT)
    {
        MHW_FUNCTION_ENTER;

        _MHW_SETCMD_CALLBASE(XY_BLOCK_COPY_BLT);
        MHW_CHK_NULL_RETURN(params.pSrcOsResource);
        MHW_CHK_NULL_RETURN(params.pDstOsResource);

        MHW_RESOURCE_PARAMS            ResourceParams;
        PGMM_RESOURCE_INFO             pSrcGmmResInfo = params.pSrcOsResource->pGmmResInfo;
        PGMM_RESOURCE_INFO             pDstGmmResInfo = params.pDstOsResource->pGmmResInfo;
        MHW_CHK_NULL_RETURN(pSrcGmmResInfo);
        MHW_CHK_NULL_RETURN(pDstGmmResInfo);

        BLT_TILE_TYPE dstTiledMode = static_cast<BLT_TILE_TYPE>(pDstGmmResInfo->GetTileType());
        BLT_TILE_TYPE srcTiledMode = static_cast<BLT_TILE_TYPE>(pSrcGmmResInfo->GetTileType());

        uint32_t sourceResourceWidth  = (uint32_t)pSrcGmmResInfo->GetBaseWidth();
        uint32_t sourceResourceHeight = (uint32_t)pSrcGmmResInfo->GetBaseHeight();
        uint32_t dstResourceWidth     = (uint32_t)pDstGmmResInfo->GetBaseWidth();
        uint32_t dstResourceHeight    = (uint32_t)pDstGmmResInfo->GetBaseHeight();

        MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        // mmc
        MOS_MEMCOMP_STATE srcMmcModel          = MOS_MEMCOMP_DISABLED;
        MOS_MEMCOMP_STATE dstMmcModel          = MOS_MEMCOMP_DISABLED;
        GMM_RESOURCE_FLAG inputFlags           = pSrcGmmResInfo->GetResFlags();
        GMM_RESOURCE_FLAG outFlags             = pDstGmmResInfo->GetResFlags();
        MCPY_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionMode(this->m_osItf, params.pSrcOsResource, (PMOS_MEMCOMP_STATE) & (srcMmcModel)));
        MCPY_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionMode(this->m_osItf, params.pDstOsResource, (PMOS_MEMCOMP_STATE) & (dstMmcModel)));
        
        uint32_t srcQPitch = pSrcGmmResInfo->GetQPitch();
        uint32_t dstQPitch = pDstGmmResInfo->GetQPitch();
        GMM_RESOURCE_TYPE   dstResType = pDstGmmResInfo->GetResourceType();
        GMM_RESOURCE_TYPE   srcResType = pSrcGmmResInfo->GetResourceType();

        uint32_t dstSampleNum        = pDstGmmResInfo->GetNumSamples();

        cmd.DW0.InstructionTargetOpcode = 0x41;
        cmd.DW0.ColorDepth              = params.dwColorDepth;
        cmd.DW1.DestinationPitch        = params.dwDstPitch - 1;
        cmd.DW1.DestinationMocsValue    = GetBlockCopyBltMOCS(MOS_GMM_RESOURCE_USAGE_BLT_DESTINATION);

        cmd.DW1.DestinationTiling             = GetFastTilingMode(dstTiledMode);
        cmd.DW8.SourceTiling                  = GetFastTilingMode(srcTiledMode);
        cmd.DW8.SourceMocs                    = GetBlockCopyBltMOCS(MOS_GMM_RESOURCE_USAGE_BLT_SOURCE);

        cmd.DW2.DestinationX1CoordinateLeft   = 0;
        cmd.DW2.DestinationY1CoordinateTop    = 0;
        cmd.DW3.DestinationX2CoordinateRight  = params.dwDstRight;
        cmd.DW3.DestinationY2CoordinateBottom = params.dwDstBottom;
        cmd.DW7.SourceX1CoordinateLeft        = params.dwSrcLeft;
        cmd.DW7.SourceY1CoordinateTop         = params.dwSrcTop;
        cmd.DW8.SourcePitch                   = params.dwSrcPitch - 1;

        if (!outFlags.Info.LocalOnly)
        {
            cmd.DW6.DestinationTargetMemory = 1;//DESTINATION_TARGET_MEMORY::DESTINATION_TARGET_MEMORY_SYSTEM_MEM;
        }
        if (!inputFlags.Info.LocalOnly)
        {
            cmd.DW11.SourceTargetMemory = 1;// SOURCE_TARGET_MEMORY::SOURCE_TARGET_MEMORY_SYSTEM_MEM;
        }

        cmd.DW16.DestinationSurfaceHeight     = dstResourceHeight -1;
        cmd.DW16.DestinationSurfaceWidth      = dstResourceWidth -1;
        cmd.DW16.DestinationSurfaceType       = 1; // 0 is 1D, 1 is 2D
        cmd.DW19.SourceSurfaceHeight          = sourceResourceHeight - 1;
        cmd.DW19.SourceSurfaceWidth           = sourceResourceWidth - 1;
        cmd.DW19.SourceSurfaceType            = 1;

        MCPY_NORMALMESSAGE("Src type %d, dst type %d, srcTiledMode %d,  dstTiledMode %d", 
            srcResType, dstResType, srcTiledMode, dstTiledMode);

        cmd.DW17.DestinationSurfaceQpitch                       = dstQPitch >> 2;
        cmd.DW20.SourceSurfaceQpitch                            = srcQPitch >> 2;

        cmd.DW18.DestinationHorizontalAlign                     = pDstGmmResInfo->GetVAlign();;
        cmd.DW18.DestinationVerticalAlign                       = pDstGmmResInfo->GetHAlign();
        cmd.DW18.DestinationMipTailStartLOD                     = 0xf;

        cmd.DW21.SourceHorizontalAlign                          = pSrcGmmResInfo->GetVAlign();
        cmd.DW21.SourceVerticalAlign                            = pSrcGmmResInfo->GetHAlign();
        cmd.DW21.SourceMipTailStartLOD                          = 0xf;

        // add source address
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.dwLsbNum        = 0;
        ResourceParams.dwOffset        = params.dwSrcOffset;
        ResourceParams.presResource    = params.pSrcOsResource;
        ResourceParams.pdwCmd          = &(cmd.DW9_10.Value[0]);
        ResourceParams.dwLocationInCmd = 9;
        ResourceParams.bIsWritable     = false;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &ResourceParams));

        // add destination address
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.dwLsbNum        = 0;
        ResourceParams.dwOffset        = params.dwDstOffset;
        ResourceParams.presResource    = params.pDstOsResource;
        ResourceParams.pdwCmd          = &(cmd.DW4_5.Value[0]);
        ResourceParams.dwLocationInCmd = 4;
        ResourceParams.bIsWritable     = true;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &ResourceParams));

        MCPY_NORMALMESSAGE("Block BLT cmd:dstSampleNum = %d;  width = %d, hieght = %d, ColorDepth = %d, Source Pitch %d, mocs = %d,tiled %d," 
            "mmc model % d, dst Pitch %d, mocs = %d,tiled %d, mmc model %d",
            dstSampleNum, params.dwDstRight, params.dwDstBottom,
            cmd.DW0.ColorDepth, cmd.DW8.SourcePitch, cmd.DW8.SourceMocs, cmd.DW8.SourceTiling, srcMmcModel,
            cmd.DW1.DestinationPitch, cmd.DW1.DestinationMocsValue, cmd.DW1.DestinationTiling, dstMmcModel);

        return MOS_STATUS_SUCCESS;
    }

protected:
    //!
    //! \brief    Get Block copy MOCS
    //! \details  BLT function to get the MOCS
    //! \param    [in] MOS_HW_RESOURCE_DEF
    //!           Pointer to UsageDef
    //! \return   uint32_t
    //!           return the MOCS value
    //!
    virtual uint32_t GetBlockCopyBltMOCS(MOS_HW_RESOURCE_DEF UsageDef)
    {
        // MemoryObject will get 7 bits data. only bits[1-7] save MOCS, while some
        // platoforms only use 4 bits mocs.
        return (m_osItf->pfnCachePolicyGetMemoryObject(UsageDef,
                    m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue & 0x1E) >>1;
    }

    using base_t = Itf;
MEDIA_CLASS_DEFINE_END(mhw__blt__Impl)
};
}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_IMPL_H__
