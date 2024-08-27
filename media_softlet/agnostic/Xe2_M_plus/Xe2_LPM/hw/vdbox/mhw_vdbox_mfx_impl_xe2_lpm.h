/*
# Copyright (c) 2024, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     mhw_vdbox_mfx_impl_xe2_lpm.h
//! \brief    MHW VDBOX MFX interface common base for Xe2_LPM
//! \details
//!

#ifndef __MHW_VDBOX_MFX_IMPL_XE2_LPM_H__
#define __MHW_VDBOX_MFX_IMPL_XE2_LPM_H__

#include "mhw_vdbox_mfx_impl_xe2_lpm_base.h"
#include "mhw_vdbox_mfx_hwcmd_xe2_lpm.h"

namespace mhw
{
namespace vdbox
{
namespace mfx
{
namespace xe2_lpm_base
{
namespace xe2_lpm
{
class Impl : public BaseImpl<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};

protected:
    _MHW_SETCMD_OVERRIDE_DECL(MFX_AVC_IMG_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_AVC_IMG_STATE);

        #define DO_FIELDS() \
        DO_FIELD(DW3, Reserved117, params.vdaqmEnable)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(MFX_PIPE_MODE_SELECT);

        #define DO_FIELDS() \
        DO_FIELD(DW4, SliceSizeStreamout32bit, params.sliceSizeStreamout32bit);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        if (params.psPreDeblockSurface != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW3.Value, 1, 6);
            cmd.DW3.PreDeblockingMemoryObjectControlState = m_preDeblockingMemoryCtrl.Gen12_7.Index;

            resourceParams.presResource            = &(params.psPreDeblockSurface->OsResource);
            resourceParams.dwOffset                = params.psPreDeblockSurface->dwOffset;
            resourceParams.pdwCmd                  = &(cmd.DW1.Value);
            resourceParams.dwLocationInCmd         = 1;
            resourceParams.bIsWritable             = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.psPostDeblockSurface != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW6.Value, 1, 6);
            cmd.DW6.PostDeblockingMemoryObjectControlState = m_postDeblockingMemoryCtrl.Gen12_7.Index;

            resourceParams.presResource    = &(params.psPostDeblockSurface->OsResource);
            resourceParams.dwOffset        = params.psPostDeblockSurface->dwOffset;
            resourceParams.pdwCmd          = &(cmd.DW4.Value);
            resourceParams.dwLocationInCmd = 4;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.psRawSurface != nullptr)
        {
            if (!params.decodeInUse)
            {
                cmd.DW9.OriginalUncompressedPictureSourceMemoryObjectControlState =
                    m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;
            }
            else
            {
                InitMocsParams(resourceParams, &cmd.DW9.Value, 1, 6);
                cmd.DW9.OriginalUncompressedPictureSourceMemoryObjectControlState = m_OriginalUncompressedPictureSourceMemoryCtrl.Gen12_7.Index;
            }

            resourceParams.presResource    = &params.psRawSurface->OsResource;
            resourceParams.dwOffset        = params.psRawSurface->dwOffset;
            resourceParams.pdwCmd          = &(cmd.DW7.Value);
            resourceParams.dwLocationInCmd = 7;
            resourceParams.bIsWritable     = false;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.presStreamOutBuffer != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW12.Value, 1, 6);
            cmd.DW12.StreamoutDataDestinationMemoryObjectControlState = m_streamoutDataDestinationMemoryCtrl.Gen12_7.Index;

            resourceParams.presResource    = params.presStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW10.Value);
            resourceParams.dwLocationInCmd = 10;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            if (!params.decodeInUse)
            {
                cmd.DW54.MacroblockStatusBufferMemoryObjectControlState =
                    m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

                resourceParams.presResource    = params.presStreamOutBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = &(cmd.DW52.Value);
                resourceParams.dwLocationInCmd = 52;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        if (m_intraRowstoreCache.enabled)
        {
            cmd.DW15.IntraRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.DW13.IntraRowStoreScratchBufferBaseAddress = m_intraRowstoreCache.dwAddress;
        }
        else if (params.presMfdIntraRowStoreScratchBuffer != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW15.Value, 1, 6);
            cmd.DW15.IntraRowStoreScratchBufferMemoryObjectControlState = m_intraRowStoreScratchBufferMemoryCtrl.Gen12_7.Index;

            resourceParams.presResource    = params.presMfdIntraRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW13.Value);
            resourceParams.dwLocationInCmd = 13;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (m_deblockingFilterRowstoreCache.enabled)
        {
            cmd.DW18.DeblockingFilterRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.DW16.DeblockingFilterRowStoreScratchBaseAddress = m_deblockingFilterRowstoreCache.dwAddress;
        }
        else if (params.presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW18.Value, 1, 6);
            cmd.DW18.DeblockingFilterRowStoreScratchMemoryObjectControlState = m_deblockingFilterRowStoreScratchMemoryCtrl.Gen12_7.Index;

            resourceParams.presResource    = params.presMfdDeblockingFilterRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW16.Value);
            resourceParams.dwLocationInCmd = 16;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        PMOS_RESOURCE *references = const_cast<PMOS_RESOURCE*>(params.presReferences);
        uint32_t       numRefIdx  = CODEC_MAX_NUM_REF_FRAME;
        uint32_t       step       = 1;

        // When one on one ref idx mapping is enabled, add active vdenc references into cmd
        // instead of full ref list in picture paramters
        if (params.oneOnOneMapping)
        {
            references = const_cast<PMOS_RESOURCE*>(params.presVdencReferences);
            step       = 2;
        }

        resourceParams.mocsParams.mocsTableIndex = nullptr;
        for (uint32_t i = 0; i < numRefIdx; i++)
        {
            if (references[i] != nullptr)
            {
                MOS_SURFACE details;
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, references[i], &details));

                MOS_MEMCOMP_STATE mmcMode = (params.PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
                    params.PostDeblockSurfMmcState : params.PreDeblockSurfMmcState;
                if (mmcMode == MOS_MEMCOMP_RC || mmcMode == MOS_MEMCOMP_MC)
                {
                    cmd.DW61.Value |= (MHW_MEDIA_MEMCOMP_ENABLED << (i * 2 * step)) | ((mmcMode == MOS_MEMCOMP_RC) << (i * 2 * step + 1));
                }

                resourceParams.presResource    = references[i];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.pdwCmd          = &(cmd.Refpicbaseaddr[i * step].DW0_1.Value[0]);
                resourceParams.dwLocationInCmd = (i * 2 * step) + 19;  // * 2 to account for QW rather than DW
                resourceParams.bIsWritable     = false;

                resourceParams.dwSharedMocsOffset = 51 - resourceParams.dwLocationInCmd;

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }
        InitMocsParams(resourceParams, &cmd.DW51.Value, 1, 6);
        // There is only one control DW51 for all references
        if (params.decodeInUse)
        {
            cmd.DW51.ReferncePictureMemoryObjectControlState = m_referncePictureMemoryObjectControlStateCtrlDecode.Gen12_7.Index << 1;
        }
        else
        {
            cmd.DW51.ReferncePictureMemoryObjectControlState = m_referncePictureMemoryObjectControlStateCtrlEncode.Gen12_7.Index << 1;
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;
        if (params.presMacroblockIldbStreamOutBuffer1 != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW57.Value, 1, 6);
            cmd.DW57.MacroblockIldbStreamoutBufferMemoryObjectControlState = m_macroblockIldbStreamoutBufferCtrl.Gen12_7.Index;

            resourceParams.presResource    = params.presMacroblockIldbStreamOutBuffer1;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW55.Value);
            resourceParams.dwLocationInCmd = 55;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.presMacroblockIldbStreamOutBuffer2 != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW60.Value, 1, 6);
            cmd.DW60.SecondMacroblockIldbStreamoutBufferMemoryObjectControlState = m_secondMacroblockIldbStreamoutBufferCtrl.Gen12_7.Index;

            resourceParams.presResource    = params.presMacroblockIldbStreamOutBuffer2;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW58.Value);
            resourceParams.dwLocationInCmd = 58;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.presSliceSizeStreamOutBuffer != nullptr)
        {
            InitMocsParams(resourceParams, &cmd.DW67.Value, 1, 6);
            cmd.DW67.SlicesizeStreamoutDataDestinationMemoryObjectControlState = m_slicesizeStreamoutDataDestinationCtrl.Gen12_7.Index;

            cmd.DW67.SlicesizeStreamoutDataDestinationMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

            resourceParams.presResource    = params.presSliceSizeStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW65.Value);
            resourceParams.dwLocationInCmd = 65;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__mfx__xe2_lpm_base__xe2_lpm__Impl)
};
}  // namespace xe2_lpm
}  // namespace xe2_lpm_base
}  // namespace mfx
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_MFX_IMPL_XE2_LPM_H__
