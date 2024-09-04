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
//! \file     mhw_blt_xe_lpm_plus_base_next_impl.h
//! \brief    MHW blt interface common base for Xe_LPM_PLUS
//! \details
//!

#ifndef __MHW_BLT_XE_LPM_PLUS_BASE_NEXT_IMPL_H__
#define __MHW_BLT_XE_LPM_PLUS_BASE_NEXT_IMPL_H__

#include "mhw_blt_impl.h"
#include "mhw_blt_hwcmd_xe_lpm_plus_next.h"
#include "mhw_blt_itf.h"
#include "mhw_impl.h"
#include "mos_solo_generic.h"

namespace mhw
{
namespace blt
{
namespace xe_lpm_plus_next
{
class Impl : public blt::Impl<mhw::blt::xe_lpm_plus_next::Cmd>
{
public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;
    }

    uint32_t GetFastTilingMode(BLT_TILE_TYPE TileType) override
    {
        MHW_FUNCTION_ENTER;
        switch (TileType)
        {
        case BLT_NOT_TILED:
            return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
        case BLT_TILED_Y:
        case BLT_TILED_4:
            return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_TILE_Y;
        case BLT_TILED_64:
            return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_64KBTILING;
        default:
            MHW_ASSERTMESSAGE("BLT: Can't support GMM TileType %d.", TileType);
        }
        return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
    }

