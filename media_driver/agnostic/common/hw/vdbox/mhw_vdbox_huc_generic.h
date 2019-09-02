/*
* Copyright (c) 2017-2018, Intel Corporation
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

//! \file     mhw_vdbox_huc_generic.h
//! \brief    MHW interface for constructing HUC commands for the Vdbox engine
//! \details  Impelements shared Vdbox HUC command construction functions across all platforms as templates
//!

#ifndef _MHW_VDBOX_HUC_GENERIC_H_
#define _MHW_VDBOX_HUC_GENERIC_H_

#include "mhw_vdbox_huc_interface.h"

//!  MHW Vdbox Huc generic interface
/*!
This class defines the shared Huc command construction functions across all platforms as templates
*/
template <class THucCmds, class TMiCmds>
class MhwVdboxHucInterfaceGeneric : public MhwVdboxHucInterface
{
protected:
    #define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)
    //!
    //! \enum     CommandsNumberOfAddresses
    //! \brief    Commands number of addresses
    //!
    enum CommandsNumberOfAddresses
    {
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address field
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                        =  1, //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES    =  1, //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field

        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for  0 address fields

        HUC_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        HUC_IMEM_STATE_CMD_NUMBER_OF_ADDRESSES                     =  0, //  0 DW for    address fields
        HUC_DMEM_STATE_CMD_NUMBER_OF_ADDRESSES                     =  2, //  3 DW for  2 address fields
        HUC_VIRTUAL_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES             = 16, // 32 DW for 16 address fields
        HUC_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES        =  4, //  8 DW for  4 address fields
        HUC_STREAM_OBJECT_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HUC_START_CMD_NUMBER_OF_ADDRESSES                          =  0, //  0 DW for    address fields
    };

    //!
    //! \brief    Constructor
    //!
    MhwVdboxHucInterfaceGeneric(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface) :
        MhwVdboxHucInterface(osInterface, miInterface, cpInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief   Destructor
    //!
    virtual ~MhwVdboxHucInterfaceGeneric() {}

protected:
    MOS_STATUS GetHucStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
    {
        MHW_FUNCTION_ENTER;

        uint32_t maxSize = 0;
        uint32_t patchListMaxSize = 0;
        uint32_t standard = CodecHal_GetStandardFromMode(mode);
        uint32_t numSlices = 1;
        uint32_t numStoreDataImm = 1;
        uint32_t numStoreReg = 1;

        if (mode == CODECHAL_DECODE_MODE_HEVCVLD && params->bShortFormat)
        {
            numSlices       = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6;
            numStoreDataImm = 2;
            numStoreReg     = 2;

            maxSize +=
                2 * TMiCmds::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize +=
                2 * PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }
        else if (standard == CODECHAL_CENC)
        {
            numStoreDataImm = 3;
            numStoreReg     = 3;

            maxSize +=
                TMiCmds::MI_FLUSH_DW_CMD::byteSize * 2 +
                TMiCmds::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2;

        }
        else if (mode == CODECHAL_ENCODE_MODE_VP9)
        {
            // for huc status 2 register and semaphore signal and reset
            numStoreDataImm = 3;

            maxSize +=
                TMiCmds::MI_BATCH_BUFFER_END_CMD::byteSize +
                TMiCmds::MI_FLUSH_DW_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);
        }
        else if (mode == CODECHAL_ENCODE_MODE_AVC)
        {
            numStoreDataImm = 2;
            numStoreReg     = 2;

            maxSize +=
                2 * TMiCmds::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize +=
                2 * PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }

        maxSize +=
            THucCmds::HUC_PIPE_MODE_SELECT_CMD::byteSize +
            THucCmds::HUC_IMEM_STATE_CMD::byteSize +
            THucCmds::HUC_DMEM_STATE_CMD::byteSize +
            THucCmds::HUC_VIRTUAL_ADDR_STATE_CMD::byteSize +
            THucCmds::HUC_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            numSlices       * THucCmds::HUC_STREAM_OBJECT_CMD::byteSize +
            numSlices       * THucCmds::HUC_START_CMD::byteSize +
            numStoreDataImm * TMiCmds::MI_STORE_DATA_IMM_CMD::byteSize +
            numStoreReg     * TMiCmds::MI_STORE_REGISTER_MEM_CMD::byteSize;

        patchListMaxSize +=
            PATCH_LIST_COMMAND(HUC_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(HUC_IMEM_STATE_CMD) +
            PATCH_LIST_COMMAND(HUC_DMEM_STATE_CMD) +
            PATCH_LIST_COMMAND(HUC_VIRTUAL_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HUC_IND_OBJ_BASE_ADDR_STATE_CMD) +
            numSlices       * PATCH_LIST_COMMAND(HUC_STREAM_OBJECT_CMD) +
            numSlices       * PATCH_LIST_COMMAND(HUC_START_CMD) +
            numStoreDataImm * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) +
            numStoreReg     * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD);

