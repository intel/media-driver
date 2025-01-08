/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     decode_nullhw_proxy_test_packet.cpp
//!

#include "decode_nullhw_proxy_test_packet.h"
#include "decode_resource_auto_lock.h"

namespace decode
{
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS DecodeNullHWProxyTestPkt::Init(DecodePipeline *pipeline, MOS_INTERFACE *osInterface)
    {
        MOS_STATUS status = MOS_STATUS_SUCCESS;
        DECODE_CHK_NULL(osInterface);
        PMOS_CONTEXT pOsContext = osInterface->pOsContext;
        DECODE_CHK_NULL(pOsContext);
        m_allocator = pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);
        MediaUserSettingSharedPtr userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
        ReadUserSetting(
            userSettingPtr,
            m_isNullHWEnabled,
            __MEDIA_USER_FEATURE_VALUE_NULLHW_ENABLE,
            MediaUserSetting::Group::Device);
        if (!m_isNullHWEnabled) //NullHW is Not Enabled, return
        {
            return MOS_STATUS_SUCCESS;
        }
        ReadUserSetting(
            userSettingPtr,
            m_repeatCount,
            __MEDIA_USER_FEATURE_VALUE_NULLHW_PROXY_REPEAT_COUNT,
            MediaUserSetting::Group::Device);

        if (m_dataStoreBuf == nullptr)
        {
            m_dataStoreBuf = m_allocator->AllocateBuffer(2 * sizeof(uint32_t), "Store data buffer", resourceInternalReadWriteCache, notLockableVideoMem, true, 0);
            DECODE_CHK_NULL(m_dataStoreBuf);
        }
        if (m_secondLevelBBArray == nullptr)
        {
            m_secondLevelBBArray = m_allocator->AllocateBatchBufferArray(
                CODECHAL_CACHELINE_SIZE, 1, 64, true, lockableVideoMem);
            DECODE_CHK_NULL(m_secondLevelBBArray);
        }
        m_miItf = pipeline->GetHwInterface()->GetMiInterfaceNext();
        DECODE_CHK_NULL(m_miItf);

        MOS_ZeroMemory(&m_conditionalBbEndDummy, sizeof(m_conditionalBbEndDummy));

