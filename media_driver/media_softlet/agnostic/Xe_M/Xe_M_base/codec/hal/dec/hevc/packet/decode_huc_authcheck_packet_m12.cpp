/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     decode_huc_authcheck_packet_m12.cpp
//! \brief    Defines the interface for HuC authentication check packet
//!

#include "decode_huc_authcheck_packet_m12.h"
#include "decode_resource_auto_lock.h"
#include "mhw_mmio_g12.h"
#include "mhw_mi_g12_X.h"

namespace decode
{
    MOS_STATUS DecodeHucAuthCheckPktM12::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_pipeline);

        m_allocator = m_pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        if (m_hucAuthCheckBuf == nullptr)
        {
            // Init buffer for storing Huc auth status MMIO
            m_hucAuthCheckBuf = m_allocator->AllocateBuffer(
                sizeof(uint64_t), "Huc authentication register store Buffer",
                resourceInternalReadWriteCache, lockableVideoMem, true, 0);
            DECODE_CHK_NULL(m_hucAuthCheckBuf);

            // add MMIO mask in DW0
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly  = 1;
            uint32_t *hucRegData = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &m_hucAuthCheckBuf->OsResource, &lockFlags);
            DECODE_CHK_NULL(hucRegData);
            *hucRegData = HUC_LOAD_INFO_REG_MASK_G12;
            m_osInterface->pfnUnlockResource(m_osInterface, &m_hucAuthCheckBuf->OsResource);
        }

        if (m_secondLevelBBArray == nullptr)
        {
            // 1 CL enough for huc auth check
            m_secondLevelBBArray = m_allocator->AllocateBatchBufferArray(
                CODECHAL_CACHELINE_SIZE, 1, CODECHAL_HEVC_NUM_DMEM_BUFFERS, true, lockableVideoMem);
            DECODE_CHK_NULL(m_secondLevelBBArray);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeHucAuthCheckPktM12::Destroy()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(m_allocator->Destroy(m_hucAuthCheckBuf));
        DECODE_CHK_STATUS(m_allocator->Destroy(m_secondLevelBBArray));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeHucAuthCheckPktM12::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // add media reset check 100ms, which equals to 1080p WDT threshold
        DECODE_CHK_STATUS(m_miInterface->SetWatchdogTimerThreshold(1920, 1080, true));
        DECODE_CHK_STATUS(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));
        DECODE_CHK_STATUS(m_miInterface->AddWatchdogTimerStartCmd(&cmdBuffer));

        // program 2nd level chained BB for Huc auth
        m_batchBuf = m_secondLevelBBArray->Fetch();
        if (m_batchBuf != nullptr)
        {
            ResourceAutoLock resLock(m_allocator, &m_batchBuf->OsResource);
            uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
            DECODE_CHK_NULL(batchBufBase);
            DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBuf, batchBufBase));
            m_hucAuthCmdBuffer.cmdBuf1stLvl = &cmdBuffer;
            DECODE_CHK_STATUS(PackHucAuthCmds(m_hucAuthCmdBuffer));
            if (!m_osInterface->pfnIsMismatchOrderProgrammingSupported())
            {
                DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&m_hucAuthCmdBuffer, nullptr));
            }
        }

        // BB start for 2nd level BB
        DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeHucAuthCheckPktM12::Init2ndLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase)
    {
        DECODE_FUNC_CALL();

        auto &cmdBuffer = m_hucAuthCmdBuffer;
        MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
        cmdBuffer.pCmdBase   = (uint32_t *)batchBufBase;
        cmdBuffer.pCmdPtr    = cmdBuffer.pCmdBase;
        cmdBuffer.iRemaining = batchBuffer.iSize;
        cmdBuffer.OsResource = batchBuffer.OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeHucAuthCheckPktM12::PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // Store Huc Auth register
        MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &m_hucAuthCheckBuf->OsResource;
        storeRegParams.dwOffset        = sizeof(uint32_t);
        storeRegParams.dwRegister      = m_hucInterface->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucLoadInfoOffset;
        DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // Check Huc auth: if equals to 0 continue chained BB until reset, otherwise send BB end cmd.
        uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADEQUALIDD;
        DECODE_CHK_STATUS(m_hwInterface->SendCondBbEndCmd(
            &m_hucAuthCheckBuf->OsResource, 0, 0, false, true, compareOperation, &cmdBuffer));

        // Chained BB loop
        DECODE_CHK_STATUS(static_cast<MhwMiInterfaceG12 *>(m_miInterface)->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf, true));

        return MOS_STATUS_SUCCESS;
    }
}
