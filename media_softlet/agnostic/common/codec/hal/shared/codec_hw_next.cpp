/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     codec_hw_next.cpp
//! \brief    This modules implements HW interface layer to be used on all platforms on     all operating systems/DDIs, across CODECHAL components.
//!
#include "codec_hw_next.h"
#include "codechal_setting.h"

CodechalHwInterfaceNext::CodechalHwInterfaceNext(
    PMOS_INTERFACE     osInterface,
    CODECHAL_FUNCTION  codecFunction,
    MhwInterfacesNext  *mhwInterfacesNext,
    bool               disableScalability)
{
    CODEC_HW_FUNCTION_ENTER;

    m_avpItf   = mhwInterfacesNext->m_avpItf;
    m_vdencItf = mhwInterfacesNext->m_vdencItf;
    m_hucItf   = mhwInterfacesNext->m_hucItf;
    m_miItf    = mhwInterfacesNext->m_miItf;
    m_hcpItf   = mhwInterfacesNext->m_hcpItf;

    // Remove legacy mhw sub interfaces.
    m_cpInterface = mhwInterfacesNext->m_cpInterface;
    m_mfxInterface = mhwInterfacesNext->m_mfxInterface;
    m_vdencInterface = mhwInterfacesNext->m_vdencInterface;
    m_hcpInterface = mhwInterfacesNext->m_hcpInterface;
}