        if (params->bHucDummyStream)
        {
            maxSize +=
                THucCmds::HUC_PIPE_MODE_SELECT_CMD::byteSize +
                THucCmds::HUC_IMEM_STATE_CMD::byteSize +
                THucCmds::HUC_DMEM_STATE_CMD::byteSize +
                THucCmds::HUC_VIRTUAL_ADDR_STATE_CMD::byteSize +
                THucCmds::HUC_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                THucCmds::HUC_STREAM_OBJECT_CMD::byteSize +
                THucCmds::HUC_START_CMD::byteSize +
                TMiCmds::MI_FLUSH_DW_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(HUC_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(HUC_IMEM_STATE_CMD) +
                PATCH_LIST_COMMAND(HUC_DMEM_STATE_CMD) +
                PATCH_LIST_COMMAND(HUC_VIRTUAL_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(HUC_IND_OBJ_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(HUC_STREAM_OBJECT_CMD) +
                PATCH_LIST_COMMAND(HUC_START_CMD) +
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetHucPrimitiveCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        uint32_t            maxSize = 0;
        uint32_t            patchListMaxSize = 0;

        // Hal will handling the multiplication of cmd size instead of MHW

        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

    MOS_STATUS AddHucPipeModeSelectCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   *params)
    {
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename THucCmds::HUC_PIPE_MODE_SELECT_CMD cmd;

        if (!params->disableProtectionSetting)
        {
            MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForHucPipeModeSelect((uint32_t*)&cmd));
        }

        cmd.DW1.IndirectStreamOutEnable = params->bStreamOutEnabled;
        cmd.DW2.MediaSoftResetCounterPer1000Clocks = params->dwMediaSoftResetCounterValue;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddHucImemStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_IMEM_STATE_PARAMS     *params)
    {
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename THucCmds::HUC_IMEM_STATE_CMD cmd;

        cmd.DW4.HucFirmwareDescriptor = params->dwKernelDescriptor;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddHucDmemStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_DMEM_STATE_PARAMS     *params)
    {

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS                         resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_HUC_DMEM;

        typename THucCmds::HUC_DMEM_STATE_CMD cmd;

        if (params->presHucDataSource)
        {
            resourceParams.presResource = params->presHucDataSource;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.HucDataSourceBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.bIsWritable = false;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            // set HuC data destination address
            cmd.DW4.HucDataDestinationBaseAddress = params->dwDmemOffset >> MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;

            // set data length
            cmd.DW5.HucDataLength = params->dwDataLength >> MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddHucVirtualAddrStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS   *params)
    {
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS                                     resourceParams;

        // set up surface 0~15
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_HUC_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_HUC_VIRTUAL_ADDR;

        typename THucCmds::HUC_VIRTUAL_ADDR_STATE_CMD cmd;

        for (int i = 0; i < 16; i++)
        {
            if (params->regionParams[i].presRegion)
            {
                cmd.HucVirtualAddressRegion[i].HucSurfaceVirtualaddrregion015.DW0.Value |=
                    m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HUC_VIRTUAL_ADDR_REGION_BUFFER_CODEC].Value;
                resourceParams.presResource = params->regionParams[i].presRegion;
                resourceParams.dwOffset = params->regionParams[i].dwOffset;
                resourceParams.bIsWritable = params->regionParams[i].isWritable;
                resourceParams.pdwCmd = cmd.HucVirtualAddressRegion[i].HucSurfaceBaseAddressVirtualaddrregion015.DW0_1.Value;
                resourceParams.dwLocationInCmd = 1 + (i * 3);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddHucIndObjBaseAddrStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  *params)
    {
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS                                     resourceParams;

        // base address of the bit-stream buffer
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_HUC_UPPER_BOUND_STATE_SHIFT;
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;
        resourceParams.HwCommandType = MOS_HUC_IND_OBJ_BASE_ADDR;

        typename THucCmds::HUC_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

        if (params->presDataBuffer)
        {
            resourceParams.presResource = params->presDataBuffer;
            resourceParams.dwOffset = params->dwDataOffset;
            resourceParams.pdwCmd = cmd.HucIndirectStreamInObjectbaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 1;
            resourceParams.bIsWritable = false;
            resourceParams.dwSize = params->dwDataSize;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presStreamOutObjectBuffer)
        {
            // base address of the stream out buffer
            resourceParams.presResource = params->presStreamOutObjectBuffer;
            resourceParams.dwOffset = params->dwStreamOutObjectOffset;
            resourceParams.pdwCmd = cmd.HucIndirectStreamOutObjectbaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 6;
            resourceParams.bIsWritable = true;
            resourceParams.dwSize = params->dwStreamOutObjectSize;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddHucStreamObjectCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_STREAM_OBJ_PARAMS     *params)
    {
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename THucCmds::HUC_STREAM_OBJECT_CMD cmd;

        cmd.DW1.IndirectStreamInDataLength = params->dwIndStreamInLength;
        cmd.DW2.IndirectStreamInStartAddress = params->dwIndStreamInStartAddrOffset;
        cmd.DW2.HucProcessing = params->bHucProcessing;
        cmd.DW3.IndirectStreamOutStartAddress = params->dwIndStreamOutStartAddrOffset;
        cmd.DW4.HucBitstreamEnable = params->bStreamInEnable;
        cmd.DW4.StreamOut = params->bStreamOutEnable;
        cmd.DW4.EmulationPreventionByteRemoval = params->bEmulPreventionByteRemoval;
        cmd.DW4.StartCodeSearchEngine = params->bStartCodeSearchEngine;
        cmd.DW4.Drmlengthmode = params->bLengthModeEnabled;
        cmd.DW4.StartCodeByte2 = params->ucStartCodeByte2;
        cmd.DW4.StartCodeByte1 = params->ucStartCodeByte1;
        cmd.DW4.StartCodeByte0 = params->ucStartCodeByte0;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddHucStartCmd(
        MOS_COMMAND_BUFFER             *cmdBuffer,
        bool                            lastStreamObject)
    {
        MHW_MI_CHK_NULL(cmdBuffer);

        typename THucCmds::HUC_START_CMD cmd;

        // set last stream object or not
        cmd.DW1.Laststreamobject = (lastStreamObject != 0);

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }
};

#endif
