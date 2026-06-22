/*
* Copyright (c) 2019-2026, Intel Corporation
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
//! \file     mos_context_next.h
//! \brief    Container for parameters shared across different GPU contextNexts of the same device instance
//!

#ifndef __MOS_CONTEXT_NEXT_H__
#define __MOS_CONTEXT_NEXT_H__

#include "mos_os.h"
#include "mos_gpucontextmgr_next.h"
#if !EMUL
#include "mos_cmdbufmgr_next.h"
#include "mos_decompression.h"
#endif
#include "mos_mediacopy.h"
#if (_DEBUG || _RELEASE_INTERNAL)
#include "mos_bypass_hw_defs.h"
#include <mutex>
#endif

class MosOcaRTLogMgr;
class MosMockAdaptor;

class OsContextNext
{
protected:
    //!
    //! \brief Constructor for the OsContextNext
    //!
    OsContextNext(){};

    //!
    //! \brief    Interface for initializing NULL Hardware.
    //! \details  Interface for initializing NULL Hardware.
    //! \param    [in] osContext
    //!           Pointer to OS context.
    //! \param    [in] osDeviceContext
    //!           Pointer to OS device context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS NullHwInit(MOS_CONTEXT_HANDLE osContext);

    //!
    //! \brief    Destroy NULL Hardware.
    //! \details  Destroy NULL Hardware.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS NullHwDestroy();

public:
    //!
    //! \brief Destructor for the OsContextNext
    //!
    virtual ~OsContextNext(){};

    //!
    //! \brief  Initialzie the OS ContextNext Object
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    virtual MOS_STATUS Init(DDI_DEVICE_CONTEXT osDriverContextNext) = 0;

    //!
    //! \brief  Static entrypoint, get the OS ContextNext Object
    //! \return the os specific object for OS contextNext
    //!
    static class OsContextNext *GetOsContextObject();

    //!
    //! \brief  Clean up the smartptr
    //!
    void CleanUp();

    //!
    //! \brief  Get the skuTable
    //! \return The active SKU Table
    //!
    MEDIA_FEATURE_TABLE *GetSkuTable() { return &m_skuTable; };

    //!
    //! \brief  Get the waTable
    //! \return The active WA table
    //!
    MEDIA_WA_TABLE *GetWaTable() { return &m_waTable; };

    //!
    //! \brief  Check if deferred GPU context destroy is supported
    //! \return true if supported, false otherwise
    //!
    virtual bool IsDeferredGpuContextDestroySupported() { return false; }

    //!
    //! \brief  Get the platform information string
    //! \return value of m_platformInfo
    //!
    PLATFORM *GetPlatformInfo() { return &m_platformInfo; };

    //!
    //! \brief  Get the gtSystemInfo string
    //! \return value of m_gtSystemInfo
    //!
    MEDIA_SYSTEM_INFO *GetGtSysInfo() { return &m_gtSystemInfo; };

    //!
    //! \brief  Check the platform is Atom or not
    //! \return true on Atom platform, false on none Atom platform
    //!
    bool IsAtomSoc() { return m_isAtomSOC; };

    //!
    //! \brief  Set the OS ContextNext valid flag
    //! \param   [in] isOsContextNextValid
    //!          Flag to indicate if the os contextNext is valid. 
    //!
    void SetOsContextValid(bool isOsContextValid) { m_osContextValid = isOsContextValid; };

    //!
    //! \brief  Return the OS ContextNext valid flag
    //! \return true if the OS contextNext is valid, false if not valid
    //!
    bool GetOsContextValid() { return m_osContextValid; };

    //!
    //! \brief  Set Null Hw enabled flag
    //! \param  [in] isNullHwEnabled
    //!         Flag to indicate if Null Hw is enabled. 
    //!
    void SetNullHwIsEnabled(bool isNullHwEnabled) { m_nullHwIsEnabled = isNullHwEnabled; }

    //!
    //! \brief  Return Null Hw enabled flag
    //! \return true if Null Hw is enabled, false if not enabled
    //!
    bool GetNullHwIsEnabled() { return m_nullHwIsEnabled; }

    //!
    //! \brief  Set Vebox scalability enabled state reported flag
    //! \param  [in] isReported
    //!         Flag to indicate if Vebox scalability enabled state has been reported
    //!
    void SetEnableVeboxScalabilityReported(bool isReported) { m_veboxScalabilityEnableReported = isReported; }

    //!
    //! \brief  Return Vebox scalability enabled state reported flag
    //! \return true if Vebox scalability enabled state has been reported, false otherwise
    //!
    bool GetEnableVeboxScalabilityReported() { return m_veboxScalabilityEnableReported; }

    //!
    //! \brief  Set Vebox scalability disabled state reported flag
    //! \param  [in] isReported
    //!         Flag to indicate if Vebox scalability disabled state has been reported
    //!
    void SetDisableVeboxScalabilityReported(bool isReported) { m_veboxScalabilityDisableReported = isReported; }

    //!
    //! \brief  Return Vebox scalability disabled state reported flag
    //! \return true if Vebox scalability disabled state has been reported, false otherwise
    //!
    bool GetDisableVeboxScalabilityReported() { return m_veboxScalabilityDisableReported; }

    //!
    //! \brief  Set Vdbox scalability enabled state reported flag
    //! \param  [in] isReported
    //!         Flag to indicate if Vdbox scalability enabled state has been reported
    //!
    void SetEnableVdboxScalabilityReported(bool isReported) { m_vdboxScalabilityEnableReported = isReported; }

    //!
    //! \brief  Return Vdbox scalability enabled state reported flag
    //! \return true if Vdbox scalability enabled state has been reported, false otherwise
    //!
    bool GetEnableVdboxScalabilityReported() { return m_vdboxScalabilityEnableReported; }

    //!
    //! \brief  Set Vdbox scalability disabled state reported flag
    //! \param  [in] isReported
    //!         Flag to indicate if Vdbox scalability disabled state has been reported
    //!
    void SetDisableVdboxScalabilityReported(bool isReported) { m_vdboxScalabilityDisableReported = isReported; }

    //!
    //! \brief  Return Vdbox scalability disabled state reported flag
    //! \return true if Vdbox scalability disabled state has been reported, false otherwise
    //!
    bool GetDisableVdboxScalabilityReported() { return m_vdboxScalabilityDisableReported; }

    //!
    //! \brief  Get GPU context manager of the device
    //! \return GPU context manager
    //!
    GpuContextMgrNext *GetGpuContextMgr() { return m_gpuContextMgr; }

    //!
    //! \brief  Get Cmd buffer manager of the device
    //! \return Cmd buffer manager
    //!
    CmdBufMgrNext *GetCmdBufferMgr() { return m_cmdBufMgr; }

    //!
    //! \brief  Get GMM client context of the device
    //! \return Cmd buffer manager
    //!
    GMM_CLIENT_CONTEXT *GetGmmClientContext() { return m_gmmClientContext; }

    //!
    //! \brief  Get MosDecompression
    //! \return ptr to MosDecompression
    //!
#if !EMUL
    MosDecompression *GetMosDecompression()
    {
        return m_mosDecompression;
    }
#endif

    //!
    //! \brief  Get MosMediaCopy
    //! \return ptr to MosMediaCopy
    //!
    MosMediaCopy *GetMosMediaCopy()
    {
        return m_mosMediaCopy;
    }

    //! \brief  Get the DumpFrameNum
    //! \return The current dumped frameNum
    //!
    uint32_t GetDumpFrameNum() { return m_dumpframeNum; }

    //!
    //! \brief  Set the DumpFrameNum
    //! \return update the FrameNum and return success
    //!
    MOS_STATUS SetDumpFrameNum(uint32_t framNum)
    {
        m_dumpframeNum = framNum;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Reset the DumpFrameNum
    //! \return init the FrameNum and return success
    //!
    MOS_STATUS ResetDumpFrameNum()
    {
        m_dumpframeNum = 0xffffffff;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Get the dumpLoc
    //! \return The current dumped GetdumpLoc
    //!
    char *GetdumpLoc() { return m_dumpLoc; }

    //!
    //! \brief  Reset the dumpLoc
    //! \return init the dumpLoc and return success
    //!
    MOS_STATUS ResetdumpLoc()
    {
        m_dumpLoc[0] = 0;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Determine whether the device is using async mode
    //! \return true if the device is in async mode, false otherwise
    //!
    bool IsAynchronous() { return m_aynchronousDevice; }

    //!
    //! \brief  Determine whether pooling resource is enabled
    //! \return true if pooling resource is enabled, false otherwise
    //!
    bool IsPoolingResourceEnabled() { return m_resourcePooling; }

    //!
    //! \brief  Set oca rtlog resource
    //! \return Set oca rtlog resource and return success
    //!
    MOS_STATUS SetRtLogRes(PMOS_RESOURCE ocaRTLogResource)
    {
        m_ocaRTLogResource = ocaRTLogResource;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Get OcaRTLogResource
    //! \return ptr to OcaRTLogResource
    //!
    PMOS_RESOURCE GetOcaRTLogResource()
    {
        return m_ocaRTLogResource;
    }

protected:
    //!
    //! \brief  Destory the OS ContextNext Object, internal function, called by cleanup
    //!
    virtual void Destroy() = 0;

public:
    static const uint32_t m_cmdBufAlignment = 16;   //!> Cmd buffer alignment
    bool                  m_ocaLogSectionSupported = true;
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    bool                  m_dumpCommandBuffer                   = false;
    bool                  m_dumpCommandBufferToFile           = false;
    bool                  m_dumpCommandBufferAsMessages         = false;
    char                  m_sFileName[MOS_MAX_HLT_FILENAME_LEN] = {0};
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

protected:
    GpuContextMgrNext              *m_gpuContextMgr     = nullptr; //!> GPU context manager of the device
    CmdBufMgrNext                  *m_cmdBufMgr         = nullptr; //!> Cmd buffer manager of the device
    GMM_CLIENT_CONTEXT             *m_gmmClientContext  = nullptr; //!> GMM client context of the device

    PMOS_RESOURCE                   m_ocaRTLogResource  = nullptr;
    uint32_t                        m_dumpframeNum = 0;             // For use when dump its compressed surface, override the frame number given from MediaVeboxDecompState
    char                            m_dumpLoc[MAX_PATH] = {0};       // For use when dump its compressed surface, to distinguish each loc's pre/post decomp

    //! \brief  Platform string including product family, chipset family, etc
    PLATFORM                        m_platformInfo = {};

    //! \brief  sku table
    MEDIA_FEATURE_TABLE             m_skuTable = {};

     //! \brief  wa table
    MEDIA_WA_TABLE                  m_waTable = {};

     //! \brief  GT system information, like EU counter, thread count, etc.
    MEDIA_SYSTEM_INFO               m_gtSystemInfo = {};

    //! \brief  Whether the processor is Atom
    bool                            m_isAtomSOC = false;


    //! \brief  Flag to mark whether the os context is valid
    bool                            m_osContextValid =  false;

    //! \brief  Flag to mark whether Null Hw is enabled
    bool                            m_nullHwIsEnabled = false;

    //! \brief  Whether or not need deallocation on exit
    bool                            m_deallocateOnExit = false;

    //! \brief  need KMD to track the media frame or not
    bool                            m_enableKmdMediaFrameTracking = false;

    //! \brief   For Resource addressing, whether patch list mode is active
    bool                            m_usesPatchList = false;

    //! \brief   For Resource addressing, whether GPU address mode is active
    bool                            m_usesGfxAddress = false;

    //! \brief   Component info
    MOS_COMPONENT                   m_component = COMPONENT_UNKNOWN;

    //! \brief   Flag to indicate if Tile64 is supported
    bool                            m_mediaTile64 = false;

    //! \brief   Flag to indicate if NoGfxMemory is needed
    bool                            m_noGfxMemoryNeeded = false;

    //! \brief  Flag to indicate if Vebox BatchBuffer scalability enabled state has been reported
    bool                            m_veboxScalabilityEnableReported = false;

    //! \brief  Flag to indicate if Vdbox BatchBuffer scalability enabled state has been reported
    bool                            m_vdboxScalabilityEnableReported = false;

    //! \brief  Flag to indicate if Vebox BatchBuffer scalability disabled state has been reported
    bool                            m_veboxScalabilityDisableReported = false;

    //! \brief  Flag to indicate if Vdbox BatchBuffer scalability disabled state has been reported
    bool                            m_vdboxScalabilityDisableReported = false;

    //! \brief  the ptr to mos decompression module
#if !EMUL
    MosDecompression                *m_mosDecompression = nullptr;
#endif

    //! \brief the ptr to mos media copy module
    MosMediaCopy                    *m_mosMediaCopy = nullptr;

    //! \brief the ptr to mos mock adaptor module
    MosMockAdaptor                  *m_mockAdaptor = nullptr;

    //!< Indicate if this device is working in aync mode or normal mode
    bool                            m_aynchronousDevice = false;

    //! \brief is pooled resource is supported
    bool                            m_resourcePooling = false;

#if (_DEBUG || _RELEASE_INTERNAL)
public:
    //!
    //! \brief  Initialize dummy VDBox slot array; reads fakeCount/realCount from MockAdaptor
    //!         (one-time, under mutex, idempotent).  Requires NullHW enabled and MockAdaptor
    //!         already initialized via NullHwInit().
    //! \return MOS_STATUS
    //!
    MOS_STATUS InitDummyVdboxSlots();

    //!
    //! \brief  Select and claim the least-loaded dummy VDBox slot (thread-safe)
    //! \param  [in]      isEncode        true for encode pipeline, false for decode
    //! \param  [in,out]  isScalable      On entry: caller's intent (from config). On exit: whether
    //!                                   scalable path was actually taken (set to false when
    //!                                   vdCount < 2 forces fallback to standard path)
    //! \param  [out] gpuNode         Assigned GPU node
    //! \param  [out] claimedSlotIndex Index of the claimed slot
    //! \return MOS_STATUS
    //!
    MOS_STATUS SelectAndClaimDummyVdSlot(
        bool          isEncode,
        bool         &isScalable,
        MOS_GPU_NODE &gpuNode,
        int32_t      &claimedSlotIndex);

    //!
    //! \brief  Release a previously claimed dummy VDBox slot (thread-safe)
    //! \param  [in] slotIndex   Slot index returned by SelectAndClaimDummyVdSlot
    //! \param  [in] isScalable  true if the slot was claimed via scalable path
    //!                          (decrements all VD refCounts); false for single-slot release
    //!
    void ReleaseDummyVdSlot(int32_t slotIndex, bool isScalable);

    //!
    //! \brief  Get the per-process dummy VDBox slot mutex for thread-safe access.
    //! \note   Returns a function-local static (Meyers singleton) so OsContextNext
    //!         has no non-trivial member and its class layout is unaffected.
    //! \return Reference to the mutex
    //!
    std::mutex &GetDummyVdboxMutex()
    {
        static std::mutex s_mutex;
        return s_mutex;
    }

    //!
    //! \brief  Check if dummy VDBox slots have been initialized
    //! \return true if initialized
    //!
    bool IsDummyVdboxInitialized() const { return m_dummyVdboxInitialized; }

    //!
    //! \brief  Set dummy VDBox initialization flag
    //!
    void SetDummyVdboxInitialized(bool initialized) { m_dummyVdboxInitialized = initialized; }

    //!
    //! \brief  Get dummy VDBox array
    //! \return Pointer to DummyVdboxInfo array
    //!
    DummyVdboxInfo *GetDummyVdboxArray() { return m_dummyVdboxArray; }

    //!
    //! \brief  Get dummy VDBox slot count
    //! \return Number of slots in the array
    //!
    uint32_t GetDummyVdboxCount() const { return m_dummyVdboxCount; }

    //!
    //! \brief  Set dummy VDBox slot count
    //!
    void SetDummyVdboxCount(uint32_t count) { m_dummyVdboxCount = count; }

    //!
    //! \brief  Get per-slot reference count array
    //! \return Pointer to ref count array
    //!
    uint32_t *GetSlotRefCount() { return m_slotRefCount; }

    //!
    //! \brief  Get decode start slot counter (initial DUMMY_VDBOX_NUM_MAX-1, VEBox-first)
    //! \return Reference to counter
    //!
    uint32_t &GetStartSlotCounterDecode() { return m_startSlotCounterDecode; }

    //!
    //! \brief  Get encode start slot counter (initial 0, VDBox-first)
    //! \return Reference to counter
    //!
    uint32_t &GetStartSlotCounterEncode() { return m_startSlotCounterEncode; }

    //!
    //! \brief  Reset all dummy VDBox state (for ULT)
    //!
    void ResetDummyVdboxState()
    {
        m_dummyVdboxInitialized = false;
        m_dummyVdboxCount       = 0;
        MOS_ZeroMemory(m_dummyVdboxArray, sizeof(m_dummyVdboxArray));
        MOS_ZeroMemory(m_slotRefCount, sizeof(m_slotRefCount));
        m_startSlotCounterDecode = DUMMY_VDBOX_NUM_MAX - 1;
        m_startSlotCounterEncode = 0;
    }

protected:
    bool         m_dummyVdboxInitialized = false;                       //!< Set on first Initialize() call
    DummyVdboxInfo m_dummyVdboxArray[DUMMY_VDBOX_NUM_MAX] = {};         //!< Per-slot engine info
    uint32_t     m_dummyVdboxCount = 0;                                 //!< Number of active slots
    uint32_t     m_slotRefCount[DUMMY_VDBOX_NUM_MAX] = {};              //!< Per-slot active pipeline count
    uint32_t     m_startSlotCounterDecode = DUMMY_VDBOX_NUM_MAX - 1;    //!< Decode: VEBox-first
    uint32_t     m_startSlotCounterEncode = 0;                          //!< Encode: VDBox-first
#endif // (_DEBUG || _RELEASE_INTERNAL)

MEDIA_CLASS_DEFINE_END(OsContextNext)
};
#endif // #ifndef __MOS_CONTEXTNext_NEXT_H__