        return status;
    }

    MOS_STATUS DecodeNullHWProxyTestPkt::Destory()
    {
        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(m_allocator->Destroy(m_dataStoreBuf));
        DECODE_CHK_STATUS(m_allocator->Destroy(m_secondLevelBBArray));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeNullHWProxyTestPkt::Init2ndLevelCmdBuffer(MHW_BATCH_BUFFER& batchBuffer, uint8_t* batchBufBase)
    {
        DECODE_FUNC_CALL();

        auto &cmdBuffer = m_statusCheckCmdBuffer;
        MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
        cmdBuffer.pCmdBase   = (uint32_t *)batchBufBase;
        cmdBuffer.pCmdPtr    = cmdBuffer.pCmdBase;
        cmdBuffer.iRemaining = batchBuffer.iSize;
        cmdBuffer.OsResource = batchBuffer.OsResource;

        return MOS_STATUS_SUCCESS;
    }
    
    //!
    //! \brief    Send conditional batch buffer end cmd
    //! \details  Send conditional batch buffer end cmd
    //!
    //! \param    [in] resource
    //!           Reource used in conditional batch buffer end cmd
    //! \param    [in] offset
    //!           Reource offset used in mi atomic dword cmd
    //! \param    [in] compData
    //!           Compare data
    //! \param    [in] disableCompMask
    //!           Indicate disabling compare mask
    //! \param    [in] enableEndCurrentBatchBuffLevel
    //!           End Current Batch Buffer Level
    //! \param    [in] compareOperation
    //!           Compare operation
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DecodeNullHWProxyTestPkt::SendCondBbEndCmd(
        PMOS_RESOURCE       resource,
        uint32_t            offset,
        uint32_t            compData,
        bool                disableCompMask,
        bool                enableEndCurrentBatchBuffLevel,
        uint32_t            compareOperation,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;
        DECODE_CHK_NULL(m_miItf);

        if (!Mos_ResourceIsNull(&m_conditionalBbEndDummy))
        {
            auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
            par                   = {};
            par.postSyncOperation = 1;
            par.pOsResource       = &m_conditionalBbEndDummy;
            par.dwDataDW1         = 0;
            CODEC_HW_CHK_STATUS_RETURN(m_miItf->ADDCMD_MI_FLUSH_DW(cmdBuffer));
        }

        auto &par               = m_miItf->GETPAR_MI_CONDITIONAL_BATCH_BUFFER_END();
        par                     = {};
        par.presSemaphoreBuffer = resource;
        par.dwOffset            = offset;
        par.dwValue             = compData;
        par.bDisableCompareMask = disableCompMask;
        par.dwParamsType        = mhw::mi::MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS;
        if (enableEndCurrentBatchBuffLevel)
        {
            par.enableEndCurrentBatchBuffLevel = enableEndCurrentBatchBuffLevel;
            par.compareOperation               = compareOperation;
        }
        eStatus = m_miItf->ADDCMD_MI_CONDITIONAL_BATCH_BUFFER_END(cmdBuffer);

        return eStatus;
    }

    MOS_STATUS DecodeNullHWProxyTestPkt::Pack2ndLevelCmds(DecodePipeline* pipeline, MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // MI_ATOMIC: ADD 1 TO ORIGINAL DATA
        auto &par            = m_miItf->GETPAR_MI_ATOMIC();
        par                  = {};
        par.pOsResource      = &m_dataStoreBuf->OsResource;
        par.dwResourceOffset = sizeof(uint32_t);
        par.dwDataSize       = sizeof(uint32_t);
        par.Operation        = mhw::mi::MHW_MI_ATOMIC_INC;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_ATOMIC(&cmdBuffer));

        // check if loop stops
        uint32_t compareOperation = 2;  //COMPARE_OPERATION_MADLESSTHANIDD;
        uint32_t compData = m_repeatCount;

        DECODE_CHK_STATUS(SendCondBbEndCmd(
            &m_dataStoreBuf->OsResource, 0, compData, false, true, compareOperation, &cmdBuffer));

        // Chained BB loop
        auto &chainedBBStartPar = m_miItf->GETPAR_MI_BATCH_BUFFER_START();
        MOS_ZeroMemory(&chainedBBStartPar, sizeof(mhw::mi::MI_BATCH_BUFFER_START_PAR));
        chainedBBStartPar.secondLevelBatchBuffer = false;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBuf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeNullHWProxyTestPkt::AddNullHwProxyCmd(DecodePipeline *pipeline, MOS_INTERFACE *osInterface, PMOS_COMMAND_BUFFER cmdBuffer)
    {
        DECODE_FUNC_CALL();

        if (!m_isNullHWEnabled || m_repeatCount == 0)
        {
            return MOS_STATUS_SUCCESS;
        }

        DECODE_CHK_NULL(cmdBuffer);
        DECODE_CHK_NULL(pipeline);
        DECODE_CHK_NULL(osInterface);
        DECODE_CHK_NULL(m_miItf);

        // add 2nd level BB before start status reporting
        auto &par            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        par                  = {};
        par.pOsResource      = &m_dataStoreBuf->OsResource;
        par.dwResourceOffset = 0;
        par.dwValue          = 0xFFFFFF;

        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // clear the data store buffer each time
        par.dwResourceOffset = sizeof(uint32_t);
        par.dwValue          = 0;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // prepare 2nd level chained BB
        m_batchBuf = m_secondLevelBBArray->Fetch();
        if (m_batchBuf != nullptr)
        {
            ResourceAutoLock resLock(m_allocator, &m_batchBuf->OsResource);
            uint8_t         *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
            DECODE_CHK_NULL(batchBufBase);
            DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBuf, batchBufBase));
            m_statusCheckCmdBuffer.cmdBuf1stLvl = cmdBuffer;
            DECODE_CHK_STATUS(Pack2ndLevelCmds(pipeline, m_statusCheckCmdBuffer));
            if (!osInterface->pfnIsMismatchOrderProgrammingSupported())
            {
                DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_statusCheckCmdBuffer, nullptr));
            }
        }

        auto &BBStartPar = m_miItf->GETPAR_MI_BATCH_BUFFER_START();
        MOS_ZeroMemory(&BBStartPar, sizeof(mhw::mi::MI_BATCH_BUFFER_START_PAR));
        BBStartPar.secondLevelBatchBuffer = true;  // set chained BB mode
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(cmdBuffer, m_batchBuf));

        return MOS_STATUS_SUCCESS;
    }
#endif
}