    _MHW_SETCMD_OVERRIDE_DECL(XY_BLOCK_COPY_BLT)
    {
        MHW_FUNCTION_ENTER;

        _MHW_SETCMD_CALLBASE(XY_BLOCK_COPY_BLT);
        MHW_CHK_NULL_RETURN(params.pSrcOsResource);
        MHW_CHK_NULL_RETURN(params.pDstOsResource);

        MHW_RESOURCE_PARAMS ResourceParams;
        PGMM_RESOURCE_INFO  pSrcGmmResInfo = params.pSrcOsResource->pGmmResInfo;
        PGMM_RESOURCE_INFO  pDstGmmResInfo = params.pDstOsResource->pGmmResInfo;
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
        uint32_t          srcCompressionFormat = 0;
        uint32_t          dstCompressionFormat = 0;
        GMM_RESOURCE_FLAG inputFlags           = pSrcGmmResInfo->GetResFlags();
        GMM_RESOURCE_FLAG outFlags             = pDstGmmResInfo->GetResFlags();
        MCPY_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionMode(this->m_osItf, params.pSrcOsResource, (PMOS_MEMCOMP_STATE) & (srcMmcModel)));
        MCPY_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionFormat(this->m_osItf, params.pSrcOsResource, &srcCompressionFormat));
        MCPY_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionMode(this->m_osItf, params.pDstOsResource, (PMOS_MEMCOMP_STATE) & (dstMmcModel)));
        MCPY_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionFormat(this->m_osItf, params.pDstOsResource, &dstCompressionFormat));

        uint32_t          srcQPitch  = pSrcGmmResInfo->GetQPitch();
        uint32_t          dstQPitch  = pDstGmmResInfo->GetQPitch();
        GMM_RESOURCE_TYPE dstResType = pDstGmmResInfo->GetResourceType();
        GMM_RESOURCE_TYPE srcResType = pSrcGmmResInfo->GetResourceType();

        uint32_t dstSampleNum = pDstGmmResInfo->GetNumSamples();

        cmd.DW0.InstructionTargetOpcode = 0x41;
        cmd.DW0.ColorDepth              = params.dwColorDepth;
        cmd.DW1.DestinationPitch        = params.dwDstPitch - 1;
        cmd.DW1.DestinationMocsValue =
            this->m_osItf->pfnCachePolicyGetMemoryObject(MOS_GMM_RESOURCE_USAGE_BLT_DESTINATION,
                             m_osItf->pfnGetGmmClientContext(m_osItf))
                .DwordValue;

        if (dstMmcModel != MOS_MEMCOMP_DISABLED && dstMmcModel != MOS_MEMCOMP_RC)
        {
            cmd.DW1.DestinationCompressionEnable = 1;
            // Destination control surface type cannot be media if compression is enabled
            cmd.DW1.DestinationControlSurfaceType = 0;  // 1 is media; 0 is 3D;
            cmd.DW14.DestinationCompressionFormat = dstCompressionFormat;
        }
        if (srcMmcModel != MOS_MEMCOMP_DISABLED)
        {
            cmd.DW8.SourceCompressionEnable    = 1;
            cmd.DW12.SourceCompressionFormat   = srcCompressionFormat;
            cmd.DW8.SourceAuxiliarysurfacemode =
                Cmd::XY_BLOCK_COPY_BLT_CMD::SOURCE_AUXILIARY_SURFACE_MODE_AUX_CCS_E;
            if (srcMmcModel == MOS_MEMCOMP_MC)
            {
                cmd.DW8.SourceControlSurfaceType =
                    Cmd::XY_BLOCK_COPY_BLT_CMD::SOURCE_CONTROL_SURFACE_TYPE_MEDIA_CONTROL_SURFACE;
                if (params.dwPlaneNum >=2)
                {
                    // luma/chroma is represented by the MSB of the 5 bit format and used only for media decompression.
                    if (params.dwPlaneIndex == 0)  // first plane
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
        cmd.DW1.DestinationTiling = GetFastTilingMode(dstTiledMode);
        cmd.DW8.SourceTiling      = GetFastTilingMode(srcTiledMode);
        cmd.DW8.SourceMocs =
            this->m_osItf->pfnCachePolicyGetMemoryObject(MOS_GMM_RESOURCE_USAGE_BLT_SOURCE,
                             m_osItf->pfnGetGmmClientContext(m_osItf))
                .DwordValue;

        cmd.DW2.DestinationX1CoordinateLeft   = 0;
        cmd.DW2.DestinationY1CoordinateTop    = 0;
        cmd.DW3.DestinationX2CoordinateRight  = params.dwDstRight;
        cmd.DW3.DestinationY2CoordinateBottom = params.dwDstBottom;
        cmd.DW7.SourceX1CoordinateLeft        = params.dwSrcLeft;
        cmd.DW7.SourceY1CoordinateTop         = params.dwSrcTop;
        cmd.DW8.SourcePitch                   = params.dwSrcPitch - 1;

        if (!outFlags.Info.LocalOnly)
        {
            cmd.DW6.DestinationTargetMemory = 1;  //DESTINATION_TARGET_MEMORY::DESTINATION_TARGET_MEMORY_SYSTEM_MEM;
        }
        if (!inputFlags.Info.LocalOnly)
        {
            cmd.DW11.SourceTargetMemory = 1;  // SOURCE_TARGET_MEMORY::SOURCE_TARGET_MEMORY_SYSTEM_MEM;
        }

        cmd.DW16.DestinationSurfaceHeight = dstResourceHeight - 1;
        cmd.DW16.DestinationSurfaceWidth  = dstResourceWidth - 1;
        cmd.DW16.DestinationSurfaceType   = 1;  // 0 is 1D, 1 is 2D
        cmd.DW19.SourceSurfaceHeight      = sourceResourceHeight - 1;
        cmd.DW19.SourceSurfaceWidth       = sourceResourceWidth - 1;
        cmd.DW19.SourceSurfaceType        = 1;

        MCPY_NORMALMESSAGE("Src type %d, dst type %d, srcTiledMode %d,  dstTiledMode %d",
            srcResType,
            dstResType,
            srcTiledMode,
            dstTiledMode);

        cmd.DW17.DestinationSurfaceQpitch = dstQPitch >> 2;
        cmd.DW20.SourceSurfaceQpitch      = srcQPitch >> 2;

        cmd.DW18.DestinationHorizontalAlign = pDstGmmResInfo->GetVAlign();
        ;
        cmd.DW18.DestinationVerticalAlign   = pDstGmmResInfo->GetHAlign();
        cmd.DW18.DestinationMipTailStartLOD = 0xf;

        cmd.DW21.SourceHorizontalAlign = pSrcGmmResInfo->GetVAlign();
        cmd.DW21.SourceVerticalAlign   = pSrcGmmResInfo->GetHAlign();
        cmd.DW21.SourceMipTailStartLOD = 0xf;

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

        MCPY_NORMALMESSAGE(
            "Block BLT cmd:dstSampleNum = %d;  width = %d, hieght = %d, ColorDepth = %d, Source Pitch %d, mocs = %d,tiled %d,"
            "mmc model % d, mmc format % d, dst Pitch %d, mocs = %d,tiled %d, mmc model %d, MMC Format = %d",
            dstSampleNum,
            params.dwDstRight,
            params.dwDstBottom,
            cmd.DW0.ColorDepth,
            cmd.DW8.SourcePitch,
            cmd.DW8.SourceMocs,
            cmd.DW8.SourceTiling,
            srcMmcModel,
            cmd.DW12.SourceCompressionFormat,
            cmd.DW1.DestinationPitch,
            cmd.DW1.DestinationMocsValue,
            cmd.DW1.DestinationTiling,
            dstMmcModel,
            cmd.DW14.DestinationCompressionFormat);

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
    uint32_t GetBlockCopyBltMOCS(MOS_HW_RESOURCE_DEF UsageDef)
    {
        // MemoryObject will get 7 bits data. bit[0] for encrypt and bits[1-7] for MOCS.
        return m_osItf->pfnCachePolicyGetMemoryObject(UsageDef,
                    m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
    }

    using base_t = blt::Impl<mhw::blt::xe_lpm_plus_next::Cmd>;
MEDIA_CLASS_DEFINE_END(mhw__blt__xe_lpm_plus_next__Impl)
};  // Impl
}  // xe_lpm_plus_next
}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_IMPL_H__