MOS_STATUS CodechalHwInterfaceNext::GetAvpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    CODEC_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t    avpCommandsSize = 0;
    uint32_t    avpPatchListSize = 0;
    uint32_t    cpCmdsize        = 0;
    uint32_t    cpPatchListSize  = 0;

    if (m_avpItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_avpItf->GetAvpStateCmdSize(
            (uint32_t *)&avpCommandsSize,
            (uint32_t *)&avpPatchListSize,
            params));
    }

    if (m_cpInterface != nullptr)
    {
        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize = avpCommandsSize + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceNext::GetAvpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize)
{
    CODEC_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    if (mode == CODECHAL_DECODE_MODE_AV1VLD)
    {
        stateCmdSizeParams.bDecodeInUse = true;
    }

    uint32_t avpCommandsSize = 0;
    uint32_t avpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_avpItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_avpItf->GetAvpPrimitiveCmdSize(
            (uint32_t*)&avpCommandsSize,
            (uint32_t*)&avpPatchListSize,
            &stateCmdSizeParams));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize  = avpCommandsSize  + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceNext::SetCacheabilitySettings(
    MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    // Next step: replace with new mhw sub interfaces, including vdenc, mfx, hcp.
    /**********************************************************************/
    if (m_mfxInterface)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_mfxInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_hcpInterface)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_hcpInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_vdencInterface)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_vdencInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    /*                                                                    */

    /* New Mhw sub interfaces usage */
    if (m_avpItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_avpItf->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_hcpItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_hcpItf->SetCacheabilitySettings(cacheabilitySettings));
    }

    return eStatus;
}

    MOS_STATUS CodechalHwInterfaceNext::GetHucStateCommandSize(
               uint32_t mode,
               uint32_t* commandsSize,
               uint32_t* patchListSize,
               PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)

    {
        MHW_FUNCTION_ENTER;

        uint32_t maxSize = 0;
        uint32_t patchListMaxSize = 0;
        uint32_t standard = CodecHal_GetStandardFromMode(mode);
        uint32_t numSlices = 1;
        uint32_t numStoreDataImm = 1;
        uint32_t numStoreReg = 1;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);
        MHW_MI_CHK_NULL(params);

        if(params->uNumStoreDataImm)
        {
            numStoreDataImm = params->uNumStoreDataImm;
        }
        if(params->uNumStoreReg)
        {
            numStoreReg = params->uNumStoreReg;
        }

        if (mode == CODECHAL_DECODE_MODE_HEVCVLD && params->bShortFormat)
        {
            numSlices       = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6;
            numStoreDataImm = 2;
            numStoreReg     = 2;

            maxSize +=
                2 * m_miItf->MHW_GETSIZE_F(MI_CONDITIONAL_BATCH_BUFFER_END)();

            patchListMaxSize +=
                2 * PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }
        else if (standard == CODECHAL_CENC)
        {
            numStoreDataImm = 3;
            numStoreReg     = 3;

            maxSize +=
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() * 2 +
                m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2;

        }
        else if (mode == CODECHAL_ENCODE_MODE_VP9)
        {
            // for huc status 2 register and semaphore signal and reset
            numStoreDataImm = 3;

            maxSize +=
                m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)() +
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);
        }
        else if (mode == CODECHAL_ENCODE_MODE_AVC)
        {
            numStoreDataImm = 2;
            numStoreReg     = 2;

            maxSize +=
                2 * m_miItf->MHW_GETSIZE_F(MI_CONDITIONAL_BATCH_BUFFER_END)();

            patchListMaxSize +=
                2 * PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }

        maxSize +=
            m_hucItf->MHW_GETSIZE_F(HUC_PIPE_MODE_SELECT)() +
            m_hucItf->MHW_GETSIZE_F(HUC_IMEM_STATE)() +
            m_hucItf->MHW_GETSIZE_F(HUC_DMEM_STATE)() +
            m_hucItf->MHW_GETSIZE_F(HUC_VIRTUAL_ADDR_STATE)() +
            m_hucItf->MHW_GETSIZE_F(HUC_IND_OBJ_BASE_ADDR_STATE)() +
            numSlices       * m_hucItf->MHW_GETSIZE_F(HUC_STREAM_OBJECT)() +
            numSlices       * m_hucItf->MHW_GETSIZE_F(HUC_START)() +
            numStoreDataImm * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +
            numStoreReg     * m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)();

        if(params->uNumMfxWait)
        {
            maxSize +=
                params->uNumMfxWait * m_miItf->MHW_GETSIZE_F(MFX_WAIT)();
        }

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

        if(params->uNumAddConBBEnd)
        {
            maxSize +=
                params->uNumAddConBBEnd * m_miItf->MHW_GETSIZE_F(MI_CONDITIONAL_BATCH_BUFFER_END)();

            patchListMaxSize +=
                params->uNumAddConBBEnd * PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }
        if(params->uNumMiCopy)
        {
            maxSize +=
                params->uNumMiCopy * m_miItf->MHW_GETSIZE_F(MI_COPY_MEM_MEM)();

            patchListMaxSize +=
                params->uNumMiCopy * PATCH_LIST_COMMAND(MI_COPY_MEM_MEM_CMD);
        }
        if(params->uNumMiFlush)
        {
            maxSize +=
                params->uNumMiFlush * m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

            patchListMaxSize +=
                params->uNumMiFlush * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);
        }

        if (params->bHucDummyStream || params->bPerformHucStreamOut)
        {
            uint32_t dummyTimes = params->bPerformHucStreamOut ? 2: 1;
            for (uint32_t i = 0; i < dummyTimes; i++)
            {
                maxSize +=
                    m_hucItf->MHW_GETSIZE_F(HUC_PIPE_MODE_SELECT)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_IMEM_STATE)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_DMEM_STATE)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_VIRTUAL_ADDR_STATE)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_IND_OBJ_BASE_ADDR_STATE)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_STREAM_OBJECT)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_START)() +
                    m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

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
            if (params->bPerformHucStreamOut)
            {
                maxSize +=
                    m_hucItf->MHW_GETSIZE_F(HUC_PIPE_MODE_SELECT)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_IND_OBJ_BASE_ADDR_STATE)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_STREAM_OBJECT)() +
                    4 * m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(HUC_PIPE_MODE_SELECT_CMD) +
                    PATCH_LIST_COMMAND(HUC_IND_OBJ_BASE_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(HUC_STREAM_OBJECT_CMD) +
                    4 * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);
            }
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }
