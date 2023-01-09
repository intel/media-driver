/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_render_generic.h
//! \brief    MHW interface templates for render engine commands
//! \details  Impelements shared HW command construction functions across all platforms as templates
//!

#ifndef __MHW_RENDER_GENERIC_H__
#define __MHW_RENDER_GENERIC_H__

#include "mhw_render_legacy.h"

template <class TRenderCmds>
class MhwRenderInterfaceGeneric : public MhwRenderInterface
{
protected:
    MhwRenderInterfaceGeneric(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested) :
        MhwRenderInterface(miInterface, osInterface, gtSystemInfo, newStateHeapManagerRequested){}

public:
    virtual ~MhwRenderInterfaceGeneric() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddPipelineSelectCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        bool                            gpGpuPipe)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);

        typename TRenderCmds::PIPELINE_SELECT_CMD  cmd;
        cmd.DW0.PipelineSelection = (gpGpuPipe) ? cmd.PIPELINE_SELECTION_GPGPU : cmd.PIPELINE_SELECTION_MEDIA;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddStateBaseAddrCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_STATE_BASE_ADDR_PARAMS         params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum      = MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT;
        resourceParams.HwCommandType = MOS_STATE_BASE_ADDR;

        typename TRenderCmds::STATE_BASE_ADDRESS_CMD cmd;

        if (params->presGeneralState)
        {
            cmd.DW1_2.GeneralStateBaseAddressModifyEnable    = true;
            cmd.DW12.GeneralStateBufferSizeModifyEnable      = true;
            resourceParams.presResource                      = params->presGeneralState;
            resourceParams.dwOffset                          = 0;
            resourceParams.pdwCmd                            = cmd.DW1_2.Value;
            resourceParams.dwLocationInCmd                   = 1;

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW1_2.Value[0], 5, 10);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            if (params->mocs4GeneralState != 0)
            {
                cmd.DW1_2.GeneralStateMemoryObjectControlState   = params->mocs4GeneralState;
            }

            cmd.DW12.GeneralStateBufferSize = (params->dwGeneralStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
        }

        if (m_osInterface->bNoParsingAssistanceInKmd)
        {
            uint32_t indirectStateOffset, indirectStateSize;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetIndirectState(m_osInterface, &indirectStateOffset, &indirectStateSize));

            // When KMD parsing assistance is not used,
            // UMD is required to set up the SSH
            // in the STATE_BASE_ADDRESS command.
            // All addresses used in the STATE_BASE_ADDRESS
            // command need to have the modify
            // bit associated with it set to 1.
            cmd.DW4_5.SurfaceStateBaseAddressModifyEnable    = true;
            resourceParams.presResource                      = &cmdBuffer->OsResource;
            resourceParams.dwOffset                          = indirectStateOffset;
            resourceParams.pdwCmd                            = cmd.DW4_5.Value;
            resourceParams.dwLocationInCmd                   = 4;

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW4_5.Value[0], 5, 10);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            if (params->mocs4SurfaceState != 0)
            {
                cmd.DW4_5.SurfaceStateMemoryObjectControlState = params->mocs4SurfaceState;
            }
        }

        if (params->presDynamicState)
        {
            cmd.DW6_7.DynamicStateBaseAddressModifyEnable    = true;
            cmd.DW13.DynamicStateBufferSizeModifyEnable      = true;
            resourceParams.presResource                      = params->presDynamicState;
            resourceParams.dwOffset                          = 0;
            resourceParams.pdwCmd                            = cmd.DW6_7.Value;
            resourceParams.dwLocationInCmd                   = 6;
            resourceParams.bIsWritable                       = params->bDynamicStateRenderTarget;

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW6_7.Value[0], 5, 10);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            if (params->mocs4DynamicState != 0)
            {
                cmd.DW6_7.DynamicStateMemoryObjectControlState = params->mocs4DynamicState;
            }

            cmd.DW13.DynamicStateBufferSize                  = (params->dwDynamicStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;

            //Reset bRenderTarget as it should be enabled only for Dynamic State
            resourceParams.bIsWritable = false;
        }

        if (params->presIndirectObjectBuffer)
        {
            cmd.DW8_9.IndirectObjectBaseAddressModifyEnable     = true;
            cmd.DW14.IndirectObjectBufferSizeModifyEnable       = true;
            resourceParams.presResource                         = params->presIndirectObjectBuffer;
            resourceParams.dwOffset                             = 0;
            resourceParams.pdwCmd                               = cmd.DW8_9.Value;
            resourceParams.dwLocationInCmd                      = 8;

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd    = 0;

            InitMocsParams(resourceParams, &cmd.DW8_9.Value[0], 5, 10);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            if (params->mocs4IndirectObjectBuffer != 0)
            {
                cmd.DW8_9.IndirectObjectMemoryObjectControlState = params->mocs4IndirectObjectBuffer;
            }

            cmd.DW14.IndirectObjectBufferSize                   = (params->dwIndirectObjectBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
        }

        if (params->presInstructionBuffer)
        {
            cmd.DW10_11.InstructionBaseAddressModifyEnable   = true;
            cmd.DW15.InstructionBufferSizeModifyEnable       = true;
            resourceParams.presResource                      = params->presInstructionBuffer;
            resourceParams.dwOffset                          = 0;
            resourceParams.pdwCmd                            = cmd.DW10_11.Value;
            resourceParams.dwLocationInCmd                   = 10;

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW10_11.Value[0], 5, 10);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            if (params->mocs4InstructionCache != 0)
            {
                cmd.DW10_11.InstructionMemoryObjectControlState = params->mocs4InstructionCache;
            }

            cmd.DW15.InstructionBufferSize = (params->dwInstructionBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
        }

        // stateless dataport access
        cmd.DW3.StatelessDataPortAccessMemoryObjectControlState = params->mocs4StatelessDataport;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TRenderCmds::MEDIA_VFE_STATE_CMD cmd;

        if (params->pKernelState)
        {
            MHW_MI_CHK_NULL(params->pKernelState);

            cmd.DW3.MaximumNumberOfThreads          = (params->dwMaximumNumberofThreads) ?
                params->dwMaximumNumberofThreads - 1 : params->pKernelState->KernelParams.iThreadCount - 1;
            cmd.DW5.CurbeAllocationSize             =
                MOS_ROUNDUP_SHIFT(params->pKernelState->KernelParams.iCurbeLength, 5);
            cmd.DW5.UrbEntryAllocationSize          = MOS_MAX(1,
                MOS_ROUNDUP_SHIFT(params->pKernelState->KernelParams.iInlineDataLength, 5));

            uint32_t numberofURBEntries   =
                (m_hwCaps.dwMaxURBSize -
                cmd.DW5.CurbeAllocationSize -
                params->pKernelState->KernelParams.iIdCount) /
                cmd.DW5.UrbEntryAllocationSize;
            numberofURBEntries            = MOS_CLAMP_MIN_MAX(numberofURBEntries, 1, 64);
            cmd.DW3.NumberOfUrbEntries      = numberofURBEntries;
        }
        else
        {
            if (params->dwNumberofURBEntries == 0)
            {
                MHW_ASSERTMESSAGE("Parameter dwNumberofURBEntries is 0 will cause divided by zero.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (params->dwPerThreadScratchSpace)
            {
                cmd.DW1.PerThreadScratchSpace       = params->dwPerThreadScratchSpace;
                cmd.DW1.ScratchSpaceBasePointer     = params->dwScratchSpaceBasePointer >> 10;
                cmd.DW2.ScratchSpaceBasePointerHigh = 0;
            }
            cmd.DW3.MaximumNumberOfThreads          = (params->dwMaximumNumberofThreads) ?
                params->dwMaximumNumberofThreads - 1 : m_hwCaps.dwMaxThreads - 1;
            cmd.DW3.NumberOfUrbEntries              = params->dwNumberofURBEntries;
            cmd.DW5.CurbeAllocationSize             = params->dwCURBEAllocationSize >> 5;
            cmd.DW5.UrbEntryAllocationSize          = (params->dwURBEntryAllocationSize) ?
                params->dwURBEntryAllocationSize :
                (m_hwCaps.dwMaxURBSize -
                cmd.DW5.CurbeAllocationSize -
                m_hwCaps.dwMaxInterfaceDescriptorEntries) /
                params->dwNumberofURBEntries;
        }

        if ((cmd.DW3.NumberOfUrbEntries > m_hwCaps.dwMaxURBEntries) ||
            (cmd.DW5.CurbeAllocationSize > m_hwCaps.dwMaxCURBEAllocationSize) ||
            (cmd.DW5.UrbEntryAllocationSize > m_hwCaps.dwMaxURBEntryAllocationSize) ||
            (cmd.DW3.NumberOfUrbEntries * cmd.DW5.UrbEntryAllocationSize +
            cmd.DW5.CurbeAllocationSize +
            m_hwCaps.dwMaxInterfaceDescriptorEntries > m_hwCaps.dwMaxURBSize))
        {
            MHW_ASSERTMESSAGE("Parameters requested exceed maximum supported by HW.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMediaCurbeLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_CURBE_LOAD_PARAMS          params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TRenderCmds::MEDIA_CURBE_LOAD_CMD cmd;

        if (params->pKernelState)
        {
            MHW_MI_CHK_NULL(m_stateHeapInterface->pStateHeapInterface);

            auto kernelState = params->pKernelState;

            cmd.DW2.CurbeTotalDataLength    = (uint32_t)MOS_ALIGN_CEIL(
                kernelState->KernelParams.iCurbeLength,
                m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
            cmd.DW3.CurbeDataStartAddress   = MOS_ALIGN_CEIL(
                (kernelState->m_dshRegion.GetOffset() +
                kernelState->dwCurbeOffset),
                m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        }
        else
        {
            cmd.DW2.CurbeTotalDataLength    = params->dwCURBETotalDataLength;
            cmd.DW3.CurbeDataStartAddress   = params->dwCURBEDataStartAddress;
        }

        // Send the command only if there is data to load
        if (cmd.DW2.CurbeTotalDataLength)
        {
            MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
        }
        else
        {
            MHW_NORMALMESSAGE("MEDIA_CURBE_LOAD must load data. Command skipped.");
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMediaIDLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_ID_LOAD_PARAMS             params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(m_stateHeapInterface->pStateHeapInterface);

        typename TRenderCmds::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD cmd;

        if (params->pKernelState)
        {
            auto kernelState = params->pKernelState;

            cmd.DW2.InterfaceDescriptorTotalLength      =
                m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() * params->dwNumKernelsLoaded;
            cmd.DW3.InterfaceDescriptorDataStartAddress = MOS_ALIGN_CEIL(
                kernelState->m_dshRegion.GetOffset() +
                kernelState->dwIdOffset +
                (params->dwIdIdx * m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData()),
                m_stateHeapInterface->pStateHeapInterface->GetIdAlignment());
        }
        // Client managed MediaIDs
        else if (params->dwInterfaceDescriptorLength)
        {
            cmd.DW2.InterfaceDescriptorTotalLength      = params->dwInterfaceDescriptorLength;
            cmd.DW3.InterfaceDescriptorDataStartAddress = params->dwInterfaceDescriptorStartOffset;
        }

        if (cmd.DW2.InterfaceDescriptorTotalLength > 0)
        {
            MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
        }
        else
        {
            MHW_NORMALMESSAGE("MEDIA_INTERFACE_DESCRIPTOR_LOAD must load data. Command skipped.");
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMediaObject(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_OBJECT_PARAMS        params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TRenderCmds::MEDIA_OBJECT_CMD cmd;

        if (params->dwInlineDataSize > 0)
        {
            cmd.DW0.DwordLength             =
                TRenderCmds::GetOpLength(((params->dwInlineDataSize / sizeof(uint32_t)) + cmd.dwSize));
        }

        cmd.DW1.InterfaceDescriptorOffset   = params->dwInterfaceDescriptorOffset;
        cmd.DW2.IndirectDataLength          = params->dwIndirectLoadLength;
        cmd.DW2.SubsliceDestinationSelect   = params->dwHalfSliceDestinationSelect;
        cmd.DW2.SliceDestinationSelect      = params->dwSliceDestinationSelect;
        cmd.DW2.ForceDestination            = params->bForceDestination;
        cmd.DW3.IndirectDataStartAddress    = params->dwIndirectDataStartAddress;

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
            m_osInterface,
            cmdBuffer,
            batchBuffer,
            &cmd,
            cmd.byteSize));

        if (params->pInlineData && params->dwInlineDataSize > 0)
        {
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
                m_osInterface,
                cmdBuffer,
                batchBuffer,
                params->pInlineData,
                params->dwInlineDataSize));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMediaObjectWalkerCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_WALKER_PARAMS              params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TRenderCmds::MEDIA_OBJECT_WALKER_CMD cmd;

        if (params->pInlineData)
        {
            cmd.DW0.DwordLength =
                TRenderCmds::GetOpLength(cmd.dwSize + params->InlineDataLength / sizeof(uint32_t));
        }

        cmd.DW1.InterfaceDescriptorOffset   = params->InterfaceDescriptorOffset;
        cmd.DW5.GroupIdLoopSelect           = params->GroupIdLoopSelect;
        cmd.DW6.ColorCountMinusOne          = params->ColorCountMinusOne;
        cmd.DW6.MiddleLoopExtraSteps        = params->MiddleLoopExtraSteps;
        cmd.DW6.MidLoopUnitX                = params->MidLoopUnitX;
        cmd.DW6.LocalMidLoopUnitY           = params->MidLoopUnitY;
        cmd.DW7.LocalLoopExecCount          = params->dwLocalLoopExecCount;
        cmd.DW7.GlobalLoopExecCount         = params->dwGlobalLoopExecCount;
        cmd.DW8.BlockResolutionX            = params->BlockResolution.x;
        cmd.DW8.BlockResolutionY            = params->BlockResolution.y;
        cmd.DW9.LocalStartX                 = params->LocalStart.x;
        cmd.DW9.LocalStartY                 = params->LocalStart.y;
        cmd.DW11.LocalOuterLoopStrideX      = params->LocalOutLoopStride.x;
        cmd.DW11.LocalOuterLoopStrideY      = params->LocalOutLoopStride.y;
        cmd.DW12.LocalInnerLoopUnitX        = params->LocalInnerLoopUnit.x;
        cmd.DW12.LocalInnerLoopUnitY        = params->LocalInnerLoopUnit.y;
        cmd.DW13.GlobalResolutionX          = params->GlobalResolution.x;
        cmd.DW13.GlobalResolutionY          = params->GlobalResolution.y;
        cmd.DW14.GlobalStartX               = params->GlobalStart.x;
        cmd.DW14.GlobalStartY               = params->GlobalStart.y;
        cmd.DW15.GlobalOuterLoopStrideX     = params->GlobalOutlerLoopStride.x;
        cmd.DW15.GlobalOuterLoopStrideY     = params->GlobalOutlerLoopStride.y;
        cmd.DW16.GlobalInnerLoopUnitX       = params->GlobalInnerLoopUnit.x;
        cmd.DW16.GlobalInnerLoopUnitY       = params->GlobalInnerLoopUnit.y;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        if (params->pInlineData)
        {
            if (params->InlineDataLength > 0)
            {
                MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(
                    cmdBuffer,
                    params->pInlineData,
                    params->InlineDataLength));
            }
            else
            {
                MHW_NORMALMESSAGE("Inline data indicated, however cannot insert without valid length.");
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddGpGpuWalkerStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS        params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        if (params->ThreadDepth == 0)
        {
            params->ThreadDepth = 1;
        }
        if (params->GroupDepth == 0)
        {
            params->GroupDepth = 1;
        }

        typename TRenderCmds::GPGPU_WALKER_CMD cmd;
        cmd.DW1.InterfaceDescriptorOffset       = params->InterfaceDescriptorOffset;
        cmd.DW4.SimdSize                        = 2; // SIMD32
        cmd.DW4.ThreadWidthCounterMaximum       = params->ThreadWidth - 1;
        cmd.DW4.ThreadHeightCounterMaximum      = params->ThreadHeight - 1;
        cmd.DW4.ThreadDepthCounterMaximum       = params->ThreadDepth - 1;
        cmd.DW5.ThreadGroupIdStartingX          = 0;
        cmd.DW7.ThreadGroupIdXDimension         = params->GroupWidth;
        cmd.DW8.ThreadGroupIdStartingY          = 0;
        cmd.DW10.ThreadGroupIdYDimension        = params->GroupHeight;
        cmd.DW11.ThreadGroupIdStartingResumeZ   = 0;
        cmd.DW12.ThreadGroupIdZDimension        = params->GroupDepth;
        cmd.DW13.RightExecutionMask             = 0xffffffff;
        cmd.DW14.BottomExecutionMask            = 0xffffffff;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS AddChromaKeyCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_CHROMAKEY_PARAMS           params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TRenderCmds::_3DSTATE_CHROMA_KEY_CMD cmd;
        cmd.DW1.ChromakeyTableIndex = params->dwIndex;
        cmd.DW2.ChromakeyLowValue   = params->dwLow;
        cmd.DW3.ChromakeyHighValue  = params->dwHigh;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSipStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_SIP_STATE_PARAMS           params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TRenderCmds::STATE_SIP_CMD cmd;
        cmd.DW1_2.SystemInstructionPointer = (uint64_t)(params->dwSipBase >> 4);

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize))

        return MOS_STATUS_SUCCESS;
    }

    inline uint32_t GetMediaObjectCmdSize()
    {
        return TRenderCmds::MEDIA_OBJECT_CMD::byteSize;
    }
};

#endif // __MHW_RENDER_GENERIC_H__
