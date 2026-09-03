/*
* Copyright (c) 2026, Intel Corporation
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
//! \file      mhw_state_heap_xe3p_m_plus_base.h
//! \brief     State heap interface base shared from Xe3P onwards
//! \details   Command struct templated state heap implementation. A platform
//!            derives a concrete interface by instantiating this template with
//!            its own state heap command struct class, for example:
//!
//!              struct MHW_STATE_HEAP_INTERFACE_<PLATFORM> :
//!                  public MHW_STATE_HEAP_INTERFACE_XE3P_M_PLUS_BASE<mhw_state_heap_<platform>>
//!              {
//!                  MHW_STATE_HEAP_INTERFACE_<PLATFORM>(PMOS_INTERFACE pOsItf, int8_t bDynamicMode)
//!                      : base_t(pOsItf, bDynamicMode) {}
//!              };
//!
//!            and only carries the deltas against this base.
//!
//!            NOTE: the base constructor calls InitHwSizes(). A leaf that needs
//!            different HW sizes must therefore re-apply them after the base
//!            constructor has run, because virtual dispatch during base
//!            construction resolves to this class.
//!

#ifndef __MHW_STATE_HEAP_XE3P_M_PLUS_BASE_H__
#define __MHW_STATE_HEAP_XE3P_M_PLUS_BASE_H__

#include "mhw_state_heap_generic.h"
#include "mhw_utilities_next.h"
#include "mhw_cp_interface.h"

template <class TCmds>
struct MHW_STATE_HEAP_INTERFACE_XE3P_M_PLUS_BASE : public MHW_STATE_HEAP_INTERFACE_GENERIC<TCmds>
{
protected:
    using base_t = MHW_STATE_HEAP_INTERFACE_GENERIC<TCmds>;

    //! State heap command structs of the instantiating platform. Declaring them
    //! here keeps the member function bodies free of typename qualification.
    using RENDER_SURFACE_STATE_CMD               = typename TCmds::RENDER_SURFACE_STATE_CMD;
    using MEDIA_SURFACE_STATE_CMD                = typename TCmds::MEDIA_SURFACE_STATE_CMD;
    using BINDING_TABLE_STATE_CMD                = typename TCmds::BINDING_TABLE_STATE_CMD;
    using INTERFACE_DESCRIPTOR_DATA_CMD          = typename TCmds::INTERFACE_DESCRIPTOR_DATA_CMD;
    using SAMPLER_STATE_CMD                      = typename TCmds::SAMPLER_STATE_CMD;
    using SAMPLER_INDIRECT_STATE_CMD             = typename TCmds::SAMPLER_INDIRECT_STATE_CMD;
    using SAMPLER_STATE_8x8_AVS_CMD              = typename TCmds::SAMPLER_STATE_8x8_AVS_CMD;
    using SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD = typename TCmds::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD;

    //! Members inherited through a dependent base. The using declarations make
    //! them visible to unqualified lookup inside the member function bodies.
    using base_t::m_pOsInterface;
    using base_t::m_wIdAlignment;
    using base_t::m_wBtIdxAlignment;
    using base_t::m_wCurbeAlignment;
    using base_t::m_wSizeOfCmdSamplerState;
    using base_t::m_wSizeOfCmdInterfaceDescriptorData;
    using base_t::m_dwMaxSurfaceStateSize;
    using base_t::m_pfnAddResourceToCmd;
    using base_t::m_SurfaceStateHeap;
    using base_t::m_HwSizes;
    using base_t::m_mhwGenericOffsetShift;
    using base_t::m_mhwNumBindingTableEntryOffset;

private:
    uint16_t m_wSizeOfInterfaceDescriptor = 0;
    uint32_t m_dwSizeSurfaceState         = 0;
    uint32_t m_dwSizeSurfaceStateAdv      = 0;

public:
    MHW_STATE_HEAP_INTERFACE_XE3P_M_PLUS_BASE(
        PMOS_INTERFACE pInputOSInterface,
        int8_t         bDynamicMode) : base_t(pInputOSInterface, bDynamicMode)
    {
        m_wBtIdxAlignment = m_mhwNumBindingTableEntryOffset;
        m_wIdAlignment    = (1 << m_mhwGenericOffsetShift);
        m_wCurbeAlignment = (1 << m_mhwGenericOffsetShift);

        m_dwSizeSurfaceState                = MOS_ALIGN_CEIL(RENDER_SURFACE_STATE_CMD::byteSize, MHW_SURFACE_STATE_ALIGN);
        m_dwSizeSurfaceStateAdv             = MOS_ALIGN_CEIL(MEDIA_SURFACE_STATE_CMD::byteSize, MHW_SURFACE_STATE_ALIGN);
        m_dwMaxSurfaceStateSize             = MOS_MAX(m_dwSizeSurfaceState, m_dwSizeSurfaceStateAdv);
        m_wSizeOfInterfaceDescriptor        = INTERFACE_DESCRIPTOR_DATA_CMD::byteSize;
        m_wSizeOfCmdInterfaceDescriptorData = MOS_ALIGN_CEIL(m_wSizeOfInterfaceDescriptor, m_wIdAlignment);
        m_wSizeOfCmdSamplerState            = SAMPLER_STATE_CMD::byteSize;

        InitHwSizes();
    }

    ~MHW_STATE_HEAP_INTERFACE_XE3P_M_PLUS_BASE()
    {
        MHW_FUNCTION_ENTER;
    }

    MOS_STATUS InitHwSizes()
    {
        m_HwSizes.dwSizeMediaObjectHeaderCmd   = MHW_MEDIA_OBJECT_BYTE_SIZE;
        m_HwSizes.dwSizeSurfaceState           = MOS_ALIGN_CEIL(RENDER_SURFACE_STATE_CMD::byteSize, MHW_SURFACE_STATE_ALIGN);
        m_HwSizes.dwSizeSurfaceStateAvs        = MOS_ALIGN_CEIL(MEDIA_SURFACE_STATE_CMD::byteSize, MHW_SURFACE_STATE_ALIGN);
        m_HwSizes.dwMaxSizeSurfaceState        = MOS_MAX(m_HwSizes.dwSizeSurfaceState, m_HwSizes.dwSizeSurfaceStateAvs);
        m_HwSizes.dwSizeBindingTableState      = BINDING_TABLE_STATE_CMD::byteSize;
        m_HwSizes.dwSizeSamplerState           = SAMPLER_STATE_CMD::byteSize;
        m_HwSizes.dwSizeSamplerIndirectState   = SAMPLER_INDIRECT_STATE_CMD::byteSize;
        m_HwSizes.dwSizeSamplerStateVA         = 0;
        m_HwSizes.dwSizeSamplerStateVAConvolve = 0;
        m_HwSizes.dwSizeSamplerStateTable8x8   = SAMPLER_STATE_8x8_AVS_CMD::byteSize - 16 * sizeof(uint32_t);  // match old definitions to table size
        m_HwSizes.dwSizeSampler8x8Table        =
            MOS_ALIGN_CEIL(m_HwSizes.dwSizeSamplerStateTable8x8, MHW_SAMPLER_STATE_ALIGN);
        m_HwSizes.dwSizeSamplerStateAvs        =
            MOS_ALIGN_CEIL(SAMPLER_STATE_8x8_AVS_CMD::byteSize, MHW_SAMPLER_STATE_AVS_ALIGN_MEDIA);
        m_HwSizes.dwSizeInterfaceDescriptor = INTERFACE_DESCRIPTOR_DATA_CMD::byteSize;
        m_HwSizes.dwSizeMediaWalkerBlock       = 16;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetInterfaceDescriptorEntry(
        PMHW_ID_ENTRY_PARAMS      pParams)
    {
        MHW_FUNCTION_ENTER;
        // Remove setting Descriptor in stateheap.
        // Command will be set in compute walker.
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddInterfaceDescriptorData(
        PMHW_ID_ENTRY_PARAMS      pParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        //------------------------------------
        MHW_MI_CHK_NULL(pParams);
        //------------------------------------

        // Ensures that the Media ID base is correct
        MHW_ASSERT(MOS_IS_ALIGNED(pParams->dwMediaIdOffset, m_wIdAlignment));

        uint32_t offset = pParams->dwMediaIdOffset + pParams->iMediaId * m_wSizeOfInterfaceDescriptor;
        INTERFACE_DESCRIPTOR_DATA_CMD *pInterfaceDescriptor;
        pInterfaceDescriptor = (INTERFACE_DESCRIPTOR_DATA_CMD *)MOS_AllocMemory(sizeof(INTERFACE_DESCRIPTOR_DATA_CMD));
        MHW_MI_CHK_NULL(pInterfaceDescriptor);
        *pInterfaceDescriptor = INTERFACE_DESCRIPTOR_DATA_CMD();

        pInterfaceDescriptor->DW0.KernelStartPointer                 = pParams->dwKernelOffset >> MHW_KERNEL_OFFSET_SHIFT;
        pInterfaceDescriptor->DW3.SamplerStatePointer                = pParams->dwSamplerOffset >> MHW_SAMPLER_SHIFT;
        pInterfaceDescriptor->DW3.SamplerCount                       = pParams->dwSamplerCount;
        pInterfaceDescriptor->DW4.BindingTablePointer                = MOS_ROUNDUP_SHIFT(pParams->dwBindingTableOffset, MHW_BINDING_TABLE_ID_SHIFT);
        pInterfaceDescriptor->DW5.NumberOfThreadsInGpgpuThreadGroup  = pParams->dwNumberofThreadsInGPGPUGroup;
        pInterfaceDescriptor->DW5.SharedLocalMemorySize              = pParams->dwSharedLocalMemorySize;

        // need to subtract memory block's offset in current state heap for AddData API
        offset -= pParams->memoryBlock->GetOffset();
        eStatus = pParams->memoryBlock->AddData(pInterfaceDescriptor, offset,
            sizeof(INTERFACE_DESCRIPTOR_DATA_CMD));

        MOS_SafeFreeMemory(pInterfaceDescriptor);
        MHW_MI_CHK_STATUS(eStatus);
        return eStatus;
    }

    MOS_STATUS SetSurfaceStateEntry(
            PMHW_SURFACE_STATE_PARAMS   pParams)
    {
        if (!pParams)
        {
            MHW_ASSERTMESSAGE("Invalid parameter\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        uint32_t TileMode = (pParams->bGMMTileEnabled) ? pParams->TileModeGMM : ((pParams->bTiledSurface) ? ((pParams->bTileWalk == 0) ? 2 /*x-tile*/ : 3 /*y-tile*/) : 0); /*linear*/

        // Obtain the Pointer to the Surface state from SSH Buffer

        RENDER_SURFACE_STATE_CMD *pSurfaceState =
            (RENDER_SURFACE_STATE_CMD *)pParams->pSurfaceState;
        MHW_MI_CHK_NULL(pSurfaceState);

        // Initialize Surface State
        *pSurfaceState = RENDER_SURFACE_STATE_CMD();

        pSurfaceState->DW0.SurfaceType                = pParams->SurfaceType3D;
        pSurfaceState->DW0.SurfaceFormat              = pParams->dwFormat;
        pSurfaceState->DW0.TileMode                   = TileMode;
        pSurfaceState->DW0.VerticalLineStride         = pParams->bVerticalLineStride;
        pSurfaceState->DW0.VerticalLineStrideOffset   = pParams->bVerticalLineStrideOffset;
        pSurfaceState->DW0.SurfaceHorizontalAlignment = 1;
        pSurfaceState->DW0.SurfaceVerticalAlignment   = 1;

        pSurfaceState->DW1.MemoryObjectControlState = pParams->dwCacheabilityControl;
        if (pParams->SurfaceType3D == GFX3DSTATE_SURFACETYPE_BUFFER)
        {  // Buffer resources - use original width/height/pitch/depth
            pSurfaceState->DW2.Width        = pParams->dwWidth;
            pSurfaceState->DW2.Height       = pParams->dwHeight;
            pSurfaceState->DW3.SurfacePitch = pParams->dwPitch;
            pSurfaceState->DW3.Depth        = pParams->dwDepth;
        }
        else
        {
            pSurfaceState->DW1.SurfaceQpitch = pParams->dwQPitch >> 2;
            pSurfaceState->DW2.Width         = pParams->dwWidth - 1;
            pSurfaceState->DW2.Height        = pParams->dwHeight - 1;
            pSurfaceState->DW3.SurfacePitch  = pParams->dwPitch - 1;
            pSurfaceState->DW3.Depth         = pParams->dwDepth - 1;
        }

        MT_LOG6(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"SurfaceState"),
                    MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_TYPE, pSurfaceState->DW0.SurfaceType, MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_WIDTH, pSurfaceState->DW2.Width,
                    MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_HEIGHT,  pSurfaceState->DW2.Height, MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_FORMAT, pSurfaceState->DW0.SurfaceFormat,
                    MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, pParams->dwCacheabilityControl);
        MHW_NORMALMESSAGE("Feature Graph: Cache settings of Render SurfaceState: SurfaceType is %d, Surface width is %d, Surface height is %d,\
                    Surface format is %d, SurfaceMemoryObjectControlState %d, Index to Mocs table %d",pSurfaceState->DW0.SurfaceType, pSurfaceState->DW2.Width,
                   pSurfaceState->DW2.Height, pSurfaceState->DW0.SurfaceFormat, pParams->dwCacheabilityControl, (pParams->dwCacheabilityControl >> 1) & 0x0000003f);

        pSurfaceState->DW4.RenderTargetAndSampleUnormRotation = pParams->RotationMode;
        pSurfaceState->DW5.XOffset                            = pParams->iXOffset >> 2;
        pSurfaceState->DW5.YOffset                            = pParams->iYOffset >> 2;
        pSurfaceState->DW6.Obj2.SeparateUvPlaneEnable         = pParams->bSeperateUVPlane;
        pSurfaceState->DW6.Obj2.HalfPitchForChroma            = pParams->bHalfPitchChroma;
        pSurfaceState->DW6.Obj2.XOffsetForUOrUvPlane          = pParams->dwXOffsetForU;
        pSurfaceState->DW6.Obj2.YOffsetForUOrUvPlane          = pParams->dwYOffsetForU;

        // Set L1 Cache control
        pSurfaceState->DW5.L1CacheControl = pParams->L1CacheConfig;

        if (pParams->MipTailStartLOD & MHW_MIP_TAIL_START_LOD_ENABLED_MASK)
        {
            pSurfaceState->DW5.MipTailStartLod = pParams->MipTailStartLOD & (~MHW_MIP_TAIL_START_LOD_ENABLED_MASK);
        }

        // R8B8G8A8 is designed to represent media AYUV format.
        // But from Gen10+ 3D sampler doesn't support R8B8G8A8 format any more.
        // Use R8G8B8A8 + Channel Select to fake it.
        if (pParams->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_R8B8G8A8_UNORM)
        {
            pSurfaceState->DW0.SurfaceFormat            = MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
            pSurfaceState->DW7.ShaderChannelSelectAlpha = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ALPHA;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_GREEN;
            pSurfaceState->DW7.ShaderChannelSelectGreen = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_BLUE;
            pSurfaceState->DW7.ShaderChannelSelectRed   = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_RED;
        }
        else
        {
            pSurfaceState->DW7.ShaderChannelSelectAlpha = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ALPHA;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_BLUE;
            pSurfaceState->DW7.ShaderChannelSelectGreen = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_GREEN;
            pSurfaceState->DW7.ShaderChannelSelectRed   = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_RED;
        }

        // The newer border color mode is much simpler to support in HW as no filtering of
        // missing channels is required. Software will be required to use the newer border color
        // mode (enabled via Sampler State) and use Shader Channel Selects to force return
        // of 1.0 on missing channels.
        if (pParams->bBoardColorOGL)
        {
            SetMissingShaderChannels(pSurfaceState, pParams->dwFormat);
        }

        if (pParams->MmcState == MOS_MEMCOMP_MC)
        {
            pSurfaceState->DW10_11.Obj1.XOffsetForVPlane = pParams->dwXOffsetForV;
            pSurfaceState->DW10_11.Obj0.YOffsetForVPlane = pParams->dwYOffsetForV;
            pSurfaceState->DW12.CompressionFormat        = pParams->dwCompressionFormat;
        }
        else if (pParams->MmcState == MOS_MEMCOMP_RC)
        {
            pSurfaceState->DW12.CompressionFormat               = pParams->dwCompressionFormat;
        }
        else
        {
            pSurfaceState->DW10_11.Obj1.XOffsetForVPlane = pParams->dwXOffsetForV;
            pSurfaceState->DW10_11.Obj0.YOffsetForVPlane = pParams->dwYOffsetForV;
        }

        pSurfaceState->DW8_9.SurfaceBaseAddress = 0;

        // Return offset and pointer for patching
        pParams->pdwCmd          = (uint32_t *)&(pSurfaceState->DW8_9.SurfaceBaseAddress);
        pParams->dwLocationInCmd = 8;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetSurfaceState(
        PMHW_KERNEL_STATE           pKernelState,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        uint32_t                    dwNumSurfaceStatesToSet,
        PMHW_RCS_SURFACE_PARAMS     pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetSamplerState(
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

            SAMPLER_STATE_CMD Cmd;
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
                SAMPLER_STATE_CMD *pUnormSampler =
                    (SAMPLER_STATE_CMD *)pSampler;

                 SAMPLER_STATE_CMD UnormSamplerInit;

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
                    SAMPLER_INDIRECT_STATE_CMD *pUnormSamplerBorderColor =
                        (SAMPLER_INDIRECT_STATE_CMD *)pParam->Unorm.pIndirectState;

                     SAMPLER_INDIRECT_STATE_CMD UnormSamplerBorderColorInit;
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

    MOS_STATUS AddSamplerStateData(
        uint32_t                    samplerOffset,
        MemoryBlock                 *memoryBlock,
        PMHW_SAMPLER_STATE_PARAM    pParam)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_MI_CHK_NULL(memoryBlock);
        MHW_MI_CHK_NULL(pParam);

        if (pParam->SamplerType == MHW_SAMPLER_TYPE_3D)
        {
            SAMPLER_STATE_CMD          unormSampler;
            SAMPLER_INDIRECT_STATE_CMD indirectState;

            pParam->Unorm.pIndirectState = &indirectState;

            MHW_MI_CHK_STATUS(SetSamplerState(&unormSampler, pParam));

            // Add indirect state to heap if necessary
            if (pParam->Unorm.bBorderColorIsValid)
            {
                // adjust unormSampler->DW2.IndirectStatePointer
                // to include memoryBlock's offset from base of state heap
                unormSampler.DW2.IndirectStatePointer =
                    (pParam->Unorm.IndirectStateOffset + memoryBlock->GetOffset()) >> MHW_SAMPLER_INDIRECT_SHIFT;

                MHW_MI_CHK_STATUS(memoryBlock->AddData(
                    &indirectState,
                    pParam->Unorm.IndirectStateOffset,
                    sizeof(SAMPLER_INDIRECT_STATE_CMD)));
            }

            // Add sampler state data to heap
            MHW_MI_CHK_STATUS(memoryBlock->AddData(
                &unormSampler,
                samplerOffset,
                sizeof(SAMPLER_STATE_CMD)));
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid sampler type '%d'", pParam->SamplerType);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS InitSamplerStates(
        void                        *pSamplerStates,
        int32_t                     iSamplers)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(pSamplerStates);

        SAMPLER_STATE_CMD SamplerStateInit;
        // Add additional defaults specific to media
        SamplerStateInit.DW0.MinModeFilter = SamplerStateInit.MIN_MODE_FILTER_LINEAR;
        SamplerStateInit.DW0.MagModeFilter = SamplerStateInit.MAG_MODE_FILTER_LINEAR;
        SamplerStateInit.DW0.SamplerDisable = true;
        SamplerStateInit.DW1.ShadowFunction = SamplerStateInit.SHADOW_FUNCTION_PREFILTEROPNEVER;
        SamplerStateInit.DW3.TczAddressControlMode = SamplerStateInit.TCZ_ADDRESS_CONTROL_MODE_CLAMP;
        SamplerStateInit.DW3.TcyAddressControlMode = SamplerStateInit.TCY_ADDRESS_CONTROL_MODE_CLAMP;
        SamplerStateInit.DW3.TcxAddressControlMode = SamplerStateInit.TCX_ADDRESS_CONTROL_MODE_CLAMP;
        SamplerStateInit.DW3.RAddressMinFilterRoundingEnable = true;
        SamplerStateInit.DW3.RAddressMagFilterRoundingEnable = true;
        SamplerStateInit.DW3.VAddressMinFilterRoundingEnable = true;
        SamplerStateInit.DW3.VAddressMagFilterRoundingEnable = true;
        SamplerStateInit.DW3.UAddressMinFilterRoundingEnable = true;
        SamplerStateInit.DW3.UAddressMagFilterRoundingEnable = true;

        // Initialize Media Sampler States
        uint8_t *pu8SamplerState = (uint8_t*)pSamplerStates;

        for (; iSamplers > 0; iSamplers--)
        {
            MOS_SecureMemcpy(pu8SamplerState, SamplerStateInit.byteSize, &SamplerStateInit, SamplerStateInit.byteSize);
            pu8SamplerState += SamplerStateInit.byteSize;
        }

        return eStatus;
    }

    //!
    //! \brief    Load Sampler 8X8 State Table
    //! \details  Load Sampler 8x8 State Table
    //! \param    [in] void  *pvTable
    //!           Pointer to 8x8 table in GSH to load
    //! \param    [in] PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam
    //!           Pointer to 8x8 sampler state params
    //! \return   MOS_STATUS
    //!
    MOS_STATUS LoadSamplerAvsTable(
        void                         *pvTable,
        PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(pvTable);
        MHW_MI_CHK_NULL(pMhwSamplerAvsTableParam);

        SAMPLER_STATE_8x8_AVS_CMD *pSampler8x8Avs =
            (SAMPLER_STATE_8x8_AVS_CMD *)pvTable;

        // DW0 ~ DW15 are for sampler state and programmed in other function, so no need to setup for it here.

        uint32_t u32ConvolveTableNum =
            sizeof(pSampler8x8Avs->FilterCoefficient016) / sizeof(pSampler8x8Avs->FilterCoefficient016[0]);

        // DW16 ~ DW151 setting for table coefficients (DW0 ~ DW7) * 17
        for (uint32_t u32CoeffTableIdx = 0; u32CoeffTableIdx < u32ConvolveTableNum; u32CoeffTableIdx++)
        {
            PMHW_AVS_COEFFICIENT_PARAM   pCoeffParam = &pMhwSamplerAvsTableParam->paMhwAvsCoeffParam[u32CoeffTableIdx];
            SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD *pCoeffTable =
                &pSampler8x8Avs->FilterCoefficient016[u32CoeffTableIdx];
            MHW_MI_CHK_NULL(pCoeffParam);
            MHW_MI_CHK_NULL(pCoeffTable);

            pCoeffTable->DW0.Table0XFilterCoefficientN0 = pCoeffParam->ZeroXFilterCoefficient[0];
            pCoeffTable->DW0.Table0YFilterCoefficientN0 = pCoeffParam->ZeroYFilterCoefficient[0];
            pCoeffTable->DW0.Table0XFilterCoefficientN1 = pCoeffParam->ZeroXFilterCoefficient[1];
            pCoeffTable->DW0.Table0YFilterCoefficientN1 = pCoeffParam->ZeroYFilterCoefficient[1];

            pCoeffTable->DW1.Table0XFilterCoefficientN2 = pCoeffParam->ZeroXFilterCoefficient[2];
            pCoeffTable->DW1.Table0YFilterCoefficientN2 = pCoeffParam->ZeroYFilterCoefficient[2];
            pCoeffTable->DW1.Table0XFilterCoefficientN3 = pCoeffParam->ZeroXFilterCoefficient[3];
            pCoeffTable->DW1.Table0YFilterCoefficientN3 = pCoeffParam->ZeroYFilterCoefficient[3];

            pCoeffTable->DW2.Table0XFilterCoefficientN4 = pCoeffParam->ZeroXFilterCoefficient[4];
            pCoeffTable->DW2.Table0YFilterCoefficientN4 = pCoeffParam->ZeroYFilterCoefficient[4];
            pCoeffTable->DW2.Table0XFilterCoefficientN5 = pCoeffParam->ZeroXFilterCoefficient[5];
            pCoeffTable->DW2.Table0YFilterCoefficientN5 = pCoeffParam->ZeroYFilterCoefficient[5];

            pCoeffTable->DW3.Table0XFilterCoefficientN6 = pCoeffParam->ZeroXFilterCoefficient[6];
            pCoeffTable->DW3.Table0YFilterCoefficientN6 = pCoeffParam->ZeroYFilterCoefficient[6];
            pCoeffTable->DW3.Table0XFilterCoefficientN7 = pCoeffParam->ZeroXFilterCoefficient[7];
            pCoeffTable->DW3.Table0YFilterCoefficientN7 = pCoeffParam->ZeroYFilterCoefficient[7];

            pCoeffTable->DW4.Table1XFilterCoefficientN2 = pCoeffParam->OneXFilterCoefficient[0];
            pCoeffTable->DW4.Table1XFilterCoefficientN3 = pCoeffParam->OneXFilterCoefficient[1];
            pCoeffTable->DW5.Table1XFilterCoefficientN4 = pCoeffParam->OneXFilterCoefficient[2];
            pCoeffTable->DW5.Table1XFilterCoefficientN5 = pCoeffParam->OneXFilterCoefficient[3];

            pCoeffTable->DW6.Table1YFilterCoefficientN2 = pCoeffParam->OneYFilterCoefficient[0];
            pCoeffTable->DW6.Table1YFilterCoefficientN3 = pCoeffParam->OneYFilterCoefficient[1];
            pCoeffTable->DW7.Table1YFilterCoefficientN4 = pCoeffParam->OneYFilterCoefficient[2];
            pCoeffTable->DW7.Table1YFilterCoefficientN5 = pCoeffParam->OneYFilterCoefficient[3];
        }

        // DW152 ~ DW153 setting for table control
        pSampler8x8Avs->DW152.TransitionAreaWith8Pixels = pMhwSamplerAvsTableParam->byteTransitionArea8Pixels; // 3-bits
        pSampler8x8Avs->DW152.TransitionAreaWith4Pixels = pMhwSamplerAvsTableParam->byteTransitionArea4Pixels; // 3-bits
        pSampler8x8Avs->DW152.MaxDerivative8Pixels      = pMhwSamplerAvsTableParam->byteMaxDerivative8Pixels;
        pSampler8x8Avs->DW152.MaxDerivative4Pixels      = pMhwSamplerAvsTableParam->byteMaxDerivative4Pixels;
        pSampler8x8Avs->DW152.DefaultSharpnessLevel     = pMhwSamplerAvsTableParam->byteDefaultSharpnessLevel;

        pSampler8x8Avs->DW153.RgbAdaptive                   = pMhwSamplerAvsTableParam->bEnableRGBAdaptive;
        pSampler8x8Avs->DW153.AdaptiveFilterForAllChannels  = pMhwSamplerAvsTableParam->bAdaptiveFilterAllChannels;
        pSampler8x8Avs->DW153.BypassYAdaptiveFiltering      = pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering;
        pSampler8x8Avs->DW153.BypassXAdaptiveFiltering      = pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering;

        u32ConvolveTableNum =
            sizeof(pSampler8x8Avs->FilterCoefficient1731) / sizeof(pSampler8x8Avs->FilterCoefficient1731[0]);
        // DW160 ~ DW279 setting for extra table coefficients (DW0 ~ DW7) * 15
        for (uint32_t u32CoeffTableIdx = 0; u32CoeffTableIdx < u32ConvolveTableNum; u32CoeffTableIdx++)
        {
            PMHW_AVS_COEFFICIENT_PARAM   pCoeffParamExtra = &pMhwSamplerAvsTableParam->paMhwAvsCoeffParamExtra[u32CoeffTableIdx];
            SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD *pCoeffTableExtra =
                &pSampler8x8Avs->FilterCoefficient1731[u32CoeffTableIdx];
            MHW_MI_CHK_NULL(pCoeffParamExtra);
            MHW_MI_CHK_NULL(pCoeffTableExtra);

            pCoeffTableExtra->DW0.Table0XFilterCoefficientN0 = pCoeffParamExtra->ZeroXFilterCoefficient[0];
            pCoeffTableExtra->DW0.Table0YFilterCoefficientN0 = pCoeffParamExtra->ZeroYFilterCoefficient[0];
            pCoeffTableExtra->DW0.Table0XFilterCoefficientN1 = pCoeffParamExtra->ZeroXFilterCoefficient[1];
            pCoeffTableExtra->DW0.Table0YFilterCoefficientN1 = pCoeffParamExtra->ZeroYFilterCoefficient[1];

            pCoeffTableExtra->DW1.Table0XFilterCoefficientN2 = pCoeffParamExtra->ZeroXFilterCoefficient[2];
            pCoeffTableExtra->DW1.Table0YFilterCoefficientN2 = pCoeffParamExtra->ZeroYFilterCoefficient[2];
            pCoeffTableExtra->DW1.Table0XFilterCoefficientN3 = pCoeffParamExtra->ZeroXFilterCoefficient[3];
            pCoeffTableExtra->DW1.Table0YFilterCoefficientN3 = pCoeffParamExtra->ZeroYFilterCoefficient[3];

            pCoeffTableExtra->DW2.Table0XFilterCoefficientN4 = pCoeffParamExtra->ZeroXFilterCoefficient[4];
            pCoeffTableExtra->DW2.Table0YFilterCoefficientN4 = pCoeffParamExtra->ZeroYFilterCoefficient[4];
            pCoeffTableExtra->DW2.Table0XFilterCoefficientN5 = pCoeffParamExtra->ZeroXFilterCoefficient[5];
            pCoeffTableExtra->DW2.Table0YFilterCoefficientN5 = pCoeffParamExtra->ZeroYFilterCoefficient[5];

            pCoeffTableExtra->DW3.Table0XFilterCoefficientN6 = pCoeffParamExtra->ZeroXFilterCoefficient[6];
            pCoeffTableExtra->DW3.Table0YFilterCoefficientN6 = pCoeffParamExtra->ZeroYFilterCoefficient[6];
            pCoeffTableExtra->DW3.Table0XFilterCoefficientN7 = pCoeffParamExtra->ZeroXFilterCoefficient[7];
            pCoeffTableExtra->DW3.Table0YFilterCoefficientN7 = pCoeffParamExtra->ZeroYFilterCoefficient[7];

            pCoeffTableExtra->DW4.Table1XFilterCoefficientN2 = pCoeffParamExtra->OneXFilterCoefficient[0];
            pCoeffTableExtra->DW4.Table1XFilterCoefficientN3 = pCoeffParamExtra->OneXFilterCoefficient[1];
            pCoeffTableExtra->DW5.Table1XFilterCoefficientN4 = pCoeffParamExtra->OneXFilterCoefficient[2];
            pCoeffTableExtra->DW5.Table1XFilterCoefficientN5 = pCoeffParamExtra->OneXFilterCoefficient[3];

            pCoeffTableExtra->DW6.Table1YFilterCoefficientN2 = pCoeffParamExtra->OneYFilterCoefficient[0];
            pCoeffTableExtra->DW6.Table1YFilterCoefficientN3 = pCoeffParamExtra->OneYFilterCoefficient[1];
            pCoeffTableExtra->DW7.Table1YFilterCoefficientN4 = pCoeffParamExtra->OneYFilterCoefficient[2];
            pCoeffTableExtra->DW7.Table1YFilterCoefficientN5 = pCoeffParamExtra->OneYFilterCoefficient[3];
        }

        return eStatus;
    }

    MOS_STATUS SetInterfaceDescriptor(
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

            INTERFACE_DESCRIPTOR_DATA_CMD cmd;

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
            cmd.DW5.NumberOfThreadsInGpgpuThreadGroup = 1;

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

    MOS_STATUS SetMissingShaderChannels(
        RENDER_SURFACE_STATE_CMD *pSurfaceState,
        uint32_t dwFormat)
    {
        if (!pSurfaceState)
        {
            MHW_ASSERTMESSAGE("Invalid parameter\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        switch (dwFormat)
        {
            // A8_UNORM must be forced to 0.0 which is the default for the newer border color anyway.
        case MHW_GFX3DSTATE_SURFACEFORMAT_A8_UNORM:
            pSurfaceState->DW7.ShaderChannelSelectRed   = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ZERO;
            pSurfaceState->DW7.ShaderChannelSelectGreen = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ZERO;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ZERO;
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
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
            pSurfaceState->DW7.ShaderChannelSelectAlpha = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
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
            pSurfaceState->DW7.ShaderChannelSelectAlpha = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
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
            pSurfaceState->DW7.ShaderChannelSelectGreen = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ONE;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
            pSurfaceState->DW7.ShaderChannelSelectAlpha = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
            break;
        case MHW_GFX3DSTATE_SURFACEFORMAT_X32_TYPELESS_G8X24_UINT:
            pSurfaceState->DW7.ShaderChannelSelectRed   = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ONE;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
            pSurfaceState->DW7.ShaderChannelSelectAlpha = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_ALPHA_ONE;
            break;
        case MHW_GFX3DSTATE_SURFACEFORMAT_A32X32_FLOAT:
        case MHW_GFX3DSTATE_SURFACEFORMAT_A24X8_UNORM:
        case MHW_GFX3DSTATE_SURFACEFORMAT_A32_FLOAT:
        case MHW_GFX3DSTATE_SURFACEFORMAT_A16_UNORM:
        case MHW_GFX3DSTATE_SURFACEFORMAT_A16_FLOAT:
            pSurfaceState->DW7.ShaderChannelSelectRed   = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ONE;
            pSurfaceState->DW7.ShaderChannelSelectGreen = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ONE;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
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
            pSurfaceState->DW7.ShaderChannelSelectBlue = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
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
            pSurfaceState->DW7.ShaderChannelSelectGreen = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_GREEN_ONE;
            pSurfaceState->DW7.ShaderChannelSelectBlue  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
            break;
        case MHW_GFX3DSTATE_SURFACEFORMAT_X24_TYPELESS_G8_UINT:
            pSurfaceState->DW7.ShaderChannelSelectRed  = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_RED_ONE;
            pSurfaceState->DW7.ShaderChannelSelectBlue = RENDER_SURFACE_STATE_CMD::SHADER_CHANNEL_SELECT_BLUE_ONE;
            break;
        default:
            break;
        }

        return MOS_STATUS_SUCCESS;
    }
};

#endif  // __MHW_STATE_HEAP_XE3P_M_PLUS_BASE_H__
