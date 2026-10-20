/*
* Copyright (c) 2025, Intel Corporation
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
#ifndef __LEVELZERO_INTERFACE_H__
#define __LEVELZERO_INTERFACE_H__
#include "levelzero_defs.h"
#include "mos_os.h"

#define L0_PUBLIC_CHK_ZE_STATUS_RETURN(_stmt)                             \
    {                                                                     \
        if (_stmt != ZE_RESULT_SUCCESS)                                   \
        {                                                                 \
            MOS_OS_ASSERTMESSAGE("Level Zero Error %x,", _stmt);          \
            MOS_OS_CHK_STATUS_RETURN(L0Interface::ConvertZeResult(_stmt)) \
        }                                                                 \
    }


#define L0_PUBLIC_CHK_ZE_STATUS(_stmt)                           \
    {                                                            \
        if (_stmt != ZE_RESULT_SUCCESS)                          \
        {                                                        \
            MOS_OS_ASSERTMESSAGE("Level Zero Error %x,", _stmt); \
        }                                                        \
    }

class L0Interface
{
public:
    L0Interface(PMOS_INTERFACE osInterface);
    virtual ~L0Interface();

    static MOS_STATUS Init(L0Interface *l0Interface);
    static MOS_STATUS CreateCmdList(L0Interface *l0Interface, ze_command_list_handle_t &cmdList);
    static MOS_STATUS DestroyCmdList(L0Interface *l0Interface, ze_command_list_handle_t &cmdList);
    static MOS_STATUS CreateFence(L0Interface *l0Interface, ze_fence_handle_t &fence);
    static MOS_STATUS DestroyFence(L0Interface *l0Interface, ze_fence_handle_t &fence);
    static MOS_STATUS WaitFence(L0Interface *l0Interface, ze_fence_handle_t &fence);
    static MOS_STATUS AllocateHostMem(L0Interface *l0Interface, size_t size, void *&ptr);
    static MOS_STATUS FreeHostMem(L0Interface *l0Interface, void *&ptr);
    static MOS_STATUS ExecuteSingle(L0Interface *l0Interface, ze_command_list_handle_t &cmdList, ze_fence_handle_t &fence);
    static MOS_STATUS Execute(L0Interface *l0Interface, std::vector<ze_command_list_handle_t> &cmdLists, ze_fence_handle_t &fence);

    MOS_STATUS (*pfnInit) (L0Interface *l0Interface) = Init;
    MOS_STATUS (*pfnCreateCmdList) (L0Interface *l0Interface, ze_command_list_handle_t &cmdList) = CreateCmdList;
    MOS_STATUS (*pfnDestroyCmdList) (L0Interface *l0Interface, ze_command_list_handle_t &cmdList) = DestroyCmdList;
    MOS_STATUS (*pfnCreateFence) (L0Interface *l0Interface, ze_fence_handle_t &fence) = CreateFence;
    MOS_STATUS (*pfnDestroyFence) (L0Interface *l0Interface, ze_fence_handle_t &fence) = DestroyFence;
    MOS_STATUS (*pfnWaitFence) (L0Interface *l0Interface, ze_fence_handle_t &fence) = WaitFence;
    MOS_STATUS (*pfnAllocateHostMem) (L0Interface *l0Interface, size_t size, void *&ptr) = AllocateHostMem;
    MOS_STATUS (*pfnFreeHostMem) (L0Interface *l0Interface, void *&ptr) = FreeHostMem;
    MOS_STATUS (*pfnExecuteSingle) (L0Interface *l0Interface, ze_command_list_handle_t &cmdList, ze_fence_handle_t &fence) = ExecuteSingle;
    MOS_STATUS (*pfnExecute) (L0Interface *l0Interface, std::vector<ze_command_list_handle_t> &cmdLists, ze_fence_handle_t &fence) = Execute;
    

    static MOS_STATUS ConvertZeResult(ze_result_t result);

    virtual bool Supported()
    {
        return m_supported;
    }
    virtual bool Initialized()
    {
        return m_initialized;
    }

protected:
    virtual MOS_STATUS Destroy();
    virtual MOS_STATUS InitZeFunctions();
    virtual MOS_STATUS GetDriver() = 0;

protected:
    uint32_t                  m_zeDdiVersion    = 0;
    ZE_LOADER_FUNCTIONS       m_zeFunc          = {};
    ze_driver_handle_t        m_zeDriver        = nullptr;
    ze_device_handle_t        m_zeDevice        = nullptr;
    ze_context_handle_t       m_zeContext       = nullptr;
    ze_command_queue_handle_t m_zeCmdQueue      = nullptr;

    std::atomic<uint32_t> m_ordinal;

    PMOS_INTERFACE m_osInterface = nullptr;

    bool m_dllLoaded   = false;
    bool m_supported   = false;
    bool m_initialized = false;

MEDIA_CLASS_DEFINE_END(mhw__LevelZeroInterface)
};

#endif
