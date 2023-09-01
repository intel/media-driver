/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file      mhw_state_heap_xe_xpm.c  
//! \brief         This modules implements HW interface layer to be used on all platforms on
//!            all operating systems/DDIs, across MHW components.  
//!
#include "mhw_state_heap_xe_xpm.h"

MHW_STATE_HEAP_INTERFACE_XE_XPM::MHW_STATE_HEAP_INTERFACE_XE_XPM(
    PMOS_INTERFACE pInputOSInterface, int8_t bDynamicMode) 
    : MHW_STATE_HEAP_INTERFACE_G12_X(pInputOSInterface, bDynamicMode)
{
    MHW_FUNCTION_ENTER;
}

MHW_STATE_HEAP_INTERFACE_XE_XPM::~MHW_STATE_HEAP_INTERFACE_XE_XPM()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MHW_STATE_HEAP_INTERFACE_XE_XPM::SetSamplerState(
  void                        *pSampler,
  PMHW_SAMPLER_STATE_PARAM    pParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(pParam);

    if (pParam->pKernelState)
    {
        PMHW_KERNEL_STATE    pKernelState;
        uint32_t             dwCurrSampler;
        uint32_t             dwNumSampler;

        mhw_state_heap_xe_xpm::SAMPLER_STATE_CMD Cmd;
        Cmd.DW0.MinModeFilter = Cmd.MIN_MODE_FILTER_LINEAR;
        Cmd.DW0.MagModeFilter = Cmd.MAG_MODE_FILTER_LINEAR;
        Cmd.DW1.ShadowFunction = Cmd.SHADOW_FUNCTION_PREFILTEROPNEVER;
        Cmd.DW3.TczAddressControlMode = Cmd.TCZ_ADDRESS_CONTROL_MODE_CLAMP;
        Cmd.DW3.TcyAddressControlMode = Cmd.TCY_ADDRESS_CONTROL_MODE_CLAMP;
        Cmd.DW3.TcxAddressControlMode = Cmd.TCX_ADDRESS_CONTROL_MODE_CLAMP;
        Cmd.DW3.RAddressMinFilterRoundingEnable = true;
        Cmd.DW3.RAddressMagFilterRoundingEnable = true;
        Cmd.DW3.VAddressMinFilterRoundingEnable = true;
        Cmd.DW3.VAddressMagFilterRoundingEnable = true;
        Cmd.DW3.UAddressMinFilterRoundingEnable = true;
        Cmd.DW3.UAddressMagFilterRoundingEnable = true;
        Cmd.DW0.SamplerDisable = false;

        dwNumSampler = pParam->pKernelState->KernelParams.iSamplerCount;

        for (dwCurrSampler = 0; dwCurrSampler < dwNumSampler; dwCurrSampler++)
        {
            if (pParam[dwCurrSampler].bInUse)
            {
                pKernelState = pParam[dwCurrSampler].pKernelState;

                MHW_MI_CHK_NULL(pKernelState);

                MHW_MI_CHK_STATUS(pKernelState->m_dshRegion.AddData(
                    &Cmd,
                    pKernelState->dwSamplerOffset + dwCurrSampler * Cmd.byteSize,
                    sizeof(Cmd)));
            }
        }
    }
    else if (pParam->bInUse)
    {
        MHW_MI_CHK_NULL(pSampler);

        if (pParam->SamplerType == MHW_SAMPLER_TYPE_3D)
        {
            mhw_state_heap_xe_xpm::SAMPLER_STATE_CMD *pUnormSampler =
                (mhw_state_heap_xe_xpm::SAMPLER_STATE_CMD*)pSampler;

            mhw_state_heap_xe_xpm::SAMPLER_STATE_CMD UnormSamplerInit;

            // Add additional defaults specific to media
            UnormSamplerInit.DW0.MinModeFilter = UnormSamplerInit.MIN_MODE_FILTER_LINEAR;
            UnormSamplerInit.DW0.MagModeFilter = UnormSamplerInit.MAG_MODE_FILTER_LINEAR;
            UnormSamplerInit.DW0.SamplerDisable = false;
            UnormSamplerInit.DW1.ShadowFunction = UnormSamplerInit.SHADOW_FUNCTION_PREFILTEROPNEVER;
            UnormSamplerInit.DW3.TczAddressControlMode = UnormSamplerInit.TCZ_ADDRESS_CONTROL_MODE_CLAMP;
            UnormSamplerInit.DW3.TcyAddressControlMode = UnormSamplerInit.TCY_ADDRESS_CONTROL_MODE_CLAMP;
            UnormSamplerInit.DW3.TcxAddressControlMode = UnormSamplerInit.TCX_ADDRESS_CONTROL_MODE_CLAMP;
            UnormSamplerInit.DW3.RAddressMinFilterRoundingEnable = true;
            UnormSamplerInit.DW3.RAddressMagFilterRoundingEnable = true;
            UnormSamplerInit.DW3.VAddressMinFilterRoundingEnable = true;
            UnormSamplerInit.DW3.VAddressMagFilterRoundingEnable = true;
            UnormSamplerInit.DW3.UAddressMinFilterRoundingEnable = true;
            UnormSamplerInit.DW3.UAddressMagFilterRoundingEnable = true;

            *pUnormSampler = UnormSamplerInit;

            if (pParam->Unorm.SamplerFilterMode == MHW_SAMPLER_FILTER_NEAREST)
            {
                pUnormSampler->DW0.MinModeFilter = pUnormSampler->MIN_MODE_FILTER_NEAREST;
                pUnormSampler->DW0.MagModeFilter = pUnormSampler->MAG_MODE_FILTER_NEAREST;
            }
            else if (pParam->Unorm.SamplerFilterMode == MHW_SAMPLER_FILTER_BILINEAR)
            {
                pUnormSampler->DW0.MinModeFilter = pUnormSampler->MIN_MODE_FILTER_LINEAR;
                pUnormSampler->DW0.MagModeFilter = pUnormSampler->MAG_MODE_FILTER_LINEAR;
            }
            else
            {
                pUnormSampler->DW0.MinModeFilter = pParam->Unorm.MinFilter;
                pUnormSampler->DW0.MagModeFilter = pParam->Unorm.MagFilter;
            }

            pUnormSampler->DW3.TcxAddressControlMode = pParam->Unorm.AddressU;
            pUnormSampler->DW3.TcyAddressControlMode = pParam->Unorm.AddressV;
            pUnormSampler->DW3.TczAddressControlMode = pParam->Unorm.AddressW;

            if (pParam->Unorm.bBorderColorIsValid)
            {
                mhw_state_heap_xe_xpm::SAMPLER_INDIRECT_STATE_CMD *pUnormSamplerBorderColor =
                    (mhw_state_heap_xe_xpm::SAMPLER_INDIRECT_STATE_CMD*)pParam->Unorm.pIndirectState;

                mhw_state_heap_xe_xpm::SAMPLER_INDIRECT_STATE_CMD UnormSamplerBorderColorInit;
                *pUnormSamplerBorderColor = UnormSamplerBorderColorInit;

                // Since the structure is a union between float, uint, and int, can use any to set the state DW
                pUnormSamplerBorderColor->DW0.BorderColorRed = pParam->Unorm.BorderColorRedU;
                pUnormSamplerBorderColor->DW1.BorderColorGreen = pParam->Unorm.BorderColorGreenU;
                pUnormSamplerBorderColor->DW2.BorderColorBlue = pParam->Unorm.BorderColorBlueU;
                pUnormSamplerBorderColor->DW3.BorderColorAlpha = pParam->Unorm.BorderColorAlphaU;

                pUnormSampler->DW2.IndirectStatePointer = pParam->Unorm.IndirectStateOffset >> MHW_SAMPLER_INDIRECT_SHIFT;
            }
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid sampler type '%d'", pParam->SamplerType);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return eStatus;
}

MOS_STATUS MHW_STATE_HEAP_INTERFACE_XE_XPM::AddSamplerStateData(
    uint32_t                    samplerOffset,
    MemoryBlock                 *memoryBlock,
    PMHW_SAMPLER_STATE_PARAM    pParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(memoryBlock);
    MHW_MI_CHK_NULL(pParam);

    if (pParam->SamplerType == MHW_SAMPLER_TYPE_3D)
    {
        mhw_state_heap_xe_xpm::SAMPLER_STATE_CMD          unormSampler;
        mhw_state_heap_xe_xpm::SAMPLER_INDIRECT_STATE_CMD indirectState;

        pParam->Unorm.pIndirectState = &indirectState;

        MHW_MI_CHK_STATUS(SetSamplerState(&unormSampler, pParam));

        // Add indirect state to heap if necessary
        if (pParam->Unorm.bBorderColorIsValid)
        {
            // adjust unormSampler->DW2.IndirectStatePointer
            // to include memoryBlock's offset from base of state heap
            unormSampler.DW2.IndirectStatePointer =
                (pParam->Unorm.IndirectStateOffset + memoryBlock->GetOffset())
                >> MHW_SAMPLER_INDIRECT_SHIFT;

            MHW_MI_CHK_STATUS(memoryBlock->AddData(
                &indirectState,
                pParam->Unorm.IndirectStateOffset,
                sizeof(mhw_state_heap_xe_xpm::SAMPLER_INDIRECT_STATE_CMD)));
        }

        // Add sampler state data to heap
        MHW_MI_CHK_STATUS(memoryBlock->AddData(
            &unormSampler,
            samplerOffset,
            sizeof(mhw_state_heap_xe_xpm::SAMPLER_STATE_CMD)));
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid sampler type '%d'", pParam->SamplerType);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS MHW_STATE_HEAP_INTERFACE_XE_XPM::SetSurfaceStateEntry(
    PMHW_SURFACE_STATE_PARAMS pParams)
{
    if (!pParams)
    {
        MHW_ASSERTMESSAGE("Invalid parameter\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t TileMode = (pParams->bGMMTileEnabled) ? pParams->TileModeGMM : ((pParams->bTiledSurface) ? ((pParams->bTileWalk == 0) ? 2 /*x-tile*/ : 3 /*y-tile*/) : 0); /*linear*/


    // Obtain the Pointer to the Surface state from SSH Buffer

    mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD* pSurfaceState = 
        (mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD*) pParams->pSurfaceState;
    MHW_MI_CHK_NULL(pSurfaceState);

    // Initialize Surface State
    *pSurfaceState = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD();

    pSurfaceState->DW0.SurfaceType               = pParams->SurfaceType3D;
    pSurfaceState->DW0.SurfaceFormat             = pParams->dwFormat;
    pSurfaceState->DW0.TileMode                  = TileMode;
    pSurfaceState->DW0.VerticalLineStride        = pParams->bVerticalLineStride;
    pSurfaceState->DW0.VerticalLineStrideOffset  = pParams->bVerticalLineStrideOffset;
    pSurfaceState->DW0.SurfaceHorizontalAlignment = 1;
    pSurfaceState->DW0.SurfaceVerticalAlignment   = 1;

    pSurfaceState->DW1.MemoryObjectControlState  = pParams->dwCacheabilityControl;
    if (pParams->SurfaceType3D == GFX3DSTATE_SURFACETYPE_BUFFER)
    {   // Buffer resources - use original width/height/pitch/depth
        pSurfaceState->DW2.Width                 = pParams->dwWidth;
        pSurfaceState->DW2.Height                = pParams->dwHeight;
        pSurfaceState->DW3.SurfacePitch          = pParams->dwPitch;
        pSurfaceState->DW3.Depth                 = pParams->dwDepth;
    }
    else
    {
        pSurfaceState->DW1.SurfaceQpitch         = pParams->dwQPitch >> 2;
        pSurfaceState->DW2.Width                 = pParams->dwWidth - 1;
        pSurfaceState->DW2.Height                = pParams->dwHeight - 1;
        pSurfaceState->DW3.SurfacePitch          = pParams->dwPitch - 1;
        pSurfaceState->DW3.Depth                 = pParams->dwDepth - 1;
    }
    pSurfaceState->DW4.RenderTargetAndSampleUnormRotation      = pParams->RotationMode;
    pSurfaceState->DW5.XOffset                          = pParams->iXOffset >> 2;
    pSurfaceState->DW5.YOffset                          = pParams->iYOffset >> 2;
    pSurfaceState->DW6.Obj2.SeparateUvPlaneEnable       = pParams->bSeperateUVPlane;
    pSurfaceState->DW6.Obj2.HalfPitchForChroma          = pParams->bHalfPitchChroma;
    pSurfaceState->DW6.Obj2.XOffsetForUOrUvPlane        = pParams->dwXOffsetForU;
    pSurfaceState->DW6.Obj2.YOffsetForUOrUvPlane        = pParams->dwYOffsetForU;

    // Set L1 Cache control
    pSurfaceState->DW5.L1CachePolicy = pParams->L1CacheConfig;

    // R8B8G8A8 is designed to represent media AYUV format.
    // But from Gen10+ 3D sampler doesn't support R8B8G8A8 format any more.
    // Use R8G8B8A8 + Channel Select to fake it.
    if (pParams->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_R8B8G8A8_UNORM)
    {
        pSurfaceState->DW0.SurfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
        pSurfaceState->DW7.ShaderChannelSelectAlpha = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ALPHA;
        pSurfaceState->DW7.ShaderChannelSelectBlue  = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_GREEN;
        pSurfaceState->DW7.ShaderChannelSelectGreen = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_BLUE;
        pSurfaceState->DW7.ShaderChannelSelectRed   = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_RED;
    }
    else
    {
        pSurfaceState->DW7.ShaderChannelSelectAlpha = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ALPHA;
        pSurfaceState->DW7.ShaderChannelSelectBlue  = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_BLUE;
        pSurfaceState->DW7.ShaderChannelSelectGreen = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_GREEN;
        pSurfaceState->DW7.ShaderChannelSelectRed   = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_RED;
    }

    // The OGL border color mode is much simpler to support in HW as no filtering of
    // missing channels is required. Software will be required to use OGL border color
    // mode (enabled via Sampler State) and use Shader Channel Selects to force return
    // of 1.0 on missing channels.
    if (pParams->bBoardColorOGL)
    {
        SetMissingShaderChannels(pSurfaceState, pParams->dwFormat);
    }

    if (pParams->MmcState == MOS_MEMCOMP_MC)
    {
        // Media Compresseion Enable on Current surface
        pSurfaceState->DW7.MemoryCompressionEnable          = 1;
        pSurfaceState->DW7.MemoryCompressionMode            = 0;
        pSurfaceState->DW4.DecompressInL3                   = 1;

        pSurfaceState->DW10_11.Obj1.XOffsetForVPlane        = pParams->dwXOffsetForV;
        pSurfaceState->DW10_11.Obj0.YOffsetForVPlane        = pParams->dwYOffsetForV;
        pSurfaceState->DW12.CompressionFormat               = pParams->dwCompressionFormat;
    }
    else if(pParams->MmcState == MOS_MEMCOMP_RC)
    {
        // Render Compression Enable on Current Surface
        pSurfaceState->DW7.MemoryCompressionEnable = 0;
        pSurfaceState->DW7.MemoryCompressionMode   = 0;
        pSurfaceState->DW4.DecompressInL3          = 0;
        pSurfaceState->DW6.Obj0.AuxiliarySurfaceMode = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::AUXILIARY_SURFACE_MODE_AUXCCSE;
        // Disable CC because MEdia didn't have FCC input usage
        pSurfaceState->DW10_11.Obj0.ClearValueAddressEnable     = 0;
        pSurfaceState->DW12.CompressionFormat = pParams->dwCompressionFormat;
    }
    else
    {
        pSurfaceState->DW10_11.Obj1.XOffsetForVPlane = pParams->dwXOffsetForV;
        pSurfaceState->DW10_11.Obj0.YOffsetForVPlane = pParams->dwYOffsetForV;
    }

    pSurfaceState->DW8_9.SurfaceBaseAddress             = 0;

    // Return offset and pointer for patching
    pParams->pdwCmd          = (uint32_t *)&(pSurfaceState->DW8_9.SurfaceBaseAddress);
    pParams->dwLocationInCmd = 8;


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MHW_STATE_HEAP_INTERFACE_XE_XPM::SetMissingShaderChannels(
    mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD *pSurfaceState,
    uint32_t dwFormat)
{
    if (!pSurfaceState)
    {
        MHW_ASSERTMESSAGE("Invalid parameter\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    switch (dwFormat)
    {
        // A8_UNORM must be forced to 0.0 which is the default for border color anyway.
    case MHW_GFX3DSTATE_SURFACEFORMAT_A8_UNORM:
        pSurfaceState->DW7.ShaderChannelSelectRed = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ZERO;
        pSurfaceState->DW7.ShaderChannelSelectGreen = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ZERO;
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ZERO;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_R64G64_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_SINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_UINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_USCALED:
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        pSurfaceState->DW7.ShaderChannelSelectAlpha = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R64G64B64_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPY:
        pSurfaceState->DW7.ShaderChannelSelectAlpha = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_FLOAT_X8X24_TYPELESS:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R64_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_USCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8_SSCALED:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8_USCALED:
        pSurfaceState->DW7.ShaderChannelSelectGreen = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ONE;
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        pSurfaceState->DW7.ShaderChannelSelectAlpha = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_X32_TYPELESS_G8X24_UINT:
        pSurfaceState->DW7.ShaderChannelSelectRed = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ONE;
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        pSurfaceState->DW7.ShaderChannelSelectAlpha = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_A32X32_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_A24X8_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_A32_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_A16_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_A16_FLOAT:
        pSurfaceState->DW7.ShaderChannelSelectRed = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ONE;
        pSurfaceState->DW7.ShaderChannelSelectGreen = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ONE;
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_BC5_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_BC5_SNORM:
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_SINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_UINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R32_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R24_UNORM_X8_TYPELESS:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_SINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R16_FLOAT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8_SNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8_SINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_R8_UINT:
    case MHW_GFX3DSTATE_SURFACEFORMAT_BC4_UNORM:
    case MHW_GFX3DSTATE_SURFACEFORMAT_BC4_SNORM:
        pSurfaceState->DW7.ShaderChannelSelectGreen = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ONE;
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        break;
    case MHW_GFX3DSTATE_SURFACEFORMAT_X24_TYPELESS_G8_UINT:
        pSurfaceState->DW7.ShaderChannelSelectRed = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ONE;
        pSurfaceState->DW7.ShaderChannelSelectBlue = mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
        break;
    default:
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MHW_STATE_HEAP_INTERFACE_XE_XPM::SetInterfaceDescriptor(
    uint32_t                         dwNumIdsToSet,
    PMHW_INTERFACE_DESCRIPTOR_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;
    MHW_MI_CHK_NULL(pParams);

    for (uint32_t dwCurrId = 0; dwCurrId < dwNumIdsToSet; dwCurrId++)
    {
        PMHW_KERNEL_STATE pKernelState = pParams[dwCurrId].pKernelState;

        MHW_MI_CHK_NULL(pKernelState);

        mhw_state_heap_xe_xpm::INTERFACE_DESCRIPTOR_DATA_CMD cmd;

        cmd.DW0.KernelStartPointer =
            (pKernelState->m_ishRegion.GetOffset() +
                pKernelState->dwKernelBinaryOffset +
                pParams[dwCurrId].dwKernelStartOffset) >>
            MHW_KERNEL_OFFSET_SHIFT;
        cmd.DW3.SamplerStatePointer =
            (pKernelState->m_dshRegion.GetOffset() +
                pKernelState->dwSamplerOffset +
                pParams[dwCurrId].dwSamplerOffset) >>
            MHW_SAMPLER_SHIFT;
        cmd.DW3.SamplerCount        = (pKernelState->KernelParams.iSamplerCount - 1) / 4 + 1;
        cmd.DW4.BindingTablePointer = MOS_ROUNDUP_SHIFT(
            (pKernelState->dwSshOffset +
                pParams[dwCurrId].dwBtOffset),
            MHW_BINDING_TABLE_ID_SHIFT);
        cmd.DW5.ConstantIndirectUrbEntryReadLength = MOS_ROUNDUP_SHIFT(
            pParams->pKernelState->KernelParams.iCurbeLength,
            MHW_CURBE_SHIFT);
        cmd.DW6.NumberOfThreadsInGpgpuThreadGroup = 1;

        uint32_t idOffsetInIdSpace =
            pKernelState->dwIdOffset +
            (pParams[dwCurrId].dwIdIdx * m_wSizeOfCmdInterfaceDescriptorData);
        MHW_MI_CHK_STATUS(pKernelState->m_dshRegion.AddData(
            &cmd,
            idOffsetInIdSpace,
            cmd.byteSize));
    }
    return eStatus;
}
