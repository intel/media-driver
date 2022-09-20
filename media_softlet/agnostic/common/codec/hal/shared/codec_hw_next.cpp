/*
* Copyright (c) 2021-2022, Intel Corporation
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
    m_mfxItf   = mhwInterfacesNext->m_mfxItf;
    m_renderItf = mhwInterfacesNext->m_renderItf;

    CODEC_HW_ASSERT(osInterface);
    m_osInterface = osInterface;

    m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable  = m_osInterface->pfnGetWaTable(m_osInterface);
    CODEC_HW_ASSERT(m_skuTable);
    CODEC_HW_ASSERT(m_waTable);

    MOS_ZeroMemory(&m_hucDmemDummy, sizeof(m_hucDmemDummy));
    MOS_ZeroMemory(&m_dummyStreamIn, sizeof(m_dummyStreamIn));
    MOS_ZeroMemory(&m_dummyStreamOut, sizeof(m_dummyStreamOut));

    // Remove legacy mhw sub interfaces.
    m_cpInterface = mhwInterfacesNext->m_cpInterface;
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
    if (m_vdencItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_vdencItf->SetCacheabilitySettings(cacheabilitySettings));
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
    if (m_mfxItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_mfxItf->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_vdencItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_vdencItf->SetCacheabilitySettings(cacheabilitySettings));
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
                2 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }
        else if (standard == CODECHAL_CENC)
        {
            numStoreDataImm = 3;
            numStoreReg     = 3;

            maxSize +=
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() * 2 +
                m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD) * 2;

        }
        else if (mode == CODECHAL_ENCODE_MODE_VP9)
        {
            // for huc status 2 register and semaphore signal and reset
            numStoreDataImm = 3;

            maxSize +=
                m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)() +
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD);
        }
        else if (mode == CODECHAL_ENCODE_MODE_AVC)
        {
            numStoreDataImm = 2;
            numStoreReg     = 2;

            maxSize +=
                2 * m_miItf->MHW_GETSIZE_F(MI_CONDITIONAL_BATCH_BUFFER_END)();

            patchListMaxSize +=
                2 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
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
            PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_IMEM_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_DMEM_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_VIRTUAL_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_IND_OBJ_BASE_ADDR_STATE_CMD) +
            numSlices       * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_STREAM_OBJECT_CMD) +
            numSlices       * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_START_CMD) +
            numStoreDataImm * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_DATA_IMM_CMD) +
            numStoreReg     * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_REGISTER_MEM_CMD);

        if(params->uNumAddConBBEnd)
        {
            maxSize +=
                params->uNumAddConBBEnd * m_miItf->MHW_GETSIZE_F(MI_CONDITIONAL_BATCH_BUFFER_END)();

            patchListMaxSize +=
                params->uNumAddConBBEnd * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD);
        }
        if(params->uNumMiCopy)
        {
            maxSize +=
                params->uNumMiCopy * m_miItf->MHW_GETSIZE_F(MI_COPY_MEM_MEM)();

            patchListMaxSize +=
                params->uNumMiCopy * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_COPY_MEM_MEM_CMD);
        }
        if(params->uNumMiFlush)
        {
            maxSize +=
                params->uNumMiFlush * m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

            patchListMaxSize +=
                params->uNumMiFlush * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD);
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
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_PIPE_MODE_SELECT_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_IMEM_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_DMEM_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_VIRTUAL_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_IND_OBJ_BASE_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_STREAM_OBJECT_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_START_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD);
            }
            if (params->bPerformHucStreamOut)
            {
                maxSize +=
                    m_hucItf->MHW_GETSIZE_F(HUC_PIPE_MODE_SELECT)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_IND_OBJ_BASE_ADDR_STATE)() +
                    m_hucItf->MHW_GETSIZE_F(HUC_STREAM_OBJECT)() +
                    4 * m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_PIPE_MODE_SELECT_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_IND_OBJ_BASE_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::HUC_STREAM_OBJECT_CMD) +
                    4 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD);
            }
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

MOS_STATUS CodechalHwInterfaceNext::GetMfxPrimitiveCommandsDataSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;

    if (m_mfxItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_mfxItf->GetMfxPrimitiveCommandsDataSize(
            mode, (uint32_t*)commandsSize, (uint32_t*)patchListSize, modeSpecific ? true : false));

        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize += (uint32_t)cpCmdsize;
    *patchListSize += (uint32_t)cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::GetMfxStateCommandsDataSize(
    uint32_t  mode,
    uint32_t *commandsSize,
    uint32_t *patchListSize,
    bool      shortFormat)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    uint32_t cpCmdsize       = 0;
    uint32_t cpPatchListSize = 0;

    if (m_mfxItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_mfxItf->GetMfxStateCommandsDataSize(
            mode, (uint32_t *)commandsSize, (uint32_t *)patchListSize, shortFormat ? true : false));

        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }
    *commandsSize += (uint32_t)cpCmdsize;
    *patchListSize += (uint32_t)cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::GetHcpPrimitiveCommandSize(
    uint32_t  mode,
    uint32_t *commandsSize,
    uint32_t *patchListSize,
    bool      modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize  = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hcpItf && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODEC_HW_CHK_STATUS_RETURN(m_hcpItf->GetHcpPrimitiveCommandSize(
            mode, &hcpCommandsSize, &hcpPatchListSize, modeSpecific ? true : false));

        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hcpCommandsSize + cpCmdsize;
    *patchListSize = hcpPatchListSize + cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::AddHucDummyStreamOut(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable))
    {
        return eStatus;
    }

    CODEC_HW_FUNCTION_ENTER;

    CODEC_HW_CHK_NULL_RETURN(cmdBuffer);
    CODEC_HW_CHK_NULL_RETURN(m_miItf);

    if (Mos_ResourceIsNull(&m_dummyStreamOut))
    {
        MOS_LOCK_PARAMS         lockFlags;
        uint8_t*                data;
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        m_dmemBufSize = MHW_CACHELINE_SIZE;

        allocParamsForBufferLinear.dwBytes = m_dmemBufSize;
        allocParamsForBufferLinear.pBufName = "HucDmemBufferDummy";
        CODEC_HW_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_hucDmemDummy));
        // set lock flag to WRITE_ONLY
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        data =
            (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_hucDmemDummy, &lockFlags);
        CODEC_HW_CHK_NULL_RETURN(data);
        MOS_ZeroMemory(data, m_dmemBufSize);
        *data = 8;
        m_osInterface->pfnUnlockResource(m_osInterface, &m_hucDmemDummy);

        allocParamsForBufferLinear.dwBytes = CODECHAL_CACHELINE_SIZE;

        allocParamsForBufferLinear.pBufName = "HucDummyStreamInBuffer";
        CODEC_HW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_dummyStreamIn));
        allocParamsForBufferLinear.pBufName = "HucDummyStreamOutBuffer";
        CODEC_HW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_dummyStreamOut));
    }

    auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    // pipe mode select
    auto &pipeModeSelectParams = m_hucItf->MHW_GETPAR_F(HUC_PIPE_MODE_SELECT)();
    pipeModeSelectParams       = {};

    pipeModeSelectParams.mediaSoftResetCounterValue = 2400;

    // pass bit-stream buffer by Ind Obj Addr command. Set size to 1 for dummy stream
    auto &indObjParams                 = m_hucItf->MHW_GETPAR_F(HUC_IND_OBJ_BASE_ADDR_STATE)();
    indObjParams                       = {};
    indObjParams.DataBuffer            = &m_dummyStreamIn;
    indObjParams.DataSize              = 1;
    indObjParams.StreamOutObjectBuffer = &m_dummyStreamOut;
    indObjParams.StreamOutObjectSize   = 1;

    // set stream object with stream out enabled
    auto &streamObjParams                         = m_hucItf->MHW_GETPAR_F(HUC_STREAM_OBJECT)();
    streamObjParams                               = {};
    streamObjParams.IndirectStreamInDataLength    = 1;
    streamObjParams.IndirectStreamInStartAddress  = 0;
    streamObjParams.HucProcessing                 = true;
    streamObjParams.IndirectStreamOutStartAddress = 0;
    streamObjParams.StreamOut                     = 1;

    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    auto &imemParams            = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
    imemParams                  = {};
    imemParams.kernelDescriptor = VDBOX_HUC_VDENC_BRC_INIT_KERNEL_DESCRIPTOR;

    // set HuC DMEM param
    auto &dmemParams             = m_hucItf->MHW_GETPAR_F(HUC_DMEM_STATE)();
    dmemParams                   = {};
    dmemParams.hucDataSource = &m_hucDmemDummy;
    dmemParams.dataLength    = m_dmemBufSize;
    dmemParams.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    auto &virtualAddrParams                      = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();
    virtualAddrParams                            = {};
    virtualAddrParams.regionParams[0].presRegion = &m_dummyStreamOut;

    streamObjParams.HucProcessing      = true;
    streamObjParams.HucBitstreamEnable = 1;

    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_PIPE_MODE_SELECT)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_DMEM_STATE)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_VIRTUAL_ADDR_STATE)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_IND_OBJ_BASE_ADDR_STATE)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_STREAM_OBJECT)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_START)(cmdBuffer));

    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::PerformHucStreamOut(
    CodechalHucStreamoutParams *hucStreamOutParams,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;
    CODEC_HW_CHK_NULL_RETURN(cmdBuffer);

    if (MEDIA_IS_SKU(m_skuTable, FtrEnableMediaKernels) && MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable))
    {
        CODEC_HW_CHK_STATUS_RETURN(AddHucDummyStreamOut(cmdBuffer));
    }

    // pipe mode select
    auto &pipeModeSelectParams                      = m_hucItf->MHW_GETPAR_F(HUC_PIPE_MODE_SELECT)();
    pipeModeSelectParams                            = {};
    pipeModeSelectParams.mode = hucStreamOutParams->mode;
    pipeModeSelectParams.mediaSoftResetCounterValue = 2400;
    pipeModeSelectParams.streamOutEnabled = true;
    if (hucStreamOutParams->segmentInfo == nullptr && m_osInterface->osCpInterface->IsCpEnabled())
    {
        // Disable protection control setting in huc drm
        pipeModeSelectParams.disableProtectionSetting = true;
    }

    // Enlarge the stream in/out data size to avoid upper bound hit assert in HuC
    hucStreamOutParams->dataSize += hucStreamOutParams->inputRelativeOffset;
    hucStreamOutParams->streamOutObjectSize += hucStreamOutParams->outputRelativeOffset;

    // pass bit-stream buffer by Ind Obj Addr command
    auto &indObjParams                 = m_hucItf->MHW_GETPAR_F(HUC_IND_OBJ_BASE_ADDR_STATE)();
    indObjParams                       = {};
    indObjParams.DataBuffer            = hucStreamOutParams->dataBuffer;
    indObjParams.DataSize              = MOS_ALIGN_CEIL(hucStreamOutParams->dataSize, MHW_PAGE_SIZE);
    indObjParams.DataOffset            = hucStreamOutParams->dataOffset;
    indObjParams.StreamOutObjectBuffer = hucStreamOutParams->streamOutObjectBuffer;
    indObjParams.StreamOutObjectSize   = MOS_ALIGN_CEIL(hucStreamOutParams->streamOutObjectSize, MHW_PAGE_SIZE);
    indObjParams.StreamOutObjectOffset = hucStreamOutParams->streamOutObjectOffset;

    // set stream object with stream out enabled
    auto &streamObjParams                         = m_hucItf->MHW_GETPAR_F(HUC_STREAM_OBJECT)();
    streamObjParams                               = {};
    streamObjParams.IndirectStreamInDataLength    = hucStreamOutParams->indStreamInLength;
    streamObjParams.IndirectStreamInStartAddress  = hucStreamOutParams->inputRelativeOffset;
    streamObjParams.IndirectStreamOutStartAddress = hucStreamOutParams->outputRelativeOffset;
    streamObjParams.HucProcessing                 = true;
    streamObjParams.HucBitstreamEnable            = true;
    streamObjParams.StreamOut                     = true;

    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_PIPE_MODE_SELECT)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_IND_OBJ_BASE_ADDR_STATE)(cmdBuffer));
    CODEC_HW_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_STREAM_OBJECT)(cmdBuffer));

    // This flag is always false if huc fw is not loaded.
    if (MEDIA_IS_SKU(m_skuTable, FtrEnableMediaKernels) &&
        MEDIA_IS_WA(m_waTable, WaHucStreamoutOnlyDisable))
    {
        CODEC_HW_CHK_STATUS_RETURN(AddHucDummyStreamOut(cmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::ReadHcpStatus(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const EncodeStatusReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;;

    CODEC_HW_CHK_NULL_RETURN(cmdBuffer);

    CODEC_HW_CHK_COND_RETURN((vdboxIndex > GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    auto &par1           = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    par1                 = {};
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    auto mmioRegisters = m_hcpItf->GetMmioRegisters(vdboxIndex);

    auto &par2           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    par2                 = {};
    par2.presStoreBuffer = params.resBitstreamByteCountPerFrame;
    par2.dwOffset        = params.bitstreamByteCountPerFrameOffset;
    par2.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    auto &par3           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    par3                 = {};
    par3.presStoreBuffer = params.resBitstreamSyntaxElementOnlyBitCount;
    par3.dwOffset        = params.bitstreamSyntaxElementOnlyBitCountOffset;
    par3.dwRegister      = mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset;
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    auto &par4           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    par4                 = {};
    par4.presStoreBuffer = params.resQpStatusCount;
    par4.dwOffset        = params.qpStatusCountOffset;
    par4.dwRegister      = mmioRegisters->hcpEncQpStatusCountRegOffset;
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::ReadImageStatusForHcp(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const EncodeStatusReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    CODEC_HW_CHK_NULL_RETURN(cmdBuffer);

    CODEC_HW_CHK_COND_RETURN((vdboxIndex > GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    auto mmioRegisters = m_hcpItf->GetMmioRegisters(vdboxIndex);

    auto &par1           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    par1                 = {};
    par1.presStoreBuffer = params.resImageStatusMask;
    par1.dwOffset        = params.imageStatusMaskOffset;
    par1.dwRegister      = mmioRegisters->hcpEncImageStatusMaskRegOffset;
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    auto &par2           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    par2                 = {};
    par2.presStoreBuffer = params.resImageStatusCtrl;
    par2.dwOffset        = params.imageStatusCtrlOffset;
    par2.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

    auto &par3           = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    par3                 = {};
    CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::SetRowstoreCachingOffsets(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    if (m_vdencItf)
    {
        mhw::vdbox::vdenc::RowStorePar par = {};
        if (rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            par.mode    = mhw::vdbox::vdenc::RowStorePar::AVC;
            par.isField = !rowstoreParams->bIsFrame;
        }
        else if (rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            par.mode = mhw::vdbox::vdenc::RowStorePar::VP9;
            par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_8;
            if (rowstoreParams->ucBitDepthMinus8 == 1 || rowstoreParams->ucBitDepthMinus8 == 2)
            {
                par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_10;
            }
            else if (rowstoreParams->ucBitDepthMinus8 > 2)
            {
                par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_12;
            }
            par.frameWidth = rowstoreParams->dwPicWidth;
            switch (rowstoreParams->ucChromaFormat)
            {
            case HCP_CHROMA_FORMAT_MONOCHROME:
                par.format = mhw ::vdbox::vdenc::RowStorePar::MONOCHROME;
                break;
            case HCP_CHROMA_FORMAT_YUV420:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV420;
                break;
            case HCP_CHROMA_FORMAT_YUV422:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV422;
                break;
            case HCP_CHROMA_FORMAT_YUV444:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV444;
                break;
            }
        }
        else if (rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            par.mode = mhw::vdbox::vdenc::RowStorePar::HEVC;
            par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_8;
            if (rowstoreParams->ucBitDepthMinus8 == 1 || rowstoreParams->ucBitDepthMinus8 == 2)
            {
                par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_10;
            }
            else if (rowstoreParams->ucBitDepthMinus8 > 2)
            {
                par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_12;
            }
            par.lcuSize = mhw ::vdbox::vdenc::RowStorePar::SIZE_OTHER;
            if (rowstoreParams->ucLCUSize == 32)
            {
                par.lcuSize = mhw ::vdbox::vdenc::RowStorePar::SIZE_32;
            }
            else if (rowstoreParams->ucLCUSize == 64)
            {
                par.lcuSize = mhw ::vdbox::vdenc::RowStorePar::SIZE_64;
            }
            par.frameWidth = rowstoreParams->dwPicWidth;
            switch (rowstoreParams->ucChromaFormat)
            {
            case HCP_CHROMA_FORMAT_MONOCHROME:
                par.format = mhw ::vdbox::vdenc::RowStorePar::MONOCHROME;
                break;
            case HCP_CHROMA_FORMAT_YUV420:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV420;
                break;
            case HCP_CHROMA_FORMAT_YUV422:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV422;
                break;
            case HCP_CHROMA_FORMAT_YUV444:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV444;
                break;
            }
        }
        else if (rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AV1)
        {
            par.mode = mhw::vdbox::vdenc::RowStorePar::AV1;
        }
        CODEC_HW_CHK_STATUS_RETURN(m_vdencItf->SetRowstoreCachingOffsets(par));
    }
    if (m_mfxItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_mfxItf->GetRowstoreCachingAddrs(rowstoreParams));
    }
    if (m_hcpItf)
    {
        mhw::vdbox::hcp::HcpVdboxRowStorePar rowstoreParamsHCP = {};
        rowstoreParamsHCP.Mode             = rowstoreParams->Mode;
        rowstoreParamsHCP.dwPicWidth       = rowstoreParams->dwPicWidth;
        rowstoreParamsHCP.ucChromaFormat   = rowstoreParams->ucChromaFormat;
        rowstoreParamsHCP.ucBitDepthMinus8 = rowstoreParams->ucBitDepthMinus8;
        rowstoreParamsHCP.ucLCUSize        = rowstoreParams->ucLCUSize;
        CODEC_HW_CHK_STATUS_RETURN(m_hcpItf->SetRowstoreCachingOffsets(rowstoreParamsHCP));
    }
    if (m_avpItf)
    {
        CODEC_HW_CHK_STATUS_RETURN(m_avpItf->GetRowstoreCachingAddrs());
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceNext::InitL3CacheSettings()
{
    CODEC_HW_FUNCTION_ENTER;

    // Get default L3 cache settings
    CODEC_HW_CHK_STATUS_RETURN(m_renderItf->EnableL3Caching(nullptr));

#if (_DEBUG || _RELEASE_INTERNAL)
    // Override default L3 cache settings
    auto l3CacheConfig =
        m_renderItf->GetL3CacheConfig();
    mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS l3Overrides = {};
    l3Overrides.dwTcCntlReg =
        static_cast<mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG *>(l3CacheConfig)->dwL3CacheTcCntlReg_Setting;
    l3Overrides.dwAllocReg =
        static_cast<mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG *>(l3CacheConfig)->dwL3CacheAllocReg_Setting;
    CODEC_HW_CHK_STATUS_RETURN(InitL3ControlUserFeatureSettings(
        l3CacheConfig,
        &l3Overrides));
    CODEC_HW_CHK_STATUS_RETURN(m_renderItf->EnableL3Caching(
        &l3Overrides));
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodechalHwInterfaceNext::InitL3ControlUserFeatureSettings(
    mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG   *l3CacheConfig,
    mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *l3Overrides)
{
    CODEC_HW_FUNCTION_ENTER;

    CODEC_HW_CHK_NULL_RETURN(l3CacheConfig);
    CODEC_HW_CHK_NULL_RETURN(l3Overrides);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data     = l3CacheConfig->dwL3CacheCntlReg_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG_OVERRIDE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    l3Overrides->dwCntlReg = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data     = l3CacheConfig->dwL3CacheCntlReg2_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG2_OVERRIDE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    l3Overrides->dwCntlReg2 = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data     = l3CacheConfig->dwL3CacheCntlReg3_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG3_OVERRIDE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    l3Overrides->dwCntlReg3 = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data     = l3CacheConfig->dwL3CacheSqcReg1_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG1_OVERRIDE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    l3Overrides->dwSqcReg1 = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data     = l3CacheConfig->dwL3CacheSqcReg4_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG4_OVERRIDE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    l3Overrides->dwSqcReg4 = (uint32_t)userFeatureData.i32Data;

    if (l3CacheConfig->bL3LRA1Reset)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.u32Data     = l3CacheConfig->dwL3LRA1Reg_Setting;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_LRA_1_REG1_OVERRIDE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
        l3Overrides->dwLra1Reg = (uint32_t)userFeatureData.i32Data;
    }

    return MOS_STATUS_SUCCESS;
}
#endif  // _DEBUG || _RELEASE_INTERNAL
